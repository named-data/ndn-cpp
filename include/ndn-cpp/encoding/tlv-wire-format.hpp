/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#ifndef NDN_TLV_WIRE_FORMAT_HPP
#define NDN_TLV_WIRE_FORMAT_HPP

#include "tlv-0_1_1-wire-format.hpp"

namespace ndn {

/**
 * A TlvWireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using the preferred implementation of NDN-TLV.
 */
class TlvWireFormat : public Tlv0_1_1WireFormat {
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
