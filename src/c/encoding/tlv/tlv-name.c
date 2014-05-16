/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>

 * See COPYING for copyright and distribution information.
 */

#include "tlv-name.h"

ndn_Error 
ndn_encodeTlvName
  (struct ndn_Name *name, size_t *signedPortionBeginOffset, 
   size_t *signedPortionEndOffset, struct ndn_TlvEncoder *encoder)
{
  size_t nameValueLength = 0;
  size_t i;
  ndn_Error error;

  for (i = 0; i < name->nComponents; ++i)
    nameValueLength += ndn_TlvEncoder_sizeOfBlobTlv(ndn_Tlv_NameComponent, &name->components[i].value);

  if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_Name, nameValueLength)))
    return error;
  
  *signedPortionBeginOffset = encoder->offset;

  if (name->nComponents == 0)
    // There is no "final component", so set signedPortionEndOffset arbitrarily.
    *signedPortionEndOffset = *signedPortionBeginOffset;
  else {
    for (i = 0; i < name->nComponents; ++i) {
      if (i == name->nComponents - 1)
        // We will begin the final component.
        *signedPortionEndOffset = encoder->offset;
        
      if ((error = ndn_TlvEncoder_writeBlobTlv
           (encoder, ndn_Tlv_NameComponent, &name->components[i].value)))
        return error;
    }
  }
  
  return NDN_ERROR_success;
}

ndn_Error 
ndn_decodeTlvName(struct ndn_Name *name, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t endOffset;
  if ((error = ndn_TlvDecoder_readNestedTlvsStart(decoder, ndn_Tlv_Name, &endOffset)))
    return error;
    
  name->nComponents = 0;
  while (decoder->offset < endOffset) {
    struct ndn_Blob component;
    if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_NameComponent, &component)))
      return error;
    if ((error = ndn_Name_appendBlob(name, &component)))
      return error;
  }
   
  if ((error = ndn_TlvDecoder_finishNestedTlvs(decoder, endOffset)))
    return error;
  
  return NDN_ERROR_success;
}
