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

ndn_Error 
ndn_encodeTlvInterest
  (struct ndn_Interest *interest, size_t *signedPortionBeginOffset, 
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder);

ndn_Error 
ndn_decodeTlvInterest(struct ndn_Interest *interest, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
