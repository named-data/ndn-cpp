/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/encrypted-content https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_ENCRYPTED_CONTENT_HPP
#define NDN_ENCRYPTED_CONTENT_HPP

#include "../c/encrypt/algo/encrypt-params-types.h"
#include "../key-locator.hpp"
#include "../lite/encrypt/encrypted-content-lite.hpp"

namespace ndn {

/**
 * An EncryptedContent holds an encryption type, a payload and other fields
 * representing encrypted content.
 * @note This class is an experimental feature. The API may change.
 */
class EncryptedContent {
public:
  /**
   * Create an EncryptedContent where all the values are unspecified.
   */
  EncryptedContent()
  : algorithmType_((ndn_EncryptAlgorithmType)-1)
  {
  }

  /**
   * Get the algorithm type.
   * @return The algorithm type, or -1 if not specified.
   */
  ndn_EncryptAlgorithmType
  getAlgorithmType() const { return algorithmType_; }

  /**
   * Get the key locator.
   * @return The key locator. If not specified, getType() is -1.
   */
  const KeyLocator&
  getKeyLocator() const { return keyLocator_; }

  KeyLocator&
  getKeyLocator() { return keyLocator_; }

  /**
   * Check that the key locator type is KEYNAME and return the key Name.
   * @return The key Name.
   * @throws runtime_error if the key locator type is not KEYNAME.
   */
  const Name&
  getKeyLocatorName() const;

  /**
   * Check if the initial vector is specified.
   * @return True if the initial vector is specified.
   */
  bool
  hasInitialVector() const { return !initialVector_.isNull(); }

  /**
   * Get the initial vector.
   * @return The initial vector. If not specified, isNull() is true.
   */
  const Blob&
  getInitialVector() const { return initialVector_; }

  /**
   * Get the payload.
   * @return The payload. If not specified, isNull() is true.
   */
  const Blob&
  getPayload() const { return payload_; }

  /**
   * Get the encrypted payload key.
   * @return The encrypted payload key. If not specified, isNull() is true.
   */
  const Blob&
  getPayloadKey() const { return payloadKey_; }

  /**
   * Set the algorithm type.
   * @param algorithmType The algorithm type. If not specified, set to -1.
   * @return This EncryptedContent so that you can chain calls to update
   * values.
   */
  EncryptedContent&
  setAlgorithmType(ndn_EncryptAlgorithmType algorithmType)
  {
    algorithmType_ = algorithmType;
    return *this;
  }

  /**
   * Set this object to use a copy of the given KeyLocator object.
   * @note You can also call getKeyLocator and change the key locator directly.
   * @param keyLocator The KeyLocator object. This makes a copy of the object.
   * If no key locator is specified, set to a new default KeyLocator(), or to a
   * KeyLocator with an unspecified type.
   * @return This EncryptedContent so that you can chain calls to update values.
   */
  EncryptedContent&
  setKeyLocator(const KeyLocator& keyLocator)
  {
    keyLocator_ = keyLocator;
    return *this;
  }

  /**
   * Set the key locator type to KEYNAME and set the key Name.
   * @param keyName The key locator Name, which is copied.
   * @return This EncryptedContent so that you can chain calls to update values.
   */
  EncryptedContent&
  setKeyLocatorName(const Name& keyName)
  {
    keyLocator_.setType(ndn_KeyLocatorType_KEYNAME);
    keyLocator_.setKeyName(keyName);
    return *this;
  }

  /**
   * Set the initial vector.
   * @param initialVector The initial vector. If not specified, set to the
   * default Blob() where isNull() is true.
   * @return This EncryptedContent so that you can chain calls to update
   * values.
   */
  EncryptedContent&
  setInitialVector(const Blob& initialVector)
  {
    initialVector_ = initialVector;
    return *this;
  }

  /**
   * Set the encrypted payload.
   * @param payload The encrypted payload. If not specified, set to the
   * default Blob() where isNull() is true.
   * @return This EncryptedContent so that you can chain calls to update
   * values.
   */
  EncryptedContent&
  setPayload(const Blob& payload)
  {
    payload_ = payload;
    return *this;
  }

  /**
   * Set the encrypted payload key.
   * @param payloadKey The encrypted payload key. If not specified, set to the
   * default Blob() where isNull() is true.
   * @return This EncryptedContent so that you can chain calls to update
   * values.
   */
  EncryptedContent&
  setPayloadKey(const Blob& payloadKey)
  {
    payloadKey_ = payloadKey;
    return *this;
  }

  /**
   * Encode this to an EncryptedContent v1 wire encoding.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * EncryptedContent. If omitted, use WireFormat::getDefaultWireFormat().
   * @return The encoded byte array.
   */
  Blob
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const
  {
    return wireFormat.encodeEncryptedContent(*this);
  }

  /**
   * Encode this to an EncryptedContent v2 (used in Name-based Access Control
   * v2) wire encoding.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * EncryptedContent. If omitted, use WireFormat::getDefaultWireFormat().
   * @return The encoded byte array.
   */
  Blob
  wireEncodeV2(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const
  {
    return wireFormat.encodeEncryptedContentV2(*this);
  }

  /**
   * Decode the input as an EncryptedContent v1 using a particular wire format
   * and update this EncryptedContent.
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const uint8_t *input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireFormat.decodeEncryptedContent(*this, input, inputLength);
  }

  /**
   * Decode the input as an EncryptedContent v1 using a particular wire format
   * and update this EncryptedContent.
   * @param input The input byte array to be decoded.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const std::vector<uint8_t>& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Decode the input as an EncryptedContent v1 using a particular wire format
   * and update this EncryptedContent.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(input.buf(), input.size(), wireFormat);
  }

  /**
   * Decode the input as an EncryptedContent v2 (used in Name-based Access
   * Control v2) using a particular wire format and update this EncryptedContent.
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecodeV2
    (const uint8_t *input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireFormat.decodeEncryptedContentV2(*this, input, inputLength);
  }

  /**
   * Decode the input as an EncryptedContent v2 (used in Name-based Access
   * Control v2) using a particular wire format and update this EncryptedContent.
   * @param input The input byte array to be decoded.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecodeV2
    (const std::vector<uint8_t>& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecodeV2(&input[0], input.size(), wireFormat);
  }

  /**
   * Decode the input as an EncryptedContent v2 (used in Name-based Access
   * Control v2) using a particular wire format and update this EncryptedContent.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecodeV2
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecodeV2(input.buf(), input.size(), wireFormat);
  }

  /**
   * Set encryptedContentLite to point to the values in this EncryptedContent
   * object, without copying any memory.
   * WARNING: The resulting pointers in encryptedContentLite are invalid after a
   * further use of this object which could reallocate memory.
   * @param encryptedContentLite An EncryptedContentLite object where the key
   * locator name components array is already allocated.
   */
  void
  get(EncryptedContentLite& encryptedContentLite) const;

  /**
   * Clear this EncryptedContent, and set the values by copying from
   * encryptedContentLite.
   * @param encryptedContentLite An EncryptedContentLite object.
   */
  void
  set(const EncryptedContentLite& encryptedContentLite);

private:
  ndn_EncryptAlgorithmType algorithmType_; /**< -1 for none */
  KeyLocator keyLocator_;
  Blob initialVector_;
  Blob payload_;
  Blob payloadKey_;
};

}

#endif
