/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "tlv-name.h"
#include "tlv-key-locator.h"
#include "tlv-data.h"

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
       (encoder, ndn_Tlv_KeyLocatorDigest, &signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest)))
    return error;  

  return NDN_ERROR_success;  
}

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the MetaInfo value.
 * @param context This is the ndn_MetaInfo struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeMetaInfoValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_MetaInfo *metaInfo = (struct ndn_MetaInfo *)context;
  
  ndn_Error error;

  if (!((int)metaInfo->type < 0 || metaInfo->type == ndn_ContentType_BLOB)) {
    // Not the default, so we need to encode the type.
    if (metaInfo->type == ndn_ContentType_LINK || metaInfo->type == ndn_ContentType_KEY) {
      // The ContentType enum is set up with the correct integer for each NDN-TLV ContentType.
      if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
          (encoder, ndn_Tlv_ContentType, metaInfo->type)))
        return error;
    }
    else
      return NDN_ERROR_unrecognized_ndn_ContentType;
  }

  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlvFromDouble
      (encoder, ndn_Tlv_FreshnessPeriod, metaInfo->freshnessPeriod)))
    return error;
  if (metaInfo->finalBlockID.value.value && 
      metaInfo->finalBlockID.value.length > 0) {
    // The FinalBlockID has an inner NameComponent.
    if ((error = ndn_TlvEncoder_writeTypeAndLength
         (encoder, ndn_Tlv_FinalBlockId, ndn_TlvEncoder_sizeOfBlobTlv
            (ndn_Tlv_NameComponent, &metaInfo->finalBlockID.value))))
      return error;
    if ((error = ndn_TlvEncoder_writeBlobTlv
         (encoder, ndn_Tlv_NameComponent, &metaInfo->finalBlockID.value)))
      return error;    
  }
    
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
  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
       (encoder, ndn_Tlv_SignatureType, 
        ndn_Tlv_SignatureType_SignatureSha256WithRsa)))
    return error;
  // Save the offset and set omitZeroLength true so we can detect if the key locator is omitted.  (When we remove
  //   the deprecated publisherPublicKeyDigest, we can call normally with omitZeroLength false.)
  size_t saveOffset = encoder->offset;
  if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_KeyLocator, ndn_encodeTlvKeyLocatorValue, &signature->keyLocator, 1)))
    return error;
  if (encoder->offset == saveOffset) {
    // There is no keyLocator.  If there is a publisherPublicKeyDigest, the encode as KEY_LOCATOR_DIGEST.
    if (signature->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0) {
      if ((error = ndn_TlvEncoder_writeNestedTlv
           (encoder, ndn_Tlv_KeyLocator, encodeKeyLocatorPublisherPublicKeyDigestValue, signature, 0)))
        return error;
    }
    else {
      // Just encode an empty KeyLocator.
      if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_KeyLocator, 0)))
        return error;
    }
  }

  return NDN_ERROR_success;  
}

/* An DataValueContext is for passing the context to encodeDataValue so that we can include
 * signedPortionBeginOffset and signedPortionEndOffset.
 */
struct DataValueContext {
  struct ndn_Data *data;
  size_t *signedPortionBeginOffset;
  size_t *signedPortionEndOffset;
};

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Data value.
 * @param context This is the DataValueContext struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeDataValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct DataValueContext *dataValueContext = (struct DataValueContext *)context;
  struct ndn_Data *data = dataValueContext->data;
  
  *dataValueContext->signedPortionBeginOffset = encoder->offset;
  
  ndn_Error error;
  if ((error = ndn_encodeTlvName(&data->name, encoder)))
    return error;
  if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_MetaInfo, encodeMetaInfoValue, &data->metaInfo, 0)))
    return error;
  if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_Content, &data->content)))
    return error;
  
  // TODO: The library needs to handle other signature types than SignatureSha256WithRsa.
  if ((error = ndn_TlvEncoder_writeNestedTlv
       (encoder, ndn_Tlv_SignatureInfo, encodeSignatureSha256WithRsaValue, &data->signature, 0)))
    return error;  

  *dataValueContext->signedPortionEndOffset = encoder->offset;

  if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_SignatureValue, &data->signature.signature)))
    return error;  
  
  return NDN_ERROR_success;  
}

ndn_Error 
ndn_encodeTlvData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder)
{
  // Create the context to pass to encodeDataValue.
  struct DataValueContext dataValueContext;
  dataValueContext.data = data;
  dataValueContext.signedPortionBeginOffset = signedPortionBeginOffset;
  dataValueContext.signedPortionEndOffset = signedPortionEndOffset;
  
  return ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Data, encodeDataValue, &dataValueContext, 0);
}

static ndn_Error
decodeMetaInfo(struct ndn_MetaInfo *metaInfo, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_MetaInfo, &endOffset)))
    return error;

  // The ContentType enum is set up with the correct integer for each NDN-TLV ContentType.
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_ContentType, endOffset, (int *)&metaInfo->type)))
    return error;
  if ((int)metaInfo->type < 0)
    // Set to the actual value for the default.
    metaInfo->type = ndn_ContentType_BLOB;
  
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlvAsDouble
       (decoder, ndn_Tlv_FreshnessPeriod, endOffset, &metaInfo->freshnessPeriod)))
    return error;
  
  int gotExpectedType;
  if ((error = ndn_TlvDecoder_peekType
       (decoder, ndn_Tlv_FinalBlockId, endOffset, &gotExpectedType)))
    return error;    
  if (gotExpectedType) {
    size_t finalBlockIdEndOffset;
    if ((error = ndn_TlvDecoder_readNestedTlvsStart
         (decoder, ndn_Tlv_FinalBlockId, &finalBlockIdEndOffset)))
      return error;
    if ((error = ndn_TlvDecoder_readBlobTlv
         (decoder, ndn_Tlv_NameComponent, &metaInfo->finalBlockID.value)))
      return error;
    if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, finalBlockIdEndOffset)))
      return error;
  }
  else
    ndn_NameComponent_initialize(&metaInfo->finalBlockID, 0, 0);

  // Set fields not used by NDN-TLV to none.
  metaInfo->timestampMilliseconds = -1;
  
  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;    
}

static ndn_Error
decodeSignatureInfo(struct ndn_Signature *signatureInfo, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_SignatureInfo, &endOffset)))
    return error;

  uint64_t signatureType;
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv(decoder, ndn_Tlv_SignatureType, &signatureType)))
    return error;
  // TODO: The library needs to handle other signature types than 
  //   SignatureSha256WithRsa.
  if (signatureType == ndn_Tlv_SignatureType_SignatureSha256WithRsa) {
    if ((error = ndn_decodeTlvKeyLocator(&signatureInfo->keyLocator, decoder)))
      return error;
    if (signatureInfo->keyLocator.type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST)
      // For backwards compatibility, also set the publisherPublicKeyDigest.
      signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest = signatureInfo->keyLocator.keyData;
    else
      // Set publisherPublicKeyDigest to none.
      ndn_Blob_initialize(&signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest, 0, 0);      
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

ndn_Error 
ndn_decodeTlvData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_Data, &endOffset)))
    return error;
    
  *signedPortionBeginOffset = decoder->offset;

  if ((error = ndn_decodeTlvName(&data->name, decoder)))
    return error;
  if ((error = decodeMetaInfo(&data->metaInfo, decoder)))
    return error;
  if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_Content, &data->content)))
    return error;
  if ((error = decodeSignatureInfo(&data->signature, decoder)))
    return error;
  
  *signedPortionEndOffset = decoder->offset;
  
  if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_SignatureValue, &data->signature.signature)))
    return error;  
      
  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}
