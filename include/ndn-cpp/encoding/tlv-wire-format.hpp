/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_WIRE_FORMAT_HPP
#define NDN_TLV_WIRE_FORMAT_HPP

#include "tlv-0_1-wire-format.hpp"

namespace ndn {

/**
 * A TlvWireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using the preferred implementation of NDN-TLV.
 */
class TlvWireFormat : public Tlv0_1WireFormat {
public:
  /**
   * Get a singleton instance of a TlvWireFormat.  Assuming that the default wire format was set with
   * WireFormat::setDefaultWireFormat(TlvWireFormat::get()), you can check if this is the default wire encoding with
   * if (WireFormat::getDefaultWireFormat() == TlvWireFormat::get()).
   * @return A pointer to the singleton instance.
   */
  static TlvWireFormat* 
  get()
  {
    if (!instance_)
      instance_ = new TlvWireFormat();
    
    return instance_;
  }

  // Inherit encodeInterest, etc. from the base class.
  
private:
  static TlvWireFormat* instance_;
};

}

#endif
