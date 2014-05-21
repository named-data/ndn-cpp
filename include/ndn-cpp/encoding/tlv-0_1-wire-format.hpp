/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_TLV_0_1A2_WIRE_FORMAT_HPP
#define NDN_TLV_0_1A2_WIRE_FORMAT_HPP

#include "wire-format.hpp"

namespace ndn {

/**
 * A Tlv0_1WireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using NDN-TLV version 0.1.  To always use the preferred version NDN-TLV, you should use the class TlvWireFormat.
 */
class Tlv0_1WireFormat : public WireFormat {
public:
  /**
   * Encode interest in NDN-TLV and return the encoding.
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
   * Decode input as an interest in NDN-TLV and set the fields of the interest object.
   * @param interest The Interest object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  virtual void 
  decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength);

  /**
   * Encode data with NDN-TLV and return the encoding.
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
   * Decode input as a data packet in NDN-TLV and set the fields in the data object.
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
   * Encode controlParameters as NDN-TLV and return the encoding.  
   * @param controlParameters The ControlParameters object to 
   * encode.
   * @return A Blob containing the encoding.
   */
  virtual Blob 
  encodeControlParameters
    (const ControlParameters& controlParameters);
  
#if 0
  /**
   * Decode input as a ControlParameters in NDN-TLV and set the fields of the 
   * controlParameters object.
   * @param controlParameters The ControlParameters object whose 
   * fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  virtual void 
  decodeControlParameters
    (ControlParameters& controlParameters, const uint8_t *input, 
     size_t inputLength);
#endif

  /**
   * Get a singleton instance of a Tlv0_1WireFormat.  To always use the 
   * preferred version NDN-TLV, you should use TlvWireFormat::get().
   * @return A pointer to the singleton instance.
   */
  static Tlv0_1WireFormat* 
  get()
  {
    if (!instance_)
      instance_ = new Tlv0_1WireFormat();
    
    return instance_;
  }
  
private:
  static Tlv0_1WireFormat* instance_;
};

}

#endif
