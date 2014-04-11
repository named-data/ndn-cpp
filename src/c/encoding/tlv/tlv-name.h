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

/**
 * Encode the name as NDN-TLV.
 * @param name A pointer to the name object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the 
 * beginning of the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of 
 * the signed portion. The signed portion starts from the first
 * name component and ends just before the final name component (which is
 * assumed to be a signature for a signed interest).
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param encoder A pointer to the ndn_BinaryXmlEncoder struct which receives 
 * the encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvName
  (struct ndn_Name *name, size_t *signedPortionBeginOffset, 
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder);

ndn_Error 
ndn_decodeTlvName(struct ndn_Name *name, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
