/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#ifndef NDN_BINARYXMLSTRUCTUREDECODER_H
#define NDN_BINARYXMLSTRUCTUREDECODER_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/errors.h>
#include <ndn-cpp/c/encoding/element-reader-types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  ndn_BinaryXmlStructureDecoder_READ_HEADER_OR_CLOSE,
  ndn_BinaryXmlStructureDecoder_READ_BYTES
};

void ndn_BinaryXmlStructureDecoder_reset
  (struct ndn_BinaryXmlStructureDecoder *self);

static __inline void ndn_BinaryXmlStructureDecoder_initialize
  (struct ndn_BinaryXmlStructureDecoder *self)
{
  ndn_BinaryXmlStructureDecoder_reset(self);
}

/**
 * Continue scanning input starting from self->offset to find the element end.  On return, you must check
 *   self->gotElementEnd: If the end of the element which started at offset 0 is found,
 *   then self->gotElementEnd is 1 and self->offset is the length of the element.  Otherwise, self-forElementEnd is 0
 *   which means you should read more into input and call again.
 * @param self pointer to the ndn_BinaryXmlStructureDecoder struct
 * @param input the input buffer. You have to pass in input each time because the buffer could be reallocated.
 * @param inputLength the number of bytes in input.
 * @return 0 for success, else an error code
 */
ndn_Error ndn_BinaryXmlStructureDecoder_findElementEnd
  (struct ndn_BinaryXmlStructureDecoder *self, const uint8_t *input, size_t inputLength);

/**
 * Set the offset into the input, used for the next read.
 * @param self pointer to the ndn_BinaryXmlStructureDecoder struct
 * @param offset the new offset
 */
static __inline void ndn_BinaryXmlStructureDecoder_seek(struct ndn_BinaryXmlStructureDecoder *self, size_t offset)
{
  self->offset = offset;
}

#ifdef __cplusplus
}
#endif

#endif
