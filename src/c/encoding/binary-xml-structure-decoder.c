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

#include "../util/ndn_memory.h"
#include "binary-xml.h"
#include "binary-xml-decoder.h"
#include "binary-xml-structure-decoder.h"

void ndn_BinaryXmlStructureDecoder_reset(struct ndn_BinaryXmlStructureDecoder *self)
{
  self->gotElementEnd = 0;
  self->offset = 0;
  self->level = 0;
  self->state = ndn_BinaryXmlStructureDecoder_READ_HEADER_OR_CLOSE;
  self->headerLength = 0;
  self->useHeaderBuffer = 0;
  self->nBytesToRead = 0;
}

/**
 * Set the state to READ_HEADER_OR_CLOSE and set up to start reading the header.
 */
static __inline void startHeader(struct ndn_BinaryXmlStructureDecoder *self)
{
  self->headerLength = 0;
  self->useHeaderBuffer = 0;
  self->state = ndn_BinaryXmlStructureDecoder_READ_HEADER_OR_CLOSE;
}

ndn_Error ndn_BinaryXmlStructureDecoder_findElementEnd
  (struct ndn_BinaryXmlStructureDecoder *self, const uint8_t *input, size_t inputLength)
{
  struct ndn_BinaryXmlDecoder decoder;

  if (self->gotElementEnd)
    // Someone is calling when we already got the end.
    return NDN_ERROR_success;

  ndn_BinaryXmlDecoder_initialize(&decoder, input, inputLength);

  while (1) {
    if (self->offset >= inputLength)
      // All the cases assume we have some input. Return and wait for more.
      return NDN_ERROR_success;

    if (self->state == ndn_BinaryXmlStructureDecoder_READ_HEADER_OR_CLOSE) {
      size_t startingHeaderLength;
      unsigned int type;
      unsigned int value;

      // First check for CLOSE.
      if (self->headerLength == 0 && input[self->offset] == ndn_BinaryXml_CLOSE) {
        ++self->offset;
        // Close the level.
        --self->level;
        if (self->level == 0) {
          // Finished.
          self->gotElementEnd = 1;
          return NDN_ERROR_success;
        }
        if (self->level < 0)
          return NDN_ERROR_findElementEnd_unexpected_close_tag;

        // Get ready for the next header.
        startHeader(self);
        continue;
      }

      startingHeaderLength = self->headerLength;
      while (1) {
        unsigned int headerByte;

        if (self->offset >= inputLength) {
          size_t nNewBytes;

          // We can't get all of the header bytes from this input. Save in headerBuffer.
          if (self->headerLength > sizeof(self->headerBuffer))
            return NDN_ERROR_cannot_store_more_header_bytes_than_the_size_of_headerBuffer;
          self->useHeaderBuffer = 1;
          nNewBytes = self->headerLength - startingHeaderLength;
          ndn_memcpy(self->headerBuffer + startingHeaderLength, input + (self->offset - nNewBytes), nNewBytes);

          return NDN_ERROR_success;
        }
        headerByte = (unsigned int)input[self->offset++];
        ++self->headerLength;
        if (headerByte & ndn_BinaryXml_TT_FINAL)
          // Break and read the header.
          break;
      }

      if (self->useHeaderBuffer) {
        size_t nNewBytes;
        struct ndn_BinaryXmlDecoder bufferDecoder;

        // Copy the remaining bytes into headerBuffer.
        if (self->headerLength > sizeof(self->headerBuffer))
          return NDN_ERROR_cannot_store_more_header_bytes_than_the_size_of_headerBuffer;
        nNewBytes = self->headerLength - startingHeaderLength;
        ndn_memcpy(self->headerBuffer + startingHeaderLength, input + (self->offset - nNewBytes), nNewBytes);

        // Use a local decoder just for the headerBuffer.
        ndn_BinaryXmlDecoder_initialize(&bufferDecoder, self->headerBuffer, sizeof(self->headerBuffer));
        if (ndn_BinaryXmlDecoder_decodeTypeAndValue(&bufferDecoder, &type, &value))
          return NDN_ERROR_findElementEnd_cannot_read_header_type_and_value;
      }
      else {
        // We didn't have to use the headerBuffer.
        ndn_BinaryXmlDecoder_seek(&decoder, self->offset - self->headerLength);
        if (ndn_BinaryXmlDecoder_decodeTypeAndValue(&decoder, &type, &value))
          return NDN_ERROR_findElementEnd_cannot_read_header_type_and_value;
      }

      // Set the next state based on the type.
      if (type == ndn_BinaryXml_DATTR)
        // We already consumed the item. READ_HEADER_OR_CLOSE again.
        // Binary XML has rules about what must follow an attribute, but we are just scanning.
        startHeader(self);
      else if (type == ndn_BinaryXml_DTAG || type == ndn_BinaryXml_EXT) {
        // Start a new level and READ_HEADER_OR_CLOSE again.
        ++self->level;
        startHeader(self);
      }
      else if (type == ndn_BinaryXml_TAG || type == ndn_BinaryXml_ATTR) {
        if (type == ndn_BinaryXml_TAG)
          // Start a new level and read the tag.
          ++self->level;
        // Minimum tag or attribute length is 1.
        self->nBytesToRead = value + 1;
        self->state = ndn_BinaryXmlStructureDecoder_READ_BYTES;
        // Binary XML has rules about what must follow an attribute, but we are just scanning.
      }
      else if (type == ndn_BinaryXml_BLOB || type == ndn_BinaryXml_UDATA) {
        self->nBytesToRead = value;
        self->state = ndn_BinaryXmlStructureDecoder_READ_BYTES;
      }
      else
        return NDN_ERROR_findElementEnd_unrecognized_header_type;
    }
    else if (self->state == ndn_BinaryXmlStructureDecoder_READ_BYTES) {
      size_t nRemainingBytes = inputLength - self->offset;
      if (nRemainingBytes < self->nBytesToRead) {
        // Need more.
        self->offset += nRemainingBytes;
        self->nBytesToRead -= nRemainingBytes;
        return NDN_ERROR_success;
      }
      // Got the bytes. Read a new header or close.
      self->offset += self->nBytesToRead;
      startHeader(self);
    }
    else
      // We don't expect this to happen.
      return NDN_ERROR_findElementEnd_unrecognized_state;
  }
}
