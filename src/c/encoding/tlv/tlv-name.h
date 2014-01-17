/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_NAME_H
#define NDN_TLV_NAME_H

#include "../../errors.h"
#include "../../name.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

ndn_Error 
ndn_encodeTlvName(struct ndn_Name *name, struct ndn_TlvEncoder *encoder);

ndn_Error 
ndn_decodeTlvName(struct ndn_Name *name, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
