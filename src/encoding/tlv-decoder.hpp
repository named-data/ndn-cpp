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
};

}

#endif
