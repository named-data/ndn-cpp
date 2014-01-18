/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from tlv.hpp by Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_ENCODER_H
#define NDN_TLV_ENCODER_H

#include "../../errors.h"
#include "../../util/dynamic-uint8-array.h"
#include "../../util/blob.h"
#include "tlv.h"

#ifdef __cplusplus
extern "C" {
#endif

/** An ndn_TlvEncoder struct is used by all the TLV encoding functions.  You should initialize it with
 * ndn_TlvEncoder_initialize.  You can set enableOutput to 0 to only advance self->offset without writing to output
 * as a way to pre-compute the length of child elements.
 */
struct ndn_TlvEncoder {
  struct ndn_DynamicUInt8Array *output; /**< A pointer to a ndn_DynamicUInt8Array which receives the encoded output. */
  size_t offset;                        /**< The offset into output.array for the next encoding. */
  int enableOutput;                     /**< If 0, then only advance offset without writing to output. */
};

/**
 * Initialize an ndn_TlvEncoder struct with the arguments for initializing the ndn_DynamicUInt8Array, and set 
 * enableOutput to 1.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param output A pointer to a ndn_DynamicUInt8Array struct which receives the encoded output.  The struct must
 * remain valid during the entire life of this ndn_TlvEncoder. If the output->realloc
 * function pointer is null, its array must be large enough to receive the entire encoding.
 */
static inline void 
ndn_TlvEncoder_initialize(struct ndn_TlvEncoder *self, struct ndn_DynamicUInt8Array *output) 
{
  self->output = output;
  self->offset = 0;
  self->enableOutput = 1;
}

/**
 * Return the number of bytes to encode varNumber as a VAR-NUMBER in NDN-TLV.
 * @param integer The integer to encode.
 * @return The number of bytes to encode integer.
 */
static inline size_t
ndn_TlvEncoder_sizeOfVarNumber(uint64_t varNumber)
{
  if (varNumber < 253)
    return 1;
  else if (varNumber <= 0xffff)
    return 3;
  else if (varNumber <= 0xffffffff)
    return 5;
  else
    return 9;
}

/**
 * A private function to do the work of writeVarNumber, assuming that self->enableOutput is 1.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param varNumber The number to encode.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_TlvEncoder_writeVarNumberEnabled(struct ndn_TlvEncoder *self, uint64_t varNumber);

/**
 * Encode varNumber as a VAR-NUMBER in NDN-TLV and write it to self->output.  If self->enableOutput is 0, 
 * then just advance self->offset without writing to output.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param varNumber The number to encode.
 * @return 0 for success, else an error code. If self->enableOutput is 0, this always returns 0.
 */
static inline ndn_Error 
ndn_TlvEncoder_writeVarNumber(struct ndn_TlvEncoder *self, uint64_t varNumber)
{
  if (self->enableOutput)
    return ndn_TlvEncoder_writeVarNumberEnabled(self, varNumber);
  else {
    // Just advance offset.
    self->offset += ndn_TlvEncoder_sizeOfVarNumber(varNumber);
    return NDN_ERROR_success;
  }
}

/**
 * Write the type and length to self->output.  If self->enableOutput is 0, then just advance self->offset without writing 
 * to output.  
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param type the type of the TLV.
 * @param length The length of the TLV.
 * @return 0 for success, else an error code. If self->enableOutput is 0, this always returns 0.
 */
static inline ndn_Error 
ndn_TlvEncoder_writeTypeAndLength(struct ndn_TlvEncoder *self, unsigned int type, size_t length)
{
  if (self->enableOutput) {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeVarNumberEnabled(self, (uint64_t)type)))
      return error;
    if ((error = ndn_TlvEncoder_writeVarNumberEnabled(self, (uint64_t)length)))
      return error;
  }
  else
    // Just advance offset.
    self->offset += ndn_TlvEncoder_sizeOfVarNumber((uint64_t)type) + 
      ndn_TlvEncoder_sizeOfVarNumber((uint64_t)length);
  
  return NDN_ERROR_success;
}

/**
 * Return the number of bytes to encode integer as a non-negative integer.
 * @param integer The integer to encode.
 * @return The number of bytes to encode integer.
 */
static inline size_t
ndn_TlvEncoder_sizeOfNonNegativeInteger(uint64_t integer)
{
  if (integer < 253)
    return 1;
  else if (integer <= 0xffff)
    return 2;
  else if (integer <= 0xffffffff)
    return 4;
  else
    return 8;
}

/**
 * A private function to do the work of writeNonNegativeInteger, assuming that self->enableOutput is 1.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param integer The integer to encode.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_TlvEncoder_writeNonNegativeIntegerEnabled(struct ndn_TlvEncoder *self, uint64_t integer);

/**
 * Encode integer as a non-negative integer in NDN-TLV and write it to self->output.  If self->enableOutput is 0, 
 * then just advance self->offset without writing to output.  This does not write a type or length for the integer.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param integer The integer to encode.
 * @return 0 for success, else an error code. If self->enableOutput is 0, this always returns 0.
 */
static inline ndn_Error 
ndn_TlvEncoder_writeNonNegativeInteger(struct ndn_TlvEncoder *self, uint64_t integer)
{
  if (self->enableOutput)
    return ndn_TlvEncoder_writeNonNegativeIntegerEnabled(self, integer);
  else {
    // Just advance offset.
    self->offset += ndn_TlvEncoder_sizeOfNonNegativeInteger(integer);
    return NDN_ERROR_success;
  }
}

/**
 * Return the number of bytes to encode the type, length and blob value.
 * @param value A Blob with the array of bytes for the value.
 * @return The number of bytes to encode the TLV.
 */
static inline size_t
ndn_TlvEncoder_sizeOfBlobTlv(int type, struct ndn_Blob *value)
{
  return ndn_TlvEncoder_sizeOfVarNumber((uint64_t)type) + ndn_TlvEncoder_sizeOfVarNumber((uint64_t)value->length) + 
    value->length;
}

/**
 * A private function to do the work of writeBlobTlv, assuming that self->enableOutput is 1.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param type the type of the TLV.
 * @param value A Blob with the array of bytes for the value.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_TlvEncoder_writeBlobTlvEnabled(struct ndn_TlvEncoder *self, int type, struct ndn_Blob *value);

/**
 * Write the type, then the length of the blob then the blob value to self->output.  If self->enableOutput is 0, 
 * then just advance self->offset without writing to output.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param type the type of the TLV.
 * @param value A Blob with the array of bytes for the value.
 * @return 0 for success, else an error code. If self->enableOutput is 0, this always returns 0.
 */
static inline ndn_Error 
ndn_TlvEncoder_writeBlobTlv(struct ndn_TlvEncoder *self, int type, struct ndn_Blob *value)
{
  if (self->enableOutput)
    return ndn_TlvEncoder_writeBlobTlvEnabled(self, type, value);
  else
    // Just advance offset.
    self->offset += ndn_TlvEncoder_sizeOfBlobTlv(type, value);
  
  return NDN_ERROR_success;
}

/**
 * Write the type, then the length of the encoded integer then encode integer as a non-negative integer 
 * and write it to self->output.  If self->enableOutput is 0, then just advance self->offset without writing to output.  
 * (If you want to just write the non-negative integer, use ndn_TlvEncoder_writeNonNegativeInteger.)
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param type the type of the TLV.
 * @param integer The integer to encode.
 * @return 0 for success, else an error code. If self->enableOutput is 0, this always returns 0.
 */
static inline ndn_Error 
ndn_TlvEncoder_writeNonNegativeIntegerTlv(struct ndn_TlvEncoder *self, int type, uint64_t integer)
{
  size_t sizeOfInteger = ndn_TlvEncoder_sizeOfNonNegativeInteger(integer);
  if (self->enableOutput) {
    ndn_Error error;
    if ((error = ndn_TlvEncoder_writeTypeAndLength(self, type, sizeOfInteger)))
      return error;
    if ((error = ndn_TlvEncoder_writeNonNegativeIntegerEnabled(self, integer)))
      return error;
  }
  else
    // Just advance offset.
    self->offset += ndn_TlvEncoder_sizeOfVarNumber((uint64_t)type) + 
      ndn_TlvEncoder_sizeOfVarNumber((uint64_t)sizeOfInteger) + sizeOfInteger;
  
  return NDN_ERROR_success;
}

/**
 * Make a first pass to call writeValue with self->enableOutput = 0 to determine the length of the TLV. Then set
 * self->enableOutput = 1 and write the type and length to self->output and call writeValue again to write the 
 * TLVs in the body of the value.  This is to solve the problem of finding the length when the value of a TLV has
 * nested TLVs.  However, if self->enableOutput is already 0 when this is called, then just call writeValue once
 * to advance self->offset without writing to output.
 * @param self A pointer to the ndn_TlvEncoder struct.
 * @param type the type of the TLV.
 * @param writeValue A pointer to a function that writes the TLVs in the body of the value.  This calls
 * writeValue(context, self).
 * @param context A pointer to memory which is passed to writeValue.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_TlvEncoder_writeNestedTlv
  (struct ndn_TlvEncoder *self, int type, ndn_Error (*writeValue)(void *context, struct ndn_TlvEncoder *encoder), 
   void *context);

#ifdef  __cplusplus
}
#endif

#endif
