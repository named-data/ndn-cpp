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
 * A private function to do the work of readNonNegativeInteger, assuming the length != 1.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param length The number of bytes in the encoded integer.
 * @param integer Return the integer.
 * @return 
 */
ndn_Error 
ndn_TlvDecoder_readExtendedNonNegativeInteger(struct ndn_TlvDecoder *self, size_t length, uint64_t *integer);

/**
 * Decode a non-negative integer in NDN-TLV and set integer. Update offset by length.
 * @param self A pointer to the ndn_TlvDecoder struct.
 * @param length The number of bytes in the encoded integer.
 * @param integer Return the integer.
 * @return 0 for success, else an error code for read past the end of the input.
 */
static inline ndn_Error 
ndn_TlvDecoder_readNonNegativeInteger(struct ndn_TlvDecoder *self, size_t length, uint64_t *integer)
{
  if (length == 1) {
    // Read the simple integer inline.
    if (self->offset >= self->inputLength)
      return NDN_ERROR_read_past_the_end_of_the_input;

    *integer = (uint64_t)self->input[self->offset++];
    return NDN_ERROR_success;
  }
  else
    return ndn_TlvDecoder_readExtendedNonNegativeInteger(self, length, integer);
}

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

#ifdef  __cplusplus
}
#endif

#endif
