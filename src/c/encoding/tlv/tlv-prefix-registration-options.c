/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#include <ndn-cpp/c/forwarding-flags.h>
#include "tlv-name.h"
#include "tlv-prefix-registration-options.h"

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs 
 * in the body of the PrefixRegOptions value.
 * @param context This is the ndn_PrefixRegistrationOptions struct pointer which 
 * was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodePrefixRegOptionsValue
  (void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_PrefixRegistrationOptions *prefixRegistrationOptions = 
    (struct ndn_PrefixRegistrationOptions *)context;
  
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_encodeTlvName
       (&prefixRegistrationOptions->prefix, &dummyBeginOffset, &dummyEndOffset, 
        encoder)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_PrefixRegOptions_FaceId, 
        prefixRegistrationOptions->faceId)))
    return error;
  
  struct ndn_ForwardingFlags defaultFlags;
  ndn_ForwardingFlags_initialize(&defaultFlags);
  int flags = ndn_ForwardingFlags_getForwardingEntryFlags(&defaultFlags);
  if (ndn_ForwardingFlags_getForwardingEntryFlags
      (&prefixRegistrationOptions->flags) != flags) {
    // The flags are not the default value.
    if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
         (encoder, ndn_Tlv_PrefixRegOptions_Flags, flags)))
      return error;    
  }
  
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_PrefixRegOptions_Cost, 
        prefixRegistrationOptions->cost)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlvFromDouble
       (encoder, ndn_Tlv_PrefixRegOptions_ExpirationPeriod, 
        prefixRegistrationOptions->expirationPeriod)))
    return error;
  
  // TODO: Encode Protocol.
  
  return NDN_ERROR_success;  
}

ndn_Error 
ndn_encodeTlvPrefixRegOptions
  (struct ndn_PrefixRegistrationOptions *prefixRegistrationOptions, 
   struct ndn_TlvEncoder *encoder)
{
  return ndn_TlvEncoder_writeNestedTlv
    (encoder, ndn_Tlv_PrefixRegOptions_PrefixRegOptions, 
     encodePrefixRegOptionsValue, prefixRegistrationOptions, 0);
}
