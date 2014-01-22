/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_WIRE_FORMAT_HPP
#define NDN_TLV_WIRE_FORMAT_HPP

#include "tlv-0_1a2-wire-format.hpp"

namespace ndn {

/**
 * A TlvWireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using the preferred implementation of NDN-TLV.
 */
class TlvWireFormat : public Tlv1_0a2WireFormat {
  // Inherit encodeInterest, etc. from the base class.
};

}

#endif
