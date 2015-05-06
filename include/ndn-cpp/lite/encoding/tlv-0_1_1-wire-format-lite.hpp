/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#ifndef NDN_TLV_0_1_1_WIRE_FORMAT_LITE_HPP
#define NDN_TLV_0_1_1_WIRE_FORMAT_LITE_HPP

#include "../interest-lite.hpp"
#include "../data-lite.hpp"
#include "../util/dynamic-uint8-array-lite.hpp"

namespace ndn {

/**
 * A Tlv0_1_1WireFormatLite implements implement encoding and decoding using
 * NDN-TLV version 0.1.1.
 */
class Tlv0_1_1WireFormatLite {
public:
  /**
   * Encode interest as NDN-TLV.
   * @param interest The interest object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @param signedPortionEndOffset Return the offset in the encoding of the end
   * of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @param output A DynamicUInt8ArrayLite object which receives the encoded
   * output.  If the output's reallocFunction is null, its array must be large
   * enough to receive the entire encoding.
   * @param encodingLength Set encodingLength to the length of the encoded output.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  encodeInterest
    (const InterestLite& interest, size_t* signedPortionBeginOffset,
     size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
     size_t* encodingLength);

  /**
   * Decode input as an interest in NDN-TLV and set the fields of the interest
   * object.
   * @param interest The Interest object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @param signedPortionEndOffset Return the offset in the encoding of the end
   * of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  decodeInterest
    (InterestLite& interest, const uint8_t* input, size_t inputLength,
     size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset);

  /**
   * Encode the data packet as NDN-TLV.
   * @param data The data object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. If you are not encoding in order to sign,
   * you can ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the encoding of the end of
   * the signed portion. If you are not encoding in order to sign, you can ignore
   * this returned value.
   * @param output A DynamicUInt8ArrayLite object which receives the encoded
   * output.  If the output's reallocFunction is null, its array must be large
   * enough to receive the entire encoding.
   * @param encodingLength Set encodingLength to the length of the encoded output.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  encodeData
    (const DataLite& data, size_t* signedPortionBeginOffset,
     size_t* signedPortionEndOffset, DynamicUInt8ArrayLite& output,
     size_t* encodingLength);

  /**
   * Decode input as a data packet in NDN-TLV and set the fields in the data object.
   * @param data The data object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @param signedPortionBeginOffset Return the offset in the input buffer of the
   * beginning of the signed portion. If you are not decoding in order to verify,
   * you can ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the input buffer of the end
   * of the signed portion. If you are not decoding in order to verify, you can
   * ignore this returned value.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  decodeData
    (DataLite& data, const uint8_t* input, size_t inputLength,
     size_t* signedPortionBeginOffset, size_t* signedPortionEndOffset);
};

}

#endif
