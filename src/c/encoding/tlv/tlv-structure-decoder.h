/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_STRUCTURE_DECODER_H
#define NDN_TLV_STRUCTURE_DECODER_H

#include <ndn-cpp/c/common.h>
#include "../../errors.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct ndn_TlvStructureDecoder {
  int gotElementEnd; /**< boolean */
  size_t offset;
  int state;
  size_t headerLength;
  int useHeaderBuffer; /**< boolean */
  // 8 bytes is enough to hold the extended bytes in the length encoding where it is an 8-byte number.
  uint8_t headerBuffer[8];
  int nBytesToRead;
  unsigned int firstOctet;
};

enum {
  ndn_TlvStructureDecoder_READ_TYPE,
  ndn_TlvStructureDecoder_READ_TYPE_BYTES,
  ndn_TlvStructureDecoder_READ_LENGTH,
  ndn_TlvStructureDecoder_READ_LENGTH_BYTES,
  ndn_TlvStructureDecoder_READ_VALUE_BYTES
};

/**
 * Initialize a ndn_TlvStructureDecoder struct.
 * @param self A pointer to the ndn_TlvStructureDecoder struct.
 */
void 
ndn_TlvStructureDecoder_initialize(struct ndn_TlvStructureDecoder *self);
        
/**
 * Continue scanning input starting from self->offset to find the element end.  On return, you must check
 *   self->gotElementEnd: If the end of the element which started at offset 0 is found, 
 *   then self->gotElementEnd is 1 and self->offset is the length of the element.  Otherwise, self-gotElementEnd is 0 
 *   which means you should read more into input and call again.
 * @param self A pointer to the ndn_TlvStructureDecoder struct.
 * @param input The input buffer. You have to pass in input each time because the buffer could be reallocated.
 * @param inputLength The number of bytes in input.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_TlvStructureDecoder_findElementEnd(struct ndn_TlvStructureDecoder *self, uint8_t *input, size_t inputLength);

/**
 * Set the offset into the input, used for the next read.
 * @param self A pointer to the ndn_TlvStructureDecoder struct.
 * @param offset The new offset.
 */
static __inline void 
ndn_TlvStructureDecoder_seek(struct ndn_TlvStructureDecoder *self, size_t offset) 
{
  self->offset = offset;
}

#ifdef  __cplusplus
}
#endif

#endif
