/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_PRIVATE_KEY_STORAGE_HPP
#define NDN_PRIVATE_KEY_STORAGE_HPP

#include <string>
#include "../../util/blob.hpp"
#include "../certificate/public-key.hpp"
#include "../security-common.hpp"
#include "../key-params.hpp"
#include "../../name.hpp"

namespace ndn {

class PrivateKeyStorage {
public:
  /**
   * The virtual destructor.
   */
  virtual
  ~PrivateKeyStorage() {}

  /**
   * Generate a pair of asymmetric keys.
   * @param keyName The name of the key pair.
   * @param params The parameters of the key.
   */
  virtual void
  generateKeyPair(const Name& keyName, const KeyParams& params) = 0;

  /**
   * Delete a pair of asymmetric keys. If the key doesn't exist, do nothing.
   * @param keyName The name of the key pair.
   */
  virtual void
  deleteKeyPair(const Name& keyName) = 0;

  /**
   * Get the public key
   * @param keyName The name of public key.
   * @return The public key.
   */
  virtual ptr_lib::shared_ptr<PublicKey>
  getPublicKey(const Name& keyName) = 0;

  /**
   * Fetch the private key for keyName and sign the data, returning a signature Blob.
   * @param data Pointer to the input byte array.
   * @param dataLength The length of data.
   * @param keyName The name of the signing key.
   * @param digestAlgorithm the digest algorithm.
   * @return The signature Blob.
   */
  virtual Blob
  sign(const uint8_t *data, size_t dataLength, const Name& keyName, DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256) = 0;

  Blob
  sign(const Blob& data, const Name& keyName, DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256)
  {
    return sign(data.buf(), data.size(), keyName, digestAlgorithm);
  }

  /**
   * Decrypt data.
   * @param keyName The name of the decrypting key.
   * @param data The byte to be decrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric encryption is used.
   * @return The decrypted data.
   */
  virtual Blob
  decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric = false) = 0;

  Blob
  decrypt(const Name& keyName, const Blob& data, bool isSymmetric = false)
  {
    return decrypt(keyName, data.buf(), data.size(), isSymmetric);
  }

  /**
   * Encrypt data.
   * @param keyName The name of the encrypting key.
   * @param data The byte to be encrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric encryption is used.
   * @return The encrypted data.
   */
  virtual Blob
  encrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric = false) = 0;

  Blob
  encrypt(const Name& keyName, const Blob& data, bool isSymmetric = false)
  {
    return encrypt(keyName, data.buf(), data.size(), isSymmetric);
  }

  /**
   * @brief Generate a symmetric key.
   * @param keyName The name of the key.
   * @param params The parameters of the key.
   */
  virtual void
  generateKey(const Name& keyName, const KeyParams& params) = 0;

  /**
   * Check if a particular key exists.
   * @param keyName The name of the key.
   * @param keyClass The class of the key, e.g. KEY_CLASS_PUBLIC, KEY_CLASS_PRIVATE, or KEY_CLASS_SYMMETRIC.
   * @return True if the key exists, otherwise false.
   */
  virtual bool
  doesKeyExist(const Name& keyName, KeyClass keyClass) = 0;
};

}

#endif
