/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from Key.js by Meki Cheraoui.
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
#include "binary-xml-structure-decoder.h"
#include "binary-xml-name.h"
#include "binary-xml-key-locator.h"

static ndn_Error decodeKeyNameData(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlDecoder *decoder)
{
  int gotExpectedTag;
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    keyLocator->keyNameType = ndn_KeyNameType_PUBLISHER_PUBLIC_KEY_DIGEST;
    if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
        (decoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, 0, &keyLocator->keyData)))
      return error;
  }
  else {
    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_PublisherCertificateDigest, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      keyLocator->keyNameType = ndn_KeyNameType_PUBLISHER_CERTIFICATE_DIGEST;
      if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
          (decoder, ndn_BinaryXml_DTag_PublisherCertificateDigest, 0, &keyLocator->keyData)))
        return error;
    }
    else {
      if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_PublisherIssuerKeyDigest, &gotExpectedTag)))
        return error;
      if (gotExpectedTag) {
        keyLocator->keyNameType = ndn_KeyNameType_PUBLISHER_ISSUER_KEY_DIGEST;
        if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
            (decoder, ndn_BinaryXml_DTag_PublisherIssuerKeyDigest, 0, &keyLocator->keyData)))
          return error;
      }
      else {
        if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_PublisherIssuerCertificateDigest, &gotExpectedTag)))
          return error;
        if (gotExpectedTag) {
          keyLocator->keyNameType = ndn_KeyNameType_PUBLISHER_ISSUER_CERTIFICATE_DIGEST;
          if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
              (decoder, ndn_BinaryXml_DTag_PublisherIssuerCertificateDigest, 0, &keyLocator->keyData)))
            return error;
        }
        else {
          // Key name data is omitted.
          keyLocator->keyNameType = -1;
          keyLocator->keyData.length = 0;
        }
      }
    }
  }

  return NDN_ERROR_success;
}

ndn_Error ndn_encodeBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;

  if ((int)keyLocator->type < 0)
    return NDN_ERROR_success;

  if (keyLocator->type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST)
    // encodeSignedInfo already encoded this as the publisherPublicKeyDigest,
    //   so do nothing here.
    return NDN_ERROR_success;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_KeyLocator)))
    return error;

  if (keyLocator->type == ndn_KeyLocatorType_KEY) {
    if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_Key, &keyLocator->keyData)))
      return error;
  }
  else if (keyLocator->type == ndn_KeyLocatorType_CERTIFICATE) {
    if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_Certificate, &keyLocator->keyData)))
      return error;
  }
  else if (keyLocator->type == ndn_KeyLocatorType_KEYNAME) {
    size_t dummyBeginOffset, dummyEndOffset;

    if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_KeyName)))
      return error;
    if ((error = ndn_encodeBinaryXmlName
         (&keyLocator->keyName, &dummyBeginOffset, &dummyEndOffset, encoder)))
      return error;

    if ((int)keyLocator->keyNameType >= 0 && keyLocator->keyData.length > 0) {
      if (keyLocator->keyNameType == ndn_KeyNameType_PUBLISHER_PUBLIC_KEY_DIGEST) {
        if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
            (encoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest, &keyLocator->keyData)))
          return error;
      }
      else if (keyLocator->keyNameType == ndn_KeyNameType_PUBLISHER_CERTIFICATE_DIGEST) {
        if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
            (encoder, ndn_BinaryXml_DTag_PublisherCertificateDigest, &keyLocator->keyData)))
          return error;
      }
      else if (keyLocator->keyNameType == ndn_KeyNameType_PUBLISHER_ISSUER_KEY_DIGEST) {
        if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
            (encoder, ndn_BinaryXml_DTag_PublisherIssuerKeyDigest, &keyLocator->keyData)))
          return error;
      }
      else if (keyLocator->keyNameType == ndn_KeyNameType_PUBLISHER_ISSUER_CERTIFICATE_DIGEST) {
        if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
            (encoder, ndn_BinaryXml_DTag_PublisherIssuerCertificateDigest, &keyLocator->keyData)))
          return error;
      }
      else
        return NDN_ERROR_unrecognized_ndn_KeyNameType;
    }

    if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
      return error;
  }
  else
    return NDN_ERROR_unrecognized_ndn_KeyLocatorType;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  int gotExpectedTag;

  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_KeyLocator)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Key, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    keyLocator->type = ndn_KeyLocatorType_KEY;

    if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement(decoder, ndn_BinaryXml_DTag_Key, 0, &keyLocator->keyData)))
      return error;
  }
  else {
    if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Certificate, &gotExpectedTag)))
      return error;
    if (gotExpectedTag) {
      keyLocator->type = ndn_KeyLocatorType_CERTIFICATE;

      if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
          (decoder, ndn_BinaryXml_DTag_Certificate, 0, &keyLocator->keyData)))
        return error;
    }
    else {
      if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_KeyName, &gotExpectedTag)))
        return error;
      if (gotExpectedTag) {
        size_t dummyBeginOffset, dummyEndOffset;

        keyLocator->type = ndn_KeyLocatorType_KEYNAME;

        if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_KeyName)))
          return error;
        if ((error = ndn_decodeBinaryXmlName
             (&keyLocator->keyName, &dummyBeginOffset, &dummyEndOffset, decoder)))
          return error;
        if ((error = decodeKeyNameData(keyLocator, decoder)))
          return error;
        if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
          return error;
      }
      else
        return NDN_ERROR_decodeKeyLocator_unrecognized_key_locator_type;
    }
  }

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeOptionalBinaryXmlKeyLocator(struct ndn_KeyLocator *keyLocator, struct ndn_BinaryXmlDecoder *decoder)
{
  int gotExpectedTag;
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_KeyLocator, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    if ((error = ndn_decodeBinaryXmlKeyLocator(keyLocator, decoder)))
      return error;
  }
  else
    ndn_KeyLocator_initialize(keyLocator, keyLocator->keyName.components, keyLocator->keyName.maxComponents);

  return NDN_ERROR_success;
}
