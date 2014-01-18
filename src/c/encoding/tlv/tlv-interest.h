/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_INTEREST_H
#define NDN_TLV_INTEREST_H

#include "../../errors.h"
#include "../../interest.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs in the body of the Interest value.
 * @param context This is the ndn_Interest struct which was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvInterestValue(void *context, struct ndn_TlvEncoder *encoder);

static inline ndn_Error 
ndn_encodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvEncoder *encoder)
{
  return ndn_TlvEncoder_writeNestedTlv(encoder, ndn_Tlv_Interest, ndn_encodeTlvInterestValue, interest, 0);
}

ndn_Error 
ndn_decodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
