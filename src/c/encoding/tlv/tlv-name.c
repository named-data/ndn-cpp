/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>

 * See COPYING for copyright and distribution information.
 */

#include "tlv-name.h"

ndn_Error 
ndn_encodeTlvName(struct ndn_Name *name, struct ndn_TlvEncoder *encoder)
{
  size_t nameValueLength = 0;
  size_t i;
  for (i = 0; i < name->nComponents; ++i)
    nameValueLength += ndn_TlvEncoder_sizeOfBlobTlv(ndn_Tlv_NameComponent, &name->components[i].value);

  ndn_Error error;
  if ((error = ndn_TlvEncoder_writeTypeAndLength(encoder, ndn_Tlv_Name, nameValueLength)))
    return error;
  
  for (i = 0; i < name->nComponents; ++i) {
    if ((error = ndn_TlvEncoder_writeBlobTlv(encoder, ndn_Tlv_NameComponent, &name->components[i].value)))
      return error;
  }
  
  return NDN_ERROR_success;
}

ndn_Error 
ndn_decodeTlvName(struct ndn_Name *name, struct ndn_TlvDecoder *decoder)
{
  ndn_Error error;
  size_t nameValueLength;
  if ((error = ndn_TlvDecoder_readTypeAndLength(decoder, ndn_Tlv_Name, &nameValueLength)))
    return error;
  size_t endOffset = decoder->offset + nameValueLength;
    
  name->nComponents = 0;
  while (decoder->offset < endOffset) {
    struct ndn_Blob component;
    if ((error = ndn_TlvDecoder_readBlobTlv(decoder, ndn_Tlv_NameComponent, &component)))
      return error;
    if ((error = ndn_Name_appendBlob(name, &component)))
      return error;
  }
   
  if (decoder->offset != endOffset)
    return NDN_ERROR_TLV_length_does_not_equal_total_length_of_nested_TLVs;
  
  return NDN_ERROR_success;
}
