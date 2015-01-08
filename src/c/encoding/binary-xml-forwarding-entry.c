/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from ForwardingEntry.js by Meki Cheraoui.
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

#include "binary-xml.h"
#include "binary-xml-forwarding-entry.h"
#include "binary-xml-name.h"
#include "binary-xml-key-locator.h"
#include "binary-xml-publisher-public-key-digest.h"

ndn_Error ndn_encodeBinaryXmlForwardingEntry(struct ndn_ForwardingEntry *forwardingEntry, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_ForwardingEntry)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalUDataDTagElement
      (encoder, ndn_BinaryXml_DTag_Action, &forwardingEntry->action)))
    return error;
  if ((error = ndn_encodeBinaryXmlName
       (&forwardingEntry->prefix, &dummyBeginOffset, &dummyEndOffset, encoder)))
    return error;
  // This will skip encoding if there is no publisherPublicKeyDigest.
  if ((error = ndn_encodeBinaryXmlPublisherPublicKeyDigest(&forwardingEntry->publisherPublicKeyDigest, encoder)))
    return error;
  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_FaceID, forwardingEntry->faceId)))
    return error;
  if ((error = ndn_BinaryXmlEncoder_writeUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_ForwardingFlags,
       ndn_ForwardingFlags_getForwardingEntryFlags(&forwardingEntry->forwardingFlags))))
    return error;
  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_FreshnessSeconds,
       ndn_ForwardingEntry_getFreshnessSeconds(forwardingEntry))))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlForwardingEntry(struct ndn_ForwardingEntry *forwardingEntry, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  int forwardingEntryFlags;
  int freshnessSeconds;
  size_t dummyBeginOffset, dummyEndOffset;

  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_ForwardingEntry)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalUDataDTagElement
      (decoder, ndn_BinaryXml_DTag_Action, &forwardingEntry->action)))
    return error;
  if ((error = ndn_decodeBinaryXmlName
       (&forwardingEntry->prefix, &dummyBeginOffset, &dummyEndOffset, decoder)))
    return error;
  if ((error = ndn_decodeOptionalBinaryXmlPublisherPublicKeyDigest(&forwardingEntry->publisherPublicKeyDigest, decoder)))
    return error;
  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_FaceID, &forwardingEntry->faceId)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_ForwardingFlags, &forwardingEntryFlags)))
    return error;
  if (forwardingEntryFlags >= 0)
    ndn_ForwardingFlags_setForwardingEntryFlags(&forwardingEntry->forwardingFlags, forwardingEntryFlags);
  else
    // This sets the default flags.
    ndn_ForwardingFlags_initialize(&forwardingEntry->forwardingFlags);

  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_FreshnessSeconds, &freshnessSeconds)))
    return error;
  ndn_ForwardingEntry_setFreshnessSeconds(forwardingEntry, freshnessSeconds);

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}
