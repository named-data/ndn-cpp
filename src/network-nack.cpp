/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include "lp/lp-packet.hpp"
#include <ndn-cpp/network-nack.hpp>

namespace ndn {

NetworkNack::~NetworkNack() {}

ptr_lib::shared_ptr<NetworkNack>
NetworkNack::getFirstHeader(LpPacket& lpPacket)
{
  for (size_t i = 0; i < lpPacket.countHeaderFields(); ++i) {
    ptr_lib::shared_ptr<NetworkNack> field =
      ptr_lib::dynamic_pointer_cast<NetworkNack>(lpPacket.getHeaderField(i));
    if (field)
      return field;
  }

  return ptr_lib::shared_ptr<NetworkNack>();
}

void
NetworkNack::set(const NetworkNackLite& networkNackLite)
{
  reason_ = networkNackLite.getReason();
  otherReasonCode_ = networkNackLite.getOtherReasonCode();
}

}
