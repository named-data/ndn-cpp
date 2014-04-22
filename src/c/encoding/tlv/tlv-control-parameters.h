/**
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_CONTROL_PARAMETERS_OPTIONS_H
#define NDN_TLV_CONTROL_PARAMETERS_OPTIONS_H

#include "../../errors.h"
#include "../../control-parameters.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode controlParameters as an NDN-TLV ControlParameters.
 * @param controlParameters A pointer to the 
 * ndn_ControlParameters struct to encode.
 * @param encoder Pointer to the ndn_TlvEncoder struct which receives the 
 * encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvControlParameters
  (struct ndn_ControlParameters *controlParameters, 
   struct ndn_TlvEncoder *encoder);

// TODO: ndn_decodeControlParameters

#ifdef __cplusplus
}
#endif

#endif
