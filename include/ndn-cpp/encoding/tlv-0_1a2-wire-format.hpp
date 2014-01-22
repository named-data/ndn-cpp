/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_TLV_0_1A2_WIRE_FORMAT_HPP
#define NDN_TLV_0_1A2_WIRE_FORMAT_HPP

#include "wire-format.hpp"

namespace ndn {

/**
 * A Tlv1_0a2WireFormat extends WireFormat to override its virtual methods to implement encoding and decoding
 * using NDN-TLV version 1.0a2.  To always use the preferred version NDN-TLV, you should use the class TlvWireFormat.
 */
class Tlv1_0a2WireFormat : public WireFormat {
public:
  /**
   * Encode interest in NDN-TLV and return the encoding.
   * @param interest The Interest object to encode.
   * @return A Blob containing the encoding.
   */  
  virtual Blob 
  encodeInterest(const Interest& interest);
    
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

#if 0
  /**
   * Encode forwardingEntry in NDN-TLV and return the encoding. 
   * @param forwardingEntry The ForwardingEntry object to encode.
   * @return A Blob containing the encoding.
   */
  virtual Blob 
  encodeForwardingEntry(const ForwardingEntry& forwardingEntry);
  
  /**
   * Decode input as a forwarding entry in NDN-TLV and set the fields of the forwardingEntry object. 
   * @param forwardingEntry The ForwardingEntry object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  virtual void 
  decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength);
#endif
};

}

#endif
