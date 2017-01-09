/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx fields.hpp https://github.com/named-data/ndn-cxx/blob/master/src/lp/fields.hpp
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

#ifndef NDN_INCOMING_FACE_ID_HPP
#define NDN_INCOMING_FACE_ID_HPP

#include <ndn-cpp/common.hpp>
#include <ndn-cpp/lite/lp/incoming-face-id-lite.hpp>
#include <ndn-cpp/lp-packet-header-field.hpp>

namespace ndn {

class LpPacket;

class IncomingFaceId : public LpPacketHeaderField {
public:
  /**
   * Create an IncomingFaceId where all the values are unspecified.
   */
  IncomingFaceId()
  : faceId_((uint64_t)-1)
  {
  }

  /**
   * Override the virtual destructor.
   */
  virtual ~IncomingFaceId();

  /**
   * Get the incoming face ID value.
   * @return The face ID value.
   */
  uint64_t
  getFaceId() const { return faceId_; }

  /**
   * Get the first header field in lpPacket which is an IncomingFaceId. This is
   * an internal method which the application normally would not use.
   * @param lpPacket The LpPacket with the header fields to search.
   * @return The first IncomingFaceId header field, or null if not found.
   */
  static ptr_lib::shared_ptr<IncomingFaceId>
  getFirstHeader(LpPacket& lpPacket);

  /**
   * Clear this IncomingFaceId, and set the values by copying from
   * incomingFaceIdLite.
   * @param incomingFaceIdLite An IncomingFaceIdLite object.
   */
  void
  set(const IncomingFaceIdLite& incomingFaceIdLite);

private:
  uint64_t faceId_; /**< (uint64_t)-1 if not specified. */
};

}

#endif
