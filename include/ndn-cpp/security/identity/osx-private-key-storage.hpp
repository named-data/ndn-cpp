/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2019 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_OSX_PRIVATEKEY_STORAGE_H
#define NDN_OSX_PRIVATEKEY_STORAGE_H

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_OSX_SECURITY 1.
#include "../../ndn-cpp-config.h"
#if NDN_CPP_HAVE_OSX_SECURITY

#include "../../common.hpp"
#include "../tpm/helper-osx.hpp"
#include "private-key-storage.hpp"

namespace ndn
{

class OSXPrivateKeyStorage : public PrivateKeyStorage {
public:
  /**
   * Create a new OSXPrivateKeyStorage.
   */
  OSXPrivateKeyStorage() {}

  /**
   * destructor of OSXPrivateKeyStore
   */
  virtual
  ~OSXPrivateKeyStorage();

  /**
   * Generate a pair of asymmetric keys.
   * @param keyName The name of the key pair.
   * @param params The parameters of the key.
   */
  virtual void
  generateKeyPair(const Name& keyName, const KeyParams& params);

  /**
   * Delete a pair of asymmetric keys. If the key doesn't exist, do nothing.
   * @param keyName The name of the key pair.
   */
  virtual void
  deleteKeyPair(const Name& keyName);

  /**
   * Get the public key
   * @param keyName The name of public key.
   * @return The public key.
   */
  virtual ptr_lib::shared_ptr<PublicKey>
  getPublicKey(const Name& keyName);

  /**
   * Fetch the private key for keyName and sign the data, returning a signature Blob.
   * @param data Pointer to the input byte array.
   * @param dataLength The length of data.
   * @param keyName The name of the signing key.
   * @param digestAlgorithm the digest algorithm.
   * @return The signature Blob.
   */
  virtual Blob
  sign(const uint8_t *data, size_t dataLength, const Name& keyName, DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256);

  /**
   * Decrypt data.
   * @param keyName The name of the decrypting key.
   * @param data The byte to be decrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric decryption is used.
   * @return The decrypted data.
   */
  virtual Blob
  decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric = false);

  /**
   * Encrypt data.
   * @param keyName The name of the encrypting key.
   * @param data The byte to be encrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric decryption is used.
   * @return The encrypted data.
   */
  virtual Blob
  encrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric = false);

  /**
   * Generate a symmetric key.
   * @param keyName The name of the key.
   * @param params The parameters of the key.
   */
  virtual void
  generateKey(const Name& keyName, const KeyParams& params);

  /**
   * Check if a particular key exists.
   * @param keyName The name of the key.
   * @param keyClass The class of the key, e.g. KEY_CLASS_PUBLIC, KEY_CLASS_PRIVATE, or KEY_CLASS_SYMMETRIC.
   * @return True if the key exists, otherwise false.
   */
  virtual bool
  doesKeyExist(const Name& keyName, KeyClass keyClass);

  /**
   * configure ACL of a particular key
   * @param keyName the name of key
   * @param keyClass the class of key, e.g. Private Key
   * @param acl the new acl of the key
   * @param appPath the absolute path to the application
   * @return true if setting succeeds
   */
  bool
  setACL(const Name & keyName, KeyClass keyClass, int acl, const std::string & appPath);

 private:
  /**
   * convert NDN name of a key to internal name of the key
   * @param keyName the NDN name of the key
   * @param keyClass the class of the key
   * @return the internal key name
   */
  std::string
  toInternalKeyName(const Name & keyName, KeyClass keyClass);

  /**
   * Get key
   * @param keyName the name of the key
   * @param keyClass the class of the key
   * @return 0 if not found,  otherwise a Keychain ref to the key. You
   * must use CFRelease to free it.
   */
  CFReleaser<SecKeychainItemRef>
  getKey(const Name & keyName, KeyClass keyClass);

  /**
   * convert keyType to MAC OS symmetric key key type
   * @param keyType
   * @return MAC OS key type
   */
  const CFTypeRef
  getSymmetricKeyType(KeyType keyType);

  /**
   * convert keyType to MAC OS asymmetirc key type
   * @param keyType
   * @return MAC OS key type
   */
  const CFTypeRef
  getAsymmetricKeyType(KeyType keyType);

  /**
   * convert keyClass to MAC OS key class
   * @param keyClass
   * @return MAC OS key class
   */
  const CFTypeRef
  getKeyClass(KeyClass keyClass);

  /**
   * convert digestAlgorithm to MAC OS algorithm id
   * @param digestAlgorithm
   * @return MAC OS algorithm id
   */
  const CFStringRef
  getDigestAlgorithm(DigestAlgorithm digestAlgorithm);

  /**
   * get the digest size of the corresponding algorithm
   * @param digestAlgo the digest algorithm
   * @return digest size
   */
  long
  getDigestSize(DigestAlgorithm digestAlgo);
};

}

#endif // NDN_CPP_HAVE_OSX_SECURITY

#endif
