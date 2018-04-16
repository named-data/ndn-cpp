/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018 Regents of the University of California.
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

#ifndef NDN_CONGESTION_MARK_HPP
#define NDN_CONGESTION_MARK_HPP

#include <ndn-cpp/common.hpp>
#include <ndn-cpp/lite/lp/congestion-mark-lite.hpp>
#include <ndn-cpp/lp-packet-header-field.hpp>

namespace ndn {

class LpPacket;

/**
 * CongestionMark represents the congestion mark header field in an NDNLPv2
 * packet.
 * http://redmine.named-data.net/projects/nfd/wiki/NDNLPv2
 */
class CongestionMark : public LpPacketHeaderField {
public:
  /**
   * Create a CongestionMark where all the values are unspecified.
   */
  CongestionMark()
  : congestionMark_(0)
  {
  }

  /**
   * Override the virtual destructor.
   */
  virtual ~CongestionMark();

  /**
   * Get the congestion mark value.
   * @return The congestion mark  value.
   */
  uint64_t
  getCongestionMark() const { return congestionMark_; }

  /**
   * Get the first header field in lpPacket which is an CongestionMark. This is
   * an internal method which the application normally would not use.
   * @param lpPacket The LpPacket with the header fields to search.
   * @return The first CongestionMark header field, or null if not found.
   */
  static ptr_lib::shared_ptr<CongestionMark>
  getFirstHeader(LpPacket& lpPacket);

  /**
   * Clear this CongestionMark, and set the values by copying from
   * congestionMarkLite.
   * @param congestionMarkLite An CongestionMarkLite object.
   */
  void
  set(const CongestionMarkLite& congestionMarkLite);

private:
  uint64_t congestionMark_; /**< 0 if not specified. */
};

}

#endif
