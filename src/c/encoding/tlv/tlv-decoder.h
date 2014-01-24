/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from tlv.hpp by Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_DECODER_H
#define NDN_TLV_DECODER_H

#include <ndn-cpp/c/common.h>
#include "../../errors.h"
#include "../../util/blob.h"
#include "tlv.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct ndn_TlvDecoder {
  const uint8_t *input;
  size_t inputLength;
  size_t offset;
};

/**
 * Initialize an ndn_TlvDecoder struct to decode the input.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param input A pointer to the input buffer to decode.
 * @param inputLength The number of bytes in input.
 */
static inline void 
ndn_TlvDecoder_initialize(struct ndn_TlvDecoder *self, const uint8_t *input, size_t inputLength) 
{
  self->input = input;
  self->inputLength = inputLength;
  self->offset = 0;
}

/**
 * A private function to do the work of readVarNumber, given the firstOctet which is >= 253.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param firstOctet The first octet which is >= 253, used to decode the remaining bytes.
 * @param varNumber Return the VAR-NUMBER.
 * @return 
 */
ndn_Error 
ndn_TlvDecoder_readExtendedVarNumber(struct ndn_TlvDecoder *self, unsigned int firstOctet, uint64_t *varNumber);

/**
 * Decode VAR-NUMBER in NDN-TLV and set varNumber. Update offset.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param varNumber Return the VAR-NUMBER.
 * @return 0 for success, else an error code for read past the end of the input.
 */
static inline ndn_Error 
ndn_TlvDecoder_readVarNumber(struct ndn_TlvDecoder *self, uint64_t *varNumber)
{
  // Read the first octet inline.
  if (self->offset >= self->inputLength)
    return NDN_ERROR_read_past_the_end_of_the_input;

  unsigned int firstOctet = (unsigned int)self->input[self->offset++];
  if (firstOctet < 253) {
    // The value is simple, so we can return it inline.
    *varNumber = (uint64_t)firstOctet;
    return NDN_ERROR_success;
  }
  else
    return ndn_TlvDecoder_readExtendedVarNumber(self, firstOctet, varNumber);
}

/**
 * Decode the type and length from self's input starting at offset, expecting the type to be expectedType.
 * Update offset.  Also make sure the decoded length does not exceed the number of bytes remaining in the input.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param length Return the length of the TLV.
 * @return 0 for success, else an error code, including an error if not the expected type or if the decoded length
 * exceeds the number of bytes remaining.
 */
ndn_Error 
ndn_TlvDecoder_readTypeAndLength(struct ndn_TlvDecoder *self, unsigned int expectedType, size_t *length);

/**
 * Decode the type and length from self's input starting at offset, expecting the type to be expectedType.
 * Update offset.  Also make sure the decoded length does not exceed the number of bytes remaining in the input.
 * Set endOffset to the offset of the end of this parent TLV, which is used in decoding optional nested TLVs.
 * After reading all nested TLVs, call ndn_TlvDecoder_finishNestedTlvs.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param endOffset Return the offset of the end of the parent TLV.
 * @return 0 for success, else an error code, including an error if not the expected type or if the decoded length
 * exceeds the number of bytes remaining.
 */
static inline ndn_Error 
ndn_TlvDecoder_readNestedTlvsStart(struct ndn_TlvDecoder *self, unsigned int expectedType, size_t *endOffset)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(self, expectedType, &length)))
    return error;
  *endOffset = self->offset + length;
  
  return NDN_ERROR_success;
}

/**
 * Call this after reading all nested TLVs to check if the current offset matches the endOffset returned by
 * ndn_TlvDecoder_readNestedTlvsStart.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param endOffset The offset of the end of the parent TLV, returned by ndn_TlvDecoder_readNestedTlvsStart.
 * @return 0 for success, else an error code if the TLV length does not equal the total length of nested TLVs
 */
static inline ndn_Error
ndn_TlvDecoder_finishNestedTlvs(struct ndn_TlvDecoder *self, size_t endOffset)
{
  if (self->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;  
}

/**
 * Decode the type from self's input starting at offset, and if it is the expectedType,
 * then set gotExpectedType to 1, else 0.  However, if self->offset is greater than or equal to endOffset,
 * then set gotExpectedType to 0 and don't try to read the type.
 * Do not update offset, including if returning an error.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType the expected type.
 * @param endOffset The offset of the end of the parent TLV.
 * @param gotExpectedType Return a 1 if got the expected type, else 0
 * @return 0 for success, else an error code for read past the end of the input.
 */
static inline ndn_Error 
ndn_TlvDecoder_peekType(struct ndn_TlvDecoder *self, unsigned int expectedType, size_t endOffset, int *gotExpectedType)
{
  if (self->offset >= endOffset)
    // No more sub TLVs to look at.
    *gotExpectedType = 0;
  else {
    size_t saveOffset = self->offset;
    uint64_t type;
    ndn_Error error = ndn_TlvDecoder_readVarNumber(self, &type);
    // Restore offset.
    self->offset = saveOffset;
    if (error)
      return error;
    
    *gotExpectedType = (type == expectedType ? 1 : 0);
  }
  
  return NDN_ERROR_success;
}

/**
 * A private function to do the work of readNonNegativeInteger, assuming the length != 1.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param length The number of bytes in the encoded integer.
 * @param value Return the integer.
 * @return 
 */
ndn_Error 
ndn_TlvDecoder_readExtendedNonNegativeInteger(struct ndn_TlvDecoder *self, size_t length, uint64_t *value);

/**
 * Decode a non-negative integer in NDN-TLV and set value. Update offset by length.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param length The number of bytes in the encoded integer.
 * @param value Return the integer.
 * @return 0 for success, else an error code for read past the end of the input.
 */
static inline ndn_Error 
ndn_TlvDecoder_readNonNegativeInteger(struct ndn_TlvDecoder *self, size_t length, uint64_t *value)
{
  if (length == 1) {
    // Read the simple integer inline.
    if (self->offset >= self->inputLength)
      return NDN_ERROR_read_past_the_end_of_the_input;

    *value = (uint64_t)self->input[self->offset++];
    return NDN_ERROR_success;
  }
  else
    return ndn_TlvDecoder_readExtendedNonNegativeInteger(self, length, value);
}

/**
 * Decode the type and length from self's input starting at offset, expecting the type to be expectedType.
 * Then non-negative integer in NDN-TLV and set value.  Update offset.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param value Return the integer.
 * @return 0 for success, else an error code, including an error if not the expected type or if the decoded length
 * exceeds the number of bytes remaining.
 */
static inline ndn_Error 
ndn_TlvDecoder_readNonNegativeIntegerTlv(struct ndn_TlvDecoder *self, unsigned int expectedType, uint64_t *value)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(self, expectedType, &length)))
    return error;
  if ((error = ndn_TlvDecoder_readNonNegativeInteger(self, length, value)))
    return error;
  
  return NDN_ERROR_success;
}

/**
 * Peek at the next TLV, and if it has the expectedType then call ndn_TlvDecoder_readNonNegativeIntegerTlv.
 * Otherwise, set value to -1.  However, if self->offset is greater than or equal to endOffset,
 * then set value to -1 and don't try to read the type.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param endOffset The offset of the end of the parent TLV.
 * @param value Return the integer (converted to int) or -1 if the next TLV doesn't have the expected type.
 * @return 0 for success, else an error code, including if the decoded length exceeds the number of bytes remaining.
 */
ndn_Error 
ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
  (struct ndn_TlvDecoder *self, unsigned int expectedType, size_t endOffset, int *value);

/**
 * Peek at the next TLV, and if it has the expectedType then call ndn_TlvDecoder_readNonNegativeIntegerTlv
 * and return the value as a double. Otherwise, set value to -1.  
 * However, if self->offset is greater than or equal to endOffset, then set value to -1 and don't try to read the type.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param endOffset The offset of the end of the parent TLV.
 * @param value Return the integer (converted to double) or -1 if the next TLV doesn't have the expected type.
 * @return 0 for success, else an error code, including if the decoded length exceeds the number of bytes remaining.
 */
ndn_Error 
ndn_TlvDecoder_readOptionalNonNegativeIntegerTlvAsDouble
  (struct ndn_TlvDecoder *self, unsigned int expectedType, size_t endOffset, double *value);

/**
 * Decode the type and length from self's input starting at offset, expecting the type to be expectedType.
 * Then return the value and length in the ndn_Blob.  Update offset.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param value Set value.value to a pointer to the binary data inside self's input buffer and set value.length to the length.
 * @return 0 for success, else an error code, including an error if not the expected type or if the decoded length
 * exceeds the number of bytes remaining.
 */
static inline ndn_Error 
ndn_TlvDecoder_readBlobTlv
  (struct ndn_TlvDecoder *self, unsigned int expectedType, struct ndn_Blob *value)
{
  ndn_Error error;
  if ((error = ndn_TlvDecoder_readTypeAndLength(self, expectedType, &value->length)))
    return error;
  
  // ndn_TlvDecoder_readTypeAndLength already checked if the value->length exceeds the input buffer.
  value->value = self->input + self->offset;
  self->offset += value->length;
  
  return NDN_ERROR_success;
}

/**
 * Peek at the next TLV, and if it has the expectedType then set value to 1 and read a type and value, 
 * ignoring the value. Otherwise, set value to 0.  However, if self->offset is greater than or equal to endOffset,
 * then set value to 0 and don't try to read the type.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param expectedType The expected type.
 * @param endOffset The offset of the end of the parent TLV.
 * @param value Return 1 or 0 if the next TLV doesn't have the expected type.
 * @return 0 for success, else an error code, including if the decoded length exceeds the number of bytes remaining.
 */
static inline ndn_Error 
ndn_TlvDecoder_readBooleanTlv
  (struct ndn_TlvDecoder *self, unsigned int expectedType, size_t endOffset, int *value)
{
  int gotExpectedType;
  ndn_Error error;
  if ((error = ndn_TlvDecoder_peekType(self, expectedType, endOffset, &gotExpectedType)))
    return error;
    
  if (!gotExpectedType)
    *value = 0;
  else {
    size_t dummyLength;
    if ((error = ndn_TlvDecoder_readTypeAndLength(self, expectedType, &dummyLength)))
      return error;

    *value = 1;
  }
  
  return NDN_ERROR_success;
}

#ifdef  __cplusplus
}
#endif

#endif
