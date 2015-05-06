/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from Name.js by Meki Cheraoui.
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

#include "binary-xml-name.h"

ndn_Error ndn_encodeBinaryXmlName
  (struct ndn_Name *name, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_Name)))
    return error;

  *signedPortionBeginOffset = encoder->offset;

  if (name->nComponents == 0)
    // There is no "final component", so set signedPortionEndOffset arbitrarily.
    *signedPortionEndOffset = *signedPortionBeginOffset;
  else {
    size_t i;
    for (i = 0; i < name->nComponents; ++i) {
      if (i == name->nComponents - 1)
        // We will begin the final component.
        *signedPortionEndOffset = encoder->offset;

      if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
           (encoder, ndn_BinaryXml_DTag_Component, &name->components[i].value)))
        return error;
    }
  }

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlName
  (struct ndn_Name *name, size_t *signedPortionBeginOffset,
   size_t *signedPortionEndOffset, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_Name)))
    return error;

  *signedPortionBeginOffset = decoder->offset;
  // In case there are no components, set signedPortionEndOffset arbitrarily.
  *signedPortionEndOffset = *signedPortionBeginOffset;

  name->nComponents = 0;
  while (1) {
    int gotExpectedTag;
    struct ndn_Blob component;

    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Component, &gotExpectedTag)))
      return error;

    if (!gotExpectedTag)
      // No more components.
      break;

    *signedPortionEndOffset = decoder->offset;

    if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement(decoder, ndn_BinaryXml_DTag_Component, 0, &component)))
      return error;
    if ((error = ndn_Name_appendBlob(name, &component)))
      return error;
  }

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}
