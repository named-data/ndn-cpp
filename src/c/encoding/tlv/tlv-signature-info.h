/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_SIGNATURE_INFO_H
#define NDN_TLV_SIGNATURE_INFO_H

#include "../../errors.h"
#include "../../data.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode signatureInfo as an NDN-TLV SignatureInfo.
 * @param signatureInfo A pointer to the ndn_Signature struct to encode.
 * @param encoder A pointer to the ndn_TlvEncoder struct which receives the 
 * encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvSignatureInfo
  (struct ndn_Signature *signatureInfo, struct ndn_TlvEncoder *encoder);

/**
 * Decode as an NDN-TLV SignatureInfo and set the fields in signatureInfo.
 * @param signatureInfo A pointer to the ndn_Signature struct to receive the
 * values.
 * @param decoder A pointer to the ndn_TlvDecoder struct which has been 
 * initialized with the buffer to decode. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_decodeTlvSignatureInfo
  (struct ndn_Signature *signatureInfo, struct ndn_TlvDecoder *decoder);

#ifdef __cplusplus
}
#endif

#endif
