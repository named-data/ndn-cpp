/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "tlv-name.h"
#include "tlv-data.h"

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
    
  return NDN_ERROR_success;  
}

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the KeyLocator value.
 * @param context This is the ndn_Signature struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeKeyLocatorValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Signature *signatureInfo = (struct ndn_Signature *)context;
  
  ndn_Error error;
  // If the KeyName is present, use it instead of PublisherPublicKeyDigest.
  if (signatureInfo->keyLocator.type == ndn_KeyLocatorType_KEYNAME) {
    if ((error = ndn_encodeTlvName(&signatureInfo->keyLocator.keyName, encoder)))
      return error;
  }
  else if (signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0) {
    if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_KeyLocatorDigest, &signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest)))
      return error;  
  }
  else
    return NDN_ERROR_unrecognized_ndn_KeyLocatorType;

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
  // SignatureType 1 is SignatureSha256WithRsa.
  if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv(encoder, ndn_Tlv_SignatureType, 1)))
    return error;
  if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_KeyLocator, encodeKeyLocatorValue, signature, 0)))
    return error;  

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
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_MetaInfo, &length)))
    return error;
  size_t endOffset = decoder->offset + length;

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

  // Set fields not used by NDN-TLV to none.
  metaInfo->timestampMilliseconds = -1;
  ndn_NameComponent_initialize(&metaInfo->finalBlockID, 0, 0);  
  
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;    
}

static ndn_Error
decodeKeyLocator(struct ndn_Signature *signatureInfo, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_KeyLocator, &length)))
    return error;
  size_t endOffset = decoder->offset + length;

  int gotExpectedType;
  if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_Name, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    // KeyLocator is a Name.
    if ((error = ndn_decodeTlvName(&signatureInfo->keyLocator.keyName, decoder)))
      return error;
    signatureInfo->keyLocator.type = ndn_KeyLocatorType_KEYNAME;
    signatureInfo->keyLocator.keyNameType = (ndn_KeyNameType)-1;
    ndn_Blob_initialize(&signatureInfo->keyLocator.keyData, 0, 0);
    
    // Set publisherPublicKeyDigest to none.
    ndn_Blob_initialize(&signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest, 0, 0);
  }
  else {
    if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_KeyLocatorDigest, endOffset, &gotExpectedType)))
      return error;
    if (gotExpectedType) {
      // KeyLocator is a KeyLocatorDigest (a publisherPublicKeyDigest).
      if ((error = ndn_TlvDecoder_readBlobTlv
           (decoder, ndn_Tlv_KeyLocatorDigest, &signatureInfo->publisherPublicKeyDigest.publisherPublicKeyDigest)))
        return error;
      
      // Set the KeyLocator with its key name to none.
      signatureInfo->keyLocator.type = (ndn_KeyLocatorType)-1;
      signatureInfo->keyLocator.keyNameType = (ndn_KeyNameType)-1;      
      ndn_Blob_initialize(&signatureInfo->keyLocator.keyData, 0, 0);
    }
    else
      return NDN_ERROR_decodeKeyLocator_unrecognized_key_locator_type;
  }
  
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;    
}

static ndn_Error
decodeSignatureInfo(struct ndn_Signature *signatureInfo, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_SignatureInfo, &length)))
    return error;
  size_t endOffset = decoder->offset + length;

  uint64_t signatureType;
  if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv(decoder, ndn_Tlv_SignatureType, &signatureType)))
    return error;
  // TODO: The library needs to handle other signature types than SignatureSha256WithRsa.
  if (signatureType == 1) {
    // SignatureType 1 is SignatureSha256WithRsa.
    if ((error = decodeKeyLocator(signatureInfo, decoder)))
      return error;
  }
  else
    return NDN_ERROR_decodeSignatureInfo_unrecognized_SignatureInfo_type;
  
  // Set fields not used by NDN-TLV to none.
  ndn_Blob_initialize(&signatureInfo->digestAlgorithm, 0, 0);
  ndn_Blob_initialize(&signatureInfo->witness, 0, 0);
  
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;    
}

ndn_Error 
ndn_decodeTlvData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_Data, &length)))
    return error;
  size_t endOffset = decoder->offset + length;
    
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
      
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;
}
