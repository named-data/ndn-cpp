/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx packet.hpp https://github.com/named-data/ndn-cxx/blob/master/src/lp/packet.hpp
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

#ifndef NDN_LP_PACKET_HPP
#define NDN_LP_PACKET_HPP

#include <vector>
#include <ndn-cpp/lite/lp/lp-packet-lite.hpp>
#include <ndn-cpp/lp-packet-header-field.hpp>
#include <ndn-cpp/util/blob.hpp>

namespace ndn {

/**
 * An LpPacket represents an NDNLPv2 packet including header fields an an
 * optional fragment. This is an internal class which the application normally
 * would not use.
 * http://redmine.named-data.net/projects/nfd/wiki/NDNLPv2
 */
class LpPacket {
public:
  /**
   * Get the fragment wire encoding.
   * @return The wire encoding, or an isNull Blob if not specified.
   */
  const Blob&
  getFragmentWireEncoding() const { return fragmentWireEncoding_; }

  /**
   * Get the number of header fields. This does not include the fragment.
   * @return The number of header fields.
   */
  size_t
  countHeaderFields() const { return headerFields_.size(); }

  /**
   * Get the header field at the given index.
   * @param index The index, starting from 0. It is an error if index is greater
   * to or equal to countHeaderFields().
   * @return The header field at the index.
   */
  ptr_lib::shared_ptr<LpPacketHeaderField>&
  getHeaderField(int index) { return headerFields_[index]; }

  /**
   * Remove all header fields and set the fragment to an isNull Blob.
   */
  void
  clear()
  {
     headerFields_.clear();
     fragmentWireEncoding_ = Blob();
  }

  /**
   * Clear this LpPacket, and set the values by copying from lpPacketLite.
   * @param lpPacketLite An LpPacketLite object.
   */
  void
  set(const LpPacketLite& lpPacketLite);

private:
  std::vector<ptr_lib::shared_ptr<LpPacketHeaderField> > headerFields_;
  Blob fragmentWireEncoding_;
};

}

#endif
