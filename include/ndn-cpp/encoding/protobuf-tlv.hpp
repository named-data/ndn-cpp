/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2020 Regents of the University of California.
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

#include "../util/blob.hpp"
#include "../name.hpp"

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
    decode(message, input.data(), input.size());
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

  /**
   * Find the field in the message with the given field name, and return a TLV
   * encoding where the type is the field number and the value is the field
   * byte array. In this way, you can have a Protobuf message where one of the
   * fields is a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param fieldName The name of the field in the message.
   * @return The TLV encoding.
   * @throws runtime_error if the message does not have a non-repeated field
   * with the given fieldName.
   */
  static Blob
  getTlv(const google::protobuf::Message& message, const std::string& fieldName);

  /**
   * Find the repeated field in the message with the given field name, and
   * return a TLV encoding where the type is the field number and the value is
   * the field byte array at the given index. In this way, you can have a
   * Protobuf message where one of the fields is a repetition of a pre-encoded
   * TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param fieldName The name of the repeated field in the message.
   * @param index The index of the repeated field to get.
   * @return The TLV encoding.
   * @throws runtime_error if the message does not have a repeated field with
   * the given fieldName.
   */
  static Blob
  getTlv
    (const google::protobuf::Message& message, const std::string& fieldName,
     int index);

  /**
   * Find the repeated field in the message with the same field number as the
   * type in the TLV encoding, and add the TLV value to the field. In this way,
   * you can have a Protobuf message where one of the fields is a repetition of
   * a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @param encodingLength The length  of the TLV encoding.
   * @throws runtime_error if the message does not have a repeated field with
   * the same field number as the type in the TLV encoding.
   */
  static void
  addTlv
    (google::protobuf::Message& message, const uint8_t *encoding,
     size_t encodingLength);

  /**
   * Find the repeated field in the message with the same field number as the
   * type in the TLV encoding, and add the TLV value to the field. In this way,
   * you can have a Protobuf message where one of the fields is a repetition of
   * a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @throws runtime_error if the message does not have a repeated field with
   * the same field number as the type in the TLV encoding.
   */
  static void
  addTlv
    (google::protobuf::Message& message, const std::vector<uint8_t>& encoding)
  {
    addTlv(message, encoding.data(), encoding.size());
  }

  /**
   * Find the repeated field in the message with the same field number as the
   * type in the TLV encoding, and add the TLV value to the field. In this way,
   * you can have a Protobuf message where one of the fields is a repetition of
   * a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @throws runtime_error if the message does not have a repeated field with
   * the same field number as the type in the TLV encoding.
   */
  static void
  addTlv(google::protobuf::Message& message, const Blob& encoding)
  {
    addTlv(message, encoding.buf(), encoding.size());
  }

  /**
   * Find the field in the message with the same field number as the type in the
   * TLV encoding, and set the field to the TLV value. In this way, you can have
   * a Protobuf message where one of the fields is a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @param encodingLength The length  of the TLV encoding.
   * @throws runtime_error if the message does not have a non-repeated field
   * with the same field number as the type in the TLV encoding.
   */
  static void
  setTlv
    (google::protobuf::Message& message, const uint8_t *encoding,
     size_t encodingLength);

  /**
   * Find the field in the message with the same field number as the type in the
   * TLV encoding, and set the field to the TLV value. In this way, you can have
   * a Protobuf message where one of the fields is a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @throws runtime_error if the message does not have a non-repeated field
   * with the same field number as the type in the TLV encoding.
   */
  static void
  setTlv
    (google::protobuf::Message& message, const std::vector<uint8_t>& encoding)
  {
    setTlv(message, encoding.data(), encoding.size());
  }

  /**
   * Find the field in the message with the same field number as the type in the
   * TLV encoding, and set the field to the TLV value. In this way, you can have
   * a Protobuf message where one of the fields is a pre-encoded TLV.
   * @param message The Protobuf Message object. This does not search nested
   * message objects.
   * @param encoding The TLV encoding.
   * @throws runtime_error if the message does not have a non-repeated field
   * with the same field number as the type in the TLV encoding.
   */
  static void
  setTlv(google::protobuf::Message& message, const Blob& encoding)
  {
    setTlv(message, encoding.buf(), encoding.size());
  }

  /**
   * Return a Name made from the component array in a Protobuf message object,
   * assuming that it was defined with "repeated bytes". For example:
   * message Name {
   *   repeated bytes component = 8;
   * }
   * (See the test-encode-decode-fib-entry example.)
   * @param nameMessage The Protobuf message object containing the "repeated
   * bytes" component array.
   * @return A new name.
   */
  static Name
  toName(const google::protobuf::Message& nameMessage);
};

}

#endif
