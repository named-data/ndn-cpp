/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "tlv-name.h"
#include "tlv-key-locator.h"

ndn_Error 
ndn_encodeTlvKeyLocatorValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_KeyLocator *keyLocator = (struct ndn_KeyLocator *)context;
  ndn_Error error;

  if ((int)keyLocator->type < 0)
    return NDN_ERROR_success;
  
  if (keyLocator->type == ndn_KeyLocatorType_KEYNAME) {
    size_t dummyBeginOffset, dummyEndOffset;
    if ((error = ndn_encodeTlvName
         (&keyLocator->keyName, &dummyBeginOffset, &dummyEndOffset, encoder)))
      return error;
  }
  else if (keyLocator->type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST && keyLocator->keyData.length > 0) {
    if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_KeyLocatorDigest, &keyLocator->keyData)))
      return error;  
  }
  else
    return NDN_ERROR_unrecognized_ndn_KeyLocatorType;

  return NDN_ERROR_success;  
}

ndn_Error
ndn_decodeTlvKeyLocator
  (unsigned int expectedType, struct ndn_KeyLocator *keyLocator, 
   struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  int gotExpectedType;

  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, expectedType, &endOffset)))
    return error;

  ndn_KeyLocator_initialize
    (keyLocator, keyLocator->keyName.components, keyLocator->keyName.maxComponents);

  if (decoder->offset == endOffset)
    // The KeyLocator is omitted, so leave the fields as none.
    return NDN_ERROR_success;

  if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_Name, endOffset, &gotExpectedType)))
    return error;
  if (gotExpectedType) {
    // KeyLocator is a Name.
    if ((error = ndn_decodeTlvName(&keyLocator->keyName, decoder)))
      return error;
    keyLocator->type = ndn_KeyLocatorType_KEYNAME;
  }
  else {
    if ((error = ndn_TlvDecoder_peekType(decoder, ndn_Tlv_KeyLocatorDigest, endOffset, &gotExpectedType)))
      return error;
    if (gotExpectedType) {
      // KeyLocator is a KeyLocatorDigest.
      keyLocator->type = ndn_KeyLocatorType_KEY_LOCATOR_DIGEST;
      if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_KeyLocatorDigest, &keyLocator->keyData)))
        return error;
    }
    else
      return NDN_ERROR_decodeKeyLocator_unrecognized_key_locator_type;
  }
  
  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;

  return NDN_ERROR_success;
}
