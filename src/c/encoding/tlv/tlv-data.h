/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_DATA_H
#define NDN_TLV_DATA_H

#include "../../errors.h"
#include "../../data.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Encode the data packet as NDN-TLV.
 * @param data Pointer to the data object to encode.
 * @param signedPortionBeginOffset Return the offset in the encoding of the beginning of the signed portion.
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the encoding of the end of the signed portion.
 * If you are not encoding in order to sign, you can ignore this returned value.
 * @param encoder Pointer to the ndn_TlvEncoder struct which receives the encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder);

/**
 * Decode the data packet as NDN-TLV and set the fields in the data object.
 * @param data Pointer to the data object whose fields are updated.
 * @param signedPortionBeginOffset Return the offset in the input buffer of the beginning of the signed portion.
 * If you are not decoding in order to verify, you can ignore this returned value.
 * @param signedPortionEndOffset Return the offset in the input buffer of the end of the signed portion.
 * If you are not decoding in order to verify, you can ignore this returned value.
 * @param decoder Pointer to the ndn_TlvDecoder struct which has been initialized with the buffer to decode. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_decodeTlvData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_TlvDecoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif
