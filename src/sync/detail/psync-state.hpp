/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/state.hpp
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

#ifndef NDN_PSYNC_STATE_HPP
#define NDN_PSYNC_STATE_HPP

#include <ndn-cpp/name.hpp>

namespace ndn {

class TlvEncoder;

/**
 * The PSyncState class represents a sequence of Names as the state of PSync.
 * It has methods to encode and decode for the wire.
 */
class PSyncState {
public:
  /**
   * Create a PSyncState with empty content.
   */
  PSyncState() {}

  /**
   * Create a PSyncState by decoding the input as an NDN-TLV PSyncContent.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  PSyncState(const uint8_t *input, size_t inputLength)
  {
    wireDecode(input, inputLength);
  }

  /**
   * Create a PSyncState by decoding the input as an NDN-TLV PSyncContent.
   * @param input The input buffer to decode.
   */
  PSyncState(const Blob& input)
  {
    wireDecode(input);
  }

  /**
   * Append the name to the content.
   * @param name The Name to add.
   */
  void
  addContent(const Name& name) { content_.push_back(name); }

  /**
   * Get the sequence of Names in the content.
   * @return The vector of Names.
   */
  const std::vector<Name>&
  getContent() const { return content_; }

  /**
   * Remove the content.
   */
  void
  clear() { content_.clear(); }

  /**
   * Encode this as an NDN-TLV PSyncContent.
   * @return The encoding as a Blob.
   */
  Blob
  wireEncode() const;

  /**
   * Decode the input as an NDN-TLV PSyncContent and update this object.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  void
  wireDecode(const uint8_t *input, size_t inputLength);

  /**
   * Decode the input as an NDN-TLV PSyncContent and update this object.
   * @param input The input buffer to decode.
   */
  void
  wireDecode(const Blob& input)
  {
    wireDecode(input.buf(), input.size());
  }

  /**
   * Get the string representation of this SigningInfo.
   * @return The string representation.
   */
  std::string
  toString() const;

  enum {
    Tlv_PSyncContent = 128
  };

private:
  /**
   * This is called by writeNestedTlv to encode the sequence of Names in
   * the content.
   * @param context A pointer to the PSyncState.
   * @param encoder The TlvEncoder.
   */
  static void
  encodeContent(const void *context, TlvEncoder &encoder);

  std::vector<Name> content_;
};

}

#endif
