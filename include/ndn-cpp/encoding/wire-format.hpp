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

#ifndef NDN_WIREFORMAT_HPP
#define NDN_WIREFORMAT_HPP

#include "../common.hpp"
#include "../util/blob.hpp"

namespace ndn {

class Name;
class Interest;
class Data;
class ForwardingEntry;
class ControlParameters;
class Signature;

class WireFormat {
public:
  /** NDNx support and binary XML (ccnb or ndnb) encoding is deprecated and code
   * with throw an exception. To enable support while you upgrade your code to
   * use NFD, set WireFormat::ENABLE_NDNX = true . NDNx support will be
   * completely removed in an upcoming release.
   */
  static bool ENABLE_NDNX;

  /**
   * Encode name and return the encoding. Your derived class should override.
   * @param name The Name object to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeName(const Name& name);

  /**
   * Decode input as a name and set the fields of the Name object.  Your derived class should override.
   * @param name The Name object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual void
  decodeName(Name& name, const uint8_t *input, size_t inputLength);

  /**
   * Encode interest and return the encoding. Your derived class should override.
   * @param interest The Interest object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the
   * beginning of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * If you are not encoding in order to sign, you can call
   * encodeInterest(const Interest& interest) to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the encoding of the end
   * of the signed portion. The signed portion starts from the first
   * name component and ends just before the final name component (which is
   * assumed to be a signature for a signed interest).
   * If you are not encoding in order to sign, you can call
   * encodeInterest(const Interest& interest) to ignore this returned value.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeInterest
    (const Interest& interest, size_t *signedPortionBeginOffset,
     size_t *signedPortionEndOffset);

  /**
   * Encode interest and return the encoding.  Your derived class should override.
   * @param interest The Interest object to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  Blob
  encodeInterest(const Interest& interest)
  {
    size_t dummyBeginOffset, dummyEndOffset;
    return encodeInterest(interest, &dummyBeginOffset, &dummyEndOffset);
  }

  /**
   * Decode input as an interest and set the fields of the interest object.  Your derived class should override.
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
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual void
  decodeInterest
    (Interest& interest, const uint8_t *input, size_t inputLength,
     size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  void
  decodeInterest(Interest& interest, const uint8_t *input, size_t inputLength)
  {
    size_t dummyBeginOffset, dummyEndOffset;
    decodeInterest
      (interest, input, inputLength, &dummyBeginOffset, &dummyEndOffset);
  }

  /**
   * Encode data and return the encoding.  Your derived class should override.
   * @param data The Data object to encode.
   * @param signedPortionBeginOffset Return the offset in the encoding of the beginning of the signed portion.
   * If you are not encoding in order to sign, you can call encodeData(const Data& data) to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the encoding of the end of the signed portion.
   * If you are not encoding in order to sign, you can call encodeData(const Data& data) to ignore this returned value.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeData
    (const Data& data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  /**
   * Encode data and return the encoding.
   * @param data The Data object to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  Blob
  encodeData(const Data& data)
  {
    size_t dummyBeginOffset, dummyEndOffset;
    return encodeData(data, &dummyBeginOffset, &dummyEndOffset);
  }

  /**
   * Decode input as a data packet and set the fields in the data object.  Your derived class should override.
   * @param data The Data object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @param signedPortionBeginOffset Return the offset in the input buffer of the beginning of the signed portion.
   * If you are not decoding in order to verify, you can call
   * decodeData(Data& data, const uint8_t *input, size_t inputLength) to ignore this returned value.
   * @param signedPortionEndOffset Return the offset in the input buffer of the end of the signed portion.
   * If you are not decoding in order to verify, you can call
   * decodeData(Data& data, const uint8_t *input, size_t inputLength) to ignore this returned value.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual void
  decodeData
    (Data& data, const uint8_t *input, size_t inputLength, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset);

  void
  decodeData(Data& data, const uint8_t *input, size_t inputLength)
  {
    size_t dummyBeginOffset, dummyEndOffset;
    decodeData(data, input, inputLength, &dummyBeginOffset, &dummyEndOffset);
  }

  /**
   * Encode forwardingEntry and return the encoding.  Your derived class should override.
   * @param forwardingEntry The ForwardingEntry object to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeForwardingEntry(const ForwardingEntry& forwardingEntry);

  /**
   * Decode input as a forwarding entry and set the fields of the forwardingEntry object.  Your derived class should override.
   * @param forwardingEntry The ForwardingEntry object whose fields are updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual void
  decodeForwardingEntry(ForwardingEntry& forwardingEntry, const uint8_t *input, size_t inputLength);

  /**
   * Encode controlParameters and return the encoding.
   * Your derived class should override.
   * @param controlParameters The ControlParameters object to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeControlParameters
    (const ControlParameters& controlParameters);

  /**
   * Decode input as a command parameters and set the fields of the
   * controlParameters object.  Your derived class should override.
   * @param controlParameters The ControlParameters object whose fields are
   * updated.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual void
  decodeControlParameters
    (ControlParameters& controlParameters, const uint8_t *input,
     size_t inputLength);

  /**
   * Encode signature as a SignatureInfo and return the encoding.
   * Your derived class should override.
   * @param signature An object of a subclass of Signature to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeSignatureInfo(const Signature& signature);

  /**
   * Decode signatureInfo as a signature info and signatureValue as the related
   * SignatureValue, and return a new object which is a subclass of Signature.
   * Your derived class should override.
   * @param signatureInfo A pointer to the signature info input buffer to decode.
   * @param signatureInfoLength The number of bytes in signatureInfo.
   * @param signatureValue A pointer to the signature value input buffer to decode.
   * @param signatureValueLength The number of bytes in signatureValue.
   * @return A new object which is a subclass of Signature.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual ptr_lib::shared_ptr<Signature>
  decodeSignatureInfoAndValue
    (const uint8_t *signatureInfo, size_t signatureInfoLength,
     const uint8_t *signatureValue, size_t signatureValueLength);

  /**
   * Decode signatureInfo as a signature info and signatureValue as the related
   * SignatureValue, and return a new object which is a subclass of Signature.
   * @param signatureInfo The signature input buffer to decode.
   * @param signatureValue The signature value input buffer to decode.
   * @return A new object which is a subclass of Signature.
   */
  virtual ptr_lib::shared_ptr<Signature>
  decodeSignatureInfoAndValue
    (const Blob& signatureInfo, const Blob& signatureValue)
  {
    return decodeSignatureInfoAndValue
      (signatureInfo.buf(), signatureInfo.size(), signatureValue.buf(),
       signatureValue.size());
  }

  /**
   * Encode the signatureValue in the Signature object as a SignatureValue (the
   * signature bits) and return the encoding.
   * Your derived class should override.
   * @param signature An object of a subclass of Signature with the signature
   * value to encode.
   * @return A Blob containing the encoding.
   * @throws logic_error for unimplemented if the derived class does not override.
   */
  virtual Blob
  encodeSignatureValue(const Signature& signature);

  /**
   * Set the static default WireFormat used by default encoding and decoding methods.
   * @param wireFormat A Pointer to an object of a subclass of WireFormat.  This does not make a copy and
   * the caller must ensure that the object remains allocated.
   */
  static void
  setDefaultWireFormat(WireFormat *wireFormat)
  {
    defaultWireFormat_ = wireFormat;
  }

  /**
   * Return the default WireFormat used by default encoding and decoding methods which was set with
   * setDefaultWireFormat.
   * @return A pointer to the WireFormat object.
   */
  static WireFormat*
  getDefaultWireFormat();

private:
  /**
   * This is implemented by only one of the subclasses of WireFormat to return a new object used
   * as the initial value for the default WireFormat.  If the application doesn't include that class, then the application
   * needs to include another subclass which defines WireFormat::newInitialDefaultWireFormat.
   * @return a new object, which is held by a shared_ptr and freed when the application exits.
   */
  static WireFormat*
  newInitialDefaultWireFormat();

  static WireFormat *defaultWireFormat_;
};

}

#endif

