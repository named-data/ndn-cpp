/**
 * Copyright (C) 2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#include "tlv-key-locator.h"
#include "tlv-signature-info.h"

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the publisherPublicKeyDigest as a KeyLocatorDigest
 * in the body of the KeyLocator value.  (When we remove the deprecated publisherPublicKeyDigest, we won't need this.)
 * @param context This is the ndn_Signature struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeKeyLocatorPublisherPublicKeyDigestValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Signature *signatureInfo = (struct ndn_Signature *)context;

  ndn_Error error;
  if ((error = ndn_TlvEncoder_writeBlobTlv
       (encoder, ndn_Tlv_KeyLocatorDigest,
        &signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest)))
    return error;

  return NDN_ERROR_success;
}

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the SignatureSha256WithRsa value.
 * @param context This is the ndn_Signature struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error
encodeSignatureSha256WithRsaValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Signature *signature = (struct ndn_Signature *)context;
  ndn_Error error;
  size_t saveOffset;

  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_SignatureType,
        ndn_Tlv_SignatureType_SignatureSha256WithRsa)))
    return error;
  // Save the offset and set omitZeroLength true so we can detect if the key
  //   locator is omitted.  (When we remove the deprecated
  //   publisherPublicKeyDigest, we can call normally with omitZeroLength false.)
  saveOffset = encoder->offset;
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_KeyLocator, ndn_encodeTlvKeyLocatorValue,
        &signature->keyLocator, 1)))
    return error;
  if (encoder->offset == saveOffset) {
    // There is no keyLocator.  If there is a publisherPublicKeyDigest, then
    //   encode as KEY_LOCATOR_DIGEST.
    if (signature->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0) {
      if ((error = ndn_TlvEncoder_writeNestedTlv
           (encoder, ndn_Tlv_KeyLocator,
            encodeKeyLocatorPublisherPublicKeyDigestValue, signature, 0)))
        return error;
    }
    else {
      // Just encode an empty KeyLocator.
      if ((error = ndn_TlvEncoder_writeTypeAndLength
           (encoder, ndn_Tlv_KeyLocator, 0)))
        return error;
    }
  }

  return NDN_ERROR_success;
}

ndn_Error
ndn_encodeTlvSignatureInfo
  (struct ndn_Signature *signatureInfo, struct ndn_TlvEncoder *encoder)
{
  // TODO: The library needs to handle other signature types than Sha256WithRsa.
  return ndn_TlvEncoder_writeNestedTlv
    (encoder, ndn_Tlv_SignatureInfo, encodeSignatureSha256WithRsaValue,
     signatureInfo, 0);
}

ndn_Error
ndn_decodeTlvSignatureInfo
  (struct ndn_Signature *signatureInfo, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  uint64_t signatureType;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart
       (decoder, ndn_Tlv_SignatureInfo, &endOffset)))
    return error;

  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
       (decoder, ndn_Tlv_SignatureType, &signatureType)))
    return error;
  // TODO: The library needs to handle other signature types than
  //   SignatureSha256WithRsa.
  if (signatureType == ndn_Tlv_SignatureType_SignatureSha256WithRsa) {
    if ((error = ndn_decodeTlvKeyLocator
         (ndn_Tlv_KeyLocator, &signatureInfo->keyLocator, decoder)))
      return error;
    if (signatureInfo->keyLocator.type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST)
      // For backwards compatibility, also set the publisherPublicKeyDigest.
      signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest =
        signatureInfo->keyLocator.keyData;
    else
      // Set publisherPublicKeyDigest to none.
      ndn_Blob_initialize
        (&signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest, 0, 0);
  }
  else
    return NDN_ERROR_decodeSignatureInfo_unrecognized_SignatureInfo_type;

  // Set fields not used by NDN-TLV to none.
  ndn_Blob_initialize(&signatureInfo->digestAlgorithm, 0, 0);
  ndn_Blob_initialize(&signatureInfo->witness, 0, 0);

  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}
