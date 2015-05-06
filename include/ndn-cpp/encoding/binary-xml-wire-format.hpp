/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_BINARYXMLWIREFORMAT_HPP
#define NDN_BINARYXMLWIREFORMAT_HPP

#include "wire-format.hpp"

namespace ndn {

/**
 * A BinaryXmlWireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using binary XML.
 */
class BinaryXmlWireFormat : public WireFormat {
public:
  BinaryXmlWireFormat();

  /**
   * Get a singleton instance of a BinaryXmlWireFormat.  Assuming that the default wire format was set with
   * WireFormat::setDefaultWireFormat(BinaryXmlWireFormat::get()), you can check if this is the default wire encoding with
   * if (WireFormat::getDefaultWireFormat() == BinaryXmlWireFormat::get()).
   * @return A pointer to the singleton instance.
   */
  static BinaryXmlWireFormat*
  get()
  {
    if (!instance_)
      instance_ = new BinaryXmlWireFormat();

    return instance_;
  }

  /**
   * Encode name in binary XML and return the encoding.
   * @param name The Name object to encode.
   * @return A Blob containing the encoding.
   */
  virtual Blob
  encodeName(const Name& name);

  /**
   * Decode input as a name in binary XML and set the fields of the Name object.
   * @param name The Name object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  virtual void
  decodeName(Name& name, const uint8_t *input, size_t inputLength);

  /**
   * Encode interest in binary XML and return the encoding.
   * @param interest The Interest object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @param signedPortionEndOffset Return the offset in the encoding of the end
   * of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @return A Blob containing the encoding.
   */
  virtual Blob
  encodeInterest
    (const Interest& interest, size_t *signedPortionBeginOffset,
     size_t *signedPortionEndOffset);

  /**
   * Decode input as an interest in binary XML and set the fields of the interest object.
   * @param interest The Interest object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * If you are not decoding in order to verify, you can call
   * decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength)
   * to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the encoding of the end
   * of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * If you are not decoding in order to verify, you can call
   * decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength)
   * to ignore this returned value.
   */
  virtual void
  decodeInterest
    (Interest& interest, const uint8_t *input, size_t inputLength,
     size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  /**
   * Encode data with binary XML and return the encoding.
   * @param data The Data object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the beginning of the signed portion.
   * If you are not encoding in order to sign, you can call encodeData(const Data& data) to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the encoding of the end of the signed portion.
   * If you are not encoding in order to sign, you can call encodeData(const Data& data) to ignore this returned value.
   * @return A Blob containing the encoding.
   */
  virtual Blob
  encodeData
    (const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  /**
   * Decode input as a data packet in binary XML and set the fields in the data object.
   * @param data The Data object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @param signedPortionBeginOffset Return the offset in the input buffer of the beginning of the signed portion.
   * If you are not decoding in order to verify, you can call
   * decodeData(Data& data, const uint8_t *input, size_t inputLength) to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the input buffer of the end of the signed portion.
   * If you are not decoding in order to verify, you can call
   * decodeData(Data& data, const uint8_t *input, size_t inputLength) to ignore this returned value.
   */
  virtual void
  decodeData
    (Data& data, const uint8_t *input, size_t inputLength, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  /**
   * Encode forwardingEntry in binary XML and return the encoding.
   * @param forwardingEntry The ForwardingEntry object to encode.
   * @return A Blob containing the encoding.
   */
  virtual Blob
  encodeForwardingEntry(const ForwardingEntry& forwardingEntry);

  /**
   * Decode input as a forwarding entry in binary XML and set the fields of the forwardingEntry object.
   * @param forwardingEntry The ForwardingEntry object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  virtual void
  decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength);

private:
  static BinaryXmlWireFormat* instance_;
};

}

#endif

