/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <math.h>
#include "tlv-name.h"
#include "tlv-interest.h"

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Selectors value.
 * @param context This is the ndn_Interest struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeSelectorsValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Interest *interest = (struct ndn_Interest *)context;
  
  ndn_Error error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_MinSuffixComponents, interest->minSuffixComponents)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_MaxSuffixComponents, interest->maxSuffixComponents)))
    return error;
  
  // TODO: Implement ndn_Tlv_PublisherPublicKeyLocator.
  // TODO: Implement ndn_Tlv_Exclude.
  
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_ChildSelector, interest->childSelector)))
    return error;  
  
  // Instead of using ndn_Interest_getMustBeFresh, check answerOriginKind directly so that we can
  //   return an error if unsupported bits are set.
  if (interest->answerOriginKind == 0) {
    // MustBeFresh == true.
    if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_MustBeFresh, 0)))
      return error;  
  }
  else if (interest->answerOriginKind < 0 || (interest->answerOriginKind & ndn_Interest_ANSWER_STALE) != 0) {
    // The default where MustBeFresh == false.
  }
  else
    // This error will be irrelevant when we drop support for binary XML answerOriginKind.
    return NDN_ERROR_Unsupported_answerOriginKind_bits_for_encoding_TLV_MustBeFresh;
  
  return NDN_ERROR_success;  
}

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Interest value.
 * @param context This is the ndn_Interest struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeInterestValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_Interest *interest = (struct ndn_Interest *)context;
  
  ndn_Error error;
  if ((error = ndn_encodeTlvName(&interest->name, encoder)))
    return error;
  // For Selectors, set omitZeroLength true.
  if ((error = ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Selectors, encodeSelectorsValue, interest, 1)))
    return error;

  // Encode the Nonce as 4 bytes.
  uint8_t nonceBuffer[4];    
  struct ndn_Blob nonceBlob;
  nonceBlob.length = 4;
  if (interest->nonce.length < 4) {
    // TLV encoding requires 4 bytes, so pad out to 4.
    ndn_memcpy(nonceBuffer, interest->nonce.value, interest->nonce.length);    
    nonceBlob.value = nonceBuffer;
  }
  else
    // TLV encoding requires 4 bytes, so truncate to 4.    
    nonceBlob.value = interest->nonce.value;
  if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_Nonce, &nonceBlob)))
    return error;    
  
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
      (encoder, ndn_Tlv_Scope, interest->scope)))
    return error;  
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlvFromDouble
      (encoder, ndn_Tlv_InterestLifetime, interest->interestLifetimeMilliseconds)))
    return error;  
  
  return NDN_ERROR_success;  
}

ndn_Error 
ndn_encodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvEncoder *encoder)
{
  return ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Interest, encodeInterestValue, interest, 0);
}

static ndn_Error
decodeSelectors(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_Selectors, &length)))
    return error;
  size_t endOffset = decoder->offset + length;

  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_MinSuffixComponents, endOffset, &interest->minSuffixComponents)))
    return error;
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_MaxSuffixComponents, endOffset, &interest->maxSuffixComponents)))
    return error;

  // TODO: Implement ndn_Tlv_PublisherPublicKeyLocator.
  // TODO: Implement ndn_Tlv_Exclude.
  
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_ChildSelector, endOffset, &interest->childSelector)))
    return error;

  int mustBeFresh;
  if ((error = ndn_TlvDecoder_readBooleanTlv(decoder, ndn_Tlv_MustBeFresh, endOffset, &mustBeFresh)))
    return error;
  // 0 means the ndn_Interest_ANSWER_STALE bit is not set. -1 is the default where mustBeFresh is false.
  interest->answerOriginKind = (mustBeFresh ? 0 : -1);

  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;  
}

ndn_Error 
ndn_decodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t length;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_Interest, &length)))
    return error;
  size_t endOffset = decoder->offset + length;
    
  if ((error = ndn_decodeTlvName(&interest->name, decoder)))
    return error;
    
  int gotExpectedType;
  if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_Selectors, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    if ((error = decodeSelectors(interest, decoder)))
      return error;
  }
  else {
    // Set selectors to none.
    interest->minSuffixComponents = -1;
    interest->maxSuffixComponents = -1;
    ndn_PublisherPublicKeyDigest_initialize(&interest->publisherPublicKeyDigest);
    interest->exclude.nEntries = 0;
    interest->childSelector = -1;
    interest->answerOriginKind = -1;    
  }

  // Require a Nonce, but don't force it to be 4 bytes.
  if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_Nonce, &interest->nonce)))
    return error;

  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlv
       (decoder, ndn_Tlv_Scope, endOffset, &interest->scope)))
    return error;
  if ((error = ndn_TlvDecoder_readOptionalNonNegativeIntegerTlvAsDouble
       (decoder, ndn_Tlv_InterestLifetime, endOffset, &interest->interestLifetimeMilliseconds)))
    return error;
  
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;

  return NDN_ERROR_success;
}
