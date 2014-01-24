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
  
  /**
   * Decode the type from the input starting at offset, and if it is the expectedType,
   * then return true, else false.  However, if offset is greater than or equal to endOffset,
   * then return false and don't try to read the type.
   * Do not update offset, including if throwing an exception.
   * @param expectedType The expected type.
   * @param endOffset The offset of the end of the parent TLV.
   * @return True if got the expected type, else false.
   */
  bool 
  peekType(unsigned int expectedType, size_t endOffset) 
  {
    int gotExpectedType;
    ndn_Error error;
    if ((error = ndn_TlvDecoder_peekType(this, expectedType, endOffset, &gotExpectedType)))
      throw runtime_error(ndn_getErrorString(error));
    
    return gotExpectedType != 0 ? true : false;
  }  
};

}

#endif
