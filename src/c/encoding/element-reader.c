/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "element-reader.h"

ndn_Error ndn_ElementReader_onReceivedData
  (struct ndn_ElementReader *self, uint8_t *data, size_t dataLength)
{
  // Process multiple objects in the data.
  while(1) {
    if (!self->usePartialData) {
      // This is the beginning of an element.  Check whether it is binaryXML or TLV.
      if (dataLength <= 0)
        // Wait for more data.
        return NDN_ERROR_success;
      
      // A TLV Data packet starts with 0x02.
      // A TLV Interest starts with 0x01, but need to distinguish from a NDNb Interest which starts with 0x01 0xD2 0xF2.
      // (If a TLV Interest had a length of 0xD2, it would always start with 0x01 0xD2 0x03.)
      // If the first byte is 0x01, we have to make sure dataLength is large enough to check further bytes.
      if (data[0] == 0x02 ||
          (data[0] == 0x01 && !(dataLength >= 2 && data[1] == 0xD2 && dataLength >= 3 && data[2] == 0xF2)))
        self->useTlv = 1;
      else
        // Binary XML.
        self->useTlv = 0;
    }
    
    ndn_Error error;
    int gotElementEnd;
    size_t offset;
    if (self->useTlv) {
      // Scan the input to check if a whole TLV element has been read.
      ndn_TlvStructureDecoder_seek(&self->tlvStructureDecoder, 0);    
      if ((error = ndn_TlvStructureDecoder_findElementEnd(&self->tlvStructureDecoder, data, dataLength)))
        return error;
      gotElementEnd = self->tlvStructureDecoder.gotElementEnd;
      offset = self->tlvStructureDecoder.offset;
    }
    else {
      // Scan the input to check if a whole binary XML element has been read.
      ndn_BinaryXmlStructureDecoder_seek(&self->binaryXmlStructureDecoder, 0);    
      if ((error = ndn_BinaryXmlStructureDecoder_findElementEnd(&self->binaryXmlStructureDecoder, data, dataLength)))
        return error;
      gotElementEnd = self->binaryXmlStructureDecoder.gotElementEnd;
      offset = self->binaryXmlStructureDecoder.offset;
    }
    
    if (gotElementEnd) {
      // Got the remainder of an element.  Report to the caller.
      if (self->usePartialData) {
        // We have partial data from a previous call, so append this data and point to partialData.
        if ((error = ndn_DynamicUInt8Array_set(&self->partialData, data, offset, self->partialDataLength)))
          return error;
        self->partialDataLength += dataLength;
                
        (*self->elementListener->onReceivedElement)(self->elementListener, self->partialData.array, self->partialDataLength);
        // Assume we don't need to use partialData anymore until needed.
        self->usePartialData = 0;
      }
      else
        // We are not using partialData, so just point to the input data buffer.
        (*self->elementListener->onReceivedElement)(self->elementListener, data, offset);
        
      // Need to read a new object.
      data += offset;
      dataLength -= offset;
      ndn_BinaryXmlStructureDecoder_initialize(&self->binaryXmlStructureDecoder);
      ndn_TlvStructureDecoder_initialize(&self->tlvStructureDecoder);
      if (dataLength == 0)
        // No more data in the packet.
        return NDN_ERROR_success;
            
      // else loop back to decode.
    }
    else {
      // Save remaining data for a later call.
      if (!self->usePartialData) {
        self->usePartialData = 1;
        self->partialDataLength = 0;
      }
      
      if ((error = ndn_DynamicUInt8Array_set(&self->partialData, data, dataLength, self->partialDataLength)))
        return error;
      self->partialDataLength += dataLength;
      
      return NDN_ERROR_success;
    }
  }      
}
