/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_KEY_LOCATOR_H
#define NDN_TLV_KEY_LOCATOR_H

#include "../../errors.h"
#include "../../key-locator.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the KeyLocator value.
 * @param context This is the ndn_KeyLocator struct pointer which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvKeyLocatorValue(void *context, struct ndn_TlvEncoder *encoder);

/**
 * Expect the next element to be a TLV KeyLocator and decode into the ndn_KeyLocator struct.
 * @param expectedType The expected type code of the TLV, e.g. ndn_Tlv_KeyLocator
 * or ndn_Tlv_PublisherPublicKeyLocator.
 * @param keyLocator A pointer to the ndn_KeyLocator struct.
 * @param decoder A pointer to the ndn_BinaryXmlDecoder struct.
 * @return 0 for success, else an error code, including if the next element is not KeyLocator.
 */
ndn_Error
ndn_decodeTlvKeyLocator
  (unsigned int expectedType, struct ndn_KeyLocator *keyLocator, 
   struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
