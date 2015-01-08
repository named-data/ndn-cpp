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

#ifndef NDN_PROTOBUF_TLV_HPP
#define NDN_PROTOBUF_TLV_HPP

#include <ndn-cpp/util/blob.hpp>

namespace google { namespace protobuf { class Message; } }

namespace ndn {

/** ProtobufTlv has static methods to encode and decode an Protobuf Message
 * object as NDN-TLV. The Protobuf tag value is used as the TLV type code. A
 * Protobuf message is encoded/decoded as a nested TLV encoding. Protobuf types
 * uint32, uint64 and enum are encoded/decoded as TLV nonNegativeInteger. (It is
 * an error if an enum value is negative.) Protobuf types bytes and string are
 * encoded/decoded as TLV bytes. The Protobuf type bool is encoded/decoded as a
 * TLV boolean (a zero length value for True, omitted for False). Other Protobuf
 * types are an error.
 *
 * Protobuf has no "outer" message type, so you need to put your TLV message
 * inside an outer "typeless" message.
 *
 */
class ProtobufTlv {
public:
  /**
   * Encode the Protobuf Message object as NDN-TLV.
   * @param message The Protobuf Message object. This calls
   * message.CheckInitialized() to ensure that all required fields are present.
   * @return The encoded byte array as a Blob.
   */
  static Blob
  encode(const google::protobuf::Message& message);

  /**
   * Decode the input as NDN-TLV and update the fields of the Protobuf Message
   * object.
   * @param message The Protobuf Message object. This does not first clear
   * the object.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  static void
  decode
    (google::protobuf::Message& message, const uint8_t *input, size_t inputLength);

  /**
   * Decode the input as NDN-TLV and update the fields of the Protobuf Message
   * object.
   * @param message The Protobuf Message object. This does not first clear
   * the object.
   * @param input The input byte array to be decoded.
   */
  static void
  decode(google::protobuf::Message& message, const std::vector<uint8_t>& input)
  {
    decode(message, &input[0], input.size());
  }

  /**
   * Decode the input as NDN-TLV and update the fields of the Protobuf Message
   * object.
   * @param message The Protobuf Message object. This does not first clear
   * the object.
   * @param input The Blob with the input to be decoded.
   */
  static void
  decode(google::protobuf::Message& message, const Blob& input)
  {
    decode(message, input.buf(), input.size());
  }
};

}

#endif
