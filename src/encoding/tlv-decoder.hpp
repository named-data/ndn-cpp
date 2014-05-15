/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_DECODER_HPP
#define NDN_TLV_DECODER_HPP

#include <stdexcept>
#include "../c/errors.h"
#include "../c/encoding/tlv/tlv-decoder.h"

using namespace std;

namespace ndn {

/**
 * A TlvDecoder extends a C ndn_TlvDecoder struct and wraps related functions.
 */
class TlvDecoder : public ndn_TlvDecoder {
public:
  /**
   * Initialize the base ndn_TlvDecoder struct with the input.
   */
  TlvDecoder(const uint8_t *input, size_t inputLength) 
  {
    ndn_TlvDecoder_initialize(this, input, inputLength);
  }
  
  size_t
  readNestedTlvsStart(unsigned int expectedType)
  {
    size_t endOffset;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_readNestedTlvsStart
         (this, expectedType, &endOffset)))
      throw runtime_error(ndn_getErrorString(error));
    
    return endOffset;
  }
  
  void
  finishNestedTlvs(int endOffset)
  {
    ndn_Error error;
    if ((error = ndn_TlvDecoder_finishNestedTlvs(this, endOffset)))
      throw runtime_error(ndn_getErrorString(error));
  }
  
  bool 
  peekType(unsigned int expectedType, size_t endOffset) 
  {
    int gotExpectedType;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_peekType
         (this, expectedType, endOffset, &gotExpectedType)))
      throw runtime_error(ndn_getErrorString(error));
    
    return gotExpectedType != 0 ? true : false;
  }  
  
  uint64_t
  readNonNegativeIntegerTlv(unsigned int expectedType)
  {
    uint64_t value;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_readNonNegativeIntegerTlv
         (this, expectedType, &value)))
      throw runtime_error(ndn_getErrorString(error));
    
    return value;
  }
  
  struct ndn_Blob
  readBlobTlv(unsigned int expectedType)
  {
    struct ndn_Blob value;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_readBlobTlv(this, expectedType, &value)))
      throw runtime_error(ndn_getErrorString(error));
    
    return value;
  }
  
  bool
  readBooleanTlv(unsigned int expectedType, size_t endOffset)
  {
    int value;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_readBooleanTlv
         (this, expectedType, endOffset, &value)))
      throw runtime_error(ndn_getErrorString(error));
    
    return value != 0;
  }
};

}

#endif
