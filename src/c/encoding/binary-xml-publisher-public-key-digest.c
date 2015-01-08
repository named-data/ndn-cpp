/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from PublisherPublicKeyDigest.js by Meki Cheraoui.
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
#include "binary-xml-publisher-public-key-digest.h"

ndn_Error ndn_encodeBinaryXmlPublisherPublicKeyDigest
  (struct ndn_PublisherPublicKeyDigest *publisherPublicKeyDigest, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;

  if (!publisherPublicKeyDigest->publisherPublicKeyDigest.value || publisherPublicKeyDigest->publisherPublicKeyDigest.length == 0)
    return NDN_ERROR_success;

  if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
      (encoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, &publisherPublicKeyDigest->publisherPublicKeyDigest)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlPublisherPublicKeyDigest
  (struct ndn_PublisherPublicKeyDigest *publisherPublicKeyDigest, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, 0, &publisherPublicKeyDigest->publisherPublicKeyDigest)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeOptionalBinaryXmlPublisherPublicKeyDigest
  (struct ndn_PublisherPublicKeyDigest *publisherPublicKeyDigest, struct ndn_BinaryXmlDecoder *decoder)
{
  int gotExpectedTag;
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    if ((error = ndn_decodeBinaryXmlPublisherPublicKeyDigest(publisherPublicKeyDigest, decoder)))
      return error;
  }
  else {
    publisherPublicKeyDigest->publisherPublicKeyDigest.value = 0;
    publisherPublicKeyDigest->publisherPublicKeyDigest.length = 0;
  }

  return NDN_ERROR_success;
}
