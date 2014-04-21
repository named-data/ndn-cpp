/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

#include <ndn-cpp/c/forwarding-flags.h>
#include "tlv-name.h"
#include "tlv-control-parameters.h"

/**
 * This private function is called by ndn_TlvEncoder_writeTlv to write the TLVs 
 * in the body of the ControlParameters value.
 * @param context This is the ndn_ControlParameters struct pointer which 
 * was passed to writeTlv.
 * @param encoder the ndn_TlvEncoder which is calling this.
 * @return 0 for success, else an error code.
 */
static ndn_Error 
encodeControlParametersValue(void *context, struct ndn_TlvEncoder *encoder)
{
  struct ndn_ControlParameters *controlParameters = 
    (struct ndn_ControlParameters *)context;
  
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;
  if ((error = ndn_encodeTlvName
       (&controlParameters->name, &dummyBeginOffset, &dummyEndOffset, 
        encoder)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_ControlParameters_FaceId, 
        controlParameters->faceId)))
    return error;  

  // TODO: Encode Uri.

  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_ControlParameters_LocalControlFeature, 
        controlParameters->localControlFeature)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_ControlParameters_Origin, 
        controlParameters->origin)))
    return error;
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlv
       (encoder, ndn_Tlv_ControlParameters_Cost, 
        controlParameters->cost)))
    return error;
  
  struct ndn_ForwardingFlags defaultFlags;
  ndn_ForwardingFlags_initialize(&defaultFlags);
  int flags = ndn_ForwardingFlags_getForwardingEntryFlags(&defaultFlags);
  if (ndn_ForwardingFlags_getForwardingEntryFlags
      (&controlParameters->flags) != flags) {
    // The flags are not the default value.
    if ((error = ndn_TlvEncoder_writeNonNegativeIntegerTlv
         (encoder, ndn_Tlv_ControlParameters_Flags, flags)))
      return error;    
  }
  if ((error = ndn_TlvEncoder_writeOptionalNonNegativeIntegerTlvFromDouble
       (encoder, ndn_Tlv_ControlParameters_ExpirationPeriod, 
        controlParameters->expirationPeriod)))
    return error;
  
  // TODO: Encode Strategy.
  
  return NDN_ERROR_success;  
}

ndn_Error 
ndn_encodeTlvControlParameters
  (struct ndn_ControlParameters *controlParameters, 
   struct ndn_TlvEncoder *encoder)
{
  return ndn_TlvEncoder_writeNestedTlv
    (encoder, ndn_Tlv_ControlParameters_ControlParameters, 
     encodeControlParametersValue, controlParameters, 0);
}
