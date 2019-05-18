/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx nack.hpp https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/lp/nack.hpp
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

#ifndef NDN_NETWORK_NACK_HPP
#define NDN_NETWORK_NACK_HPP

#include <stdexcept>
#include "common.hpp"
#include "lite/network-nack-lite.hpp"
#include "lp-packet-header-field.hpp"

namespace ndn {

class LpPacket;

class NetworkNack : public LpPacketHeaderField {
public:
  /**
   * Create an NetworkNack where all the values are unspecified.
   */
  NetworkNack()
  : reason_(ndn_NetworkNackReason_NONE),
    otherReasonCode_(-1)
  {
  }

  /**
   * Override the virtual destructor.
   */
  virtual ~NetworkNack();

  /**
   * Get the network Nack reason.
   * @return The reason enum value. If this is ndn_NetworkNackReason_OTHER_CODE,
   * then call getOtherReasonCode() to get the unrecognized reason code.
   */
  ndn_NetworkNackReason
  getReason() const { return reason_; }

  /**
   * Get the reason code from the packet which is other than a recognized
   * Reason enum value. This is only meaningful if getReason() is
   * ndn_NetworkNackReason_OTHER_CODE.
   * @return The reason code.
   */
  int
  getOtherReasonCode() const { return otherReasonCode_; }

  /**
   * Set the network Nack reason.
   * @param reason The network Nack reason enum value. If the packet's reason
   * code is not a recognized Reason enum value, use 
   * ndn_NetworkNackReason_OTHER_CODE and call setOtherReasonCode().
   * @return This NetworkNack so that you can chain calls to update values.
   */
  NetworkNack&
  setReason(ndn_NetworkNackReason reason)
  {
    reason_ = reason;
    return *this;
  }

  /**
   * Set the packet's reason code to use when the reason enum is
   * ndn_NetworkNackReason_OTHER_CODE. If the packet's reason code is a
   * recognized enum value, just call setReason().
   * @param otherReasonCode The packet's unrecognized reason code, which must be
   * non-negative.
   * @return This NetworkNack so that you can chain calls to update values.
   */
  NetworkNack&
  setOtherReasonCode(int otherReasonCode)
  {
    if (otherReasonCode < 0)
      throw std::runtime_error("NetworkNack other reason code must be non-negative");
    otherReasonCode_ = otherReasonCode;
    return *this;
  }

  /**
   * Get the first header field in lpPacket which is a NetworkNack. This is
   * an internal method which the application normally would not use.
   * @param lpPacket The LpPacket with the header fields to search.
   * @return The first NetworkNack header field, or null if not found.
   */
  static ptr_lib::shared_ptr<NetworkNack>
  getFirstHeader(LpPacket& lpPacket);

  /**
   * Clear this NetworkNack, and set the values by copying from
   * networkNackLite.
   * @param networkNackLite An NetworkNackLite object.
   */
  void
  set(const NetworkNackLite& networkNackLite);

private:
  ndn_NetworkNackReason reason_;
  int otherReasonCode_;
};

}

#endif
