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

#include <stdexcept>
#include <ndn-cpp/network-nack.hpp>
#include "incoming-face-id.hpp"
#include "lp-packet.hpp"

using namespace std;

namespace ndn {

LpPacketHeaderField::~LpPacketHeaderField() {}

void
LpPacket::set(const LpPacketLite& lpPacketLite)
{
  clear();

  for (size_t i = 0; i < lpPacketLite.countHeaderFields(); ++i) {
    const LpPacketHeaderFieldLite& field = lpPacketLite.getHeaderField(i);

    if (field.getType() == ndn_LpPacketHeaderFieldType_INCOMING_FACE_ID) {
      ptr_lib::shared_ptr<IncomingFaceId> incomingFaceId(new IncomingFaceId());
      incomingFaceId->set(field.getIncomingFaceId());
      headerFields_.push_back(incomingFaceId);
    }
    else if (field.getType() == ndn_LpPacketHeaderFieldType_NETWORK_NACK) {
      ptr_lib::shared_ptr<NetworkNack> networkNack(new NetworkNack());
      networkNack->set(field.getNetworkNack());
      headerFields_.push_back(networkNack);
    }
    else
      // We don't expect this to happen.
      throw runtime_error("lpPacketLite field.getType() has an unrecognized value");
  }

  fragmentWireEncoding_ = Blob(lpPacketLite.getFragmentWireEncoding());
}

}
