/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_PREFIX_REGISTRATION_OPTIONS_H
#define NDN_TLV_PREFIX_REGISTRATION_OPTIONS_H

#include "../../errors.h"
#include "../../prefix-registration-options.h"
#include "tlv-encoder.h"
#include "tlv-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode prefixRegistrationOptions as an NDN-TLV PrefixRegOptions.
 * @param prefixRegistrationOptions A pointer to the 
 * ndn_PrefixRegistrationOptions struct to encode.
 * @param encoder Pointer to the ndn_TlvEncoder struct which receives the 
 * encoding. 
 * @return 0 for success, else an error code.
 */
ndn_Error 
ndn_encodeTlvPrefixRegOptions
  (struct ndn_PrefixRegistrationOptions *prefixRegistrationOptions, 
   struct ndn_TlvEncoder *encoder);

// TODO: ndn_decodePrefixRegOptions

#ifdef __cplusplus
}
#endif

#endif
