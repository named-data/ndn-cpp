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

#ifndef NDN_MEMORY_PRIVATE_KEY_STORAGE_HPP
#define NDN_MEMORY_PRIVATE_KEY_STORAGE_HPP

#include <map>
#include "private-key-storage.hpp"

struct rsa_st;
struct ec_key_st;

namespace ndn {

/**
 * MemoryPrivateKeyStorage extends PrivateKeyStorage to implement a simple in-memory private key store.  You should
 * initialize by calling setKeyPairForKeyName.
 */
class MemoryPrivateKeyStorage : public PrivateKeyStorage {
public:
  /**
   * The virtual destructor
   */
  virtual
  ~MemoryPrivateKeyStorage();

  /**
   * Set the public key for the keyName.
   * @param keyName The key name.
   * @param keyType The KeyType, such as KEY_TYPE_RSA.
   * @param publicKeyDer The public key DER byte array.
   * @param publicKeyDerLength The length of publicKeyDer.
   */
  void
  setPublicKeyForKeyName
    (const Name& keyName, KeyType keyType, const uint8_t* publicKeyDer,
     size_t publicKeyDerLength);

  /**
   * @deprecated Use setPublicKeyForKeyName(keyName, KEY_TYPE_RSA, publicKeyDer,
   * publicKeyDerLength).
   */
  void
  DEPRECATED_IN_NDN_CPP setPublicKeyForKeyName
    (const Name& keyName, const uint8_t* publicKeyDer, size_t publicKeyDerLength)
  {
    setPublicKeyForKeyName(keyName, KEY_TYPE_RSA, publicKeyDer, publicKeyDerLength);
  }

  /**
   * Set the private key for the keyName.
   * @param keyName The key name.
   * @param keyType The KeyType, such as KEY_TYPE_RSA.
   * @param privateKeyDer The private key DER byte array.
   * @param privateKeyDerLength The length of privateKeyDer.
   */
  void
 setPrivateKeyForKeyName
    (const Name& keyName, KeyType keyType, const uint8_t* privateKeyDer,
     size_t privateKeyDerLength);

  /**
   * @deprecated Use setPrivateKeyForKeyName(keyName, KEY_TYPE_RSA,
   * privateKeyDer, privateKeyDerLength).
   */
  void
   DEPRECATED_IN_NDN_CPP setPrivateKeyForKeyName
    (const Name& keyName, const uint8_t* privateKeyDer, size_t privateKeyDerLength)
  {
    setPrivateKeyForKeyName(keyName, KEY_TYPE_RSA, privateKeyDer, privateKeyDerLength);
  }

  /**
   * Set the public and private key for the keyName.
   * @param keyName The key name.
   * @param keyType The KeyType, such as KEY_TYPE_RSA.
   * @param publicKeyDer The public key DER byte array.
   * @param publicKeyDerLength The length of publicKeyDer.
   * @param privateKeyDer The private key DER byte array.
   * @param privateKeyDerLength The length of privateKeyDer.
   */
  void
  setKeyPairForKeyName
    (const Name& keyName, KeyType keyType, const uint8_t* publicKeyDer,
     size_t publicKeyDerLength, const uint8_t* privateKeyDer,
     size_t privateKeyDerLength)
  {
    setPublicKeyForKeyName(keyName, keyType, publicKeyDer, publicKeyDerLength);
    setPrivateKeyForKeyName(keyName, keyType, privateKeyDer, privateKeyDerLength);
  }

  /**
   * @deprecated Use setKeyPairForKeyName(keyName, KEY_TYPE_RSA, publicKeyDer,
   * publicKeyDerLength, privateKeyDer, privateKeyDerLength).
   */
  void
  DEPRECATED_IN_NDN_CPP setKeyPairForKeyName
    (const Name& keyName, const uint8_t* publicKeyDer, size_t publicKeyDerLength,
     const uint8_t* privateKeyDer, size_t privateKeyDerLength)
  {
    setKeyPairForKeyName
      (keyName, KEY_TYPE_RSA, publicKeyDer, publicKeyDerLength, privateKeyDer,
       privateKeyDerLength);
  }

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
  sign(const uint8_t *data, size_t dataLength, const Name& keyName, DigestAlgorithm digestAlgorithm);

  /**
   * Decrypt data.
   * @param keyName The name of the decrypting key.
   * @param data The byte to be decrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric decryption is used.
   * @return The decrypted data.
   */
  virtual Blob
  decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric);

  /**
   * Encrypt data.
   * @param keyName The name of the encrypting key.
   * @param data The byte to be encrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric decryption is used.
   * @return The encrypted data.
   */
  virtual Blob
  encrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric);

  /**
   * @brief Generate a symmetric key.
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

private:
  /**
   * PrivateKey is a simple class to hold an RSA or EC private key.
   */
  class PrivateKey {
  public:
    PrivateKey(KeyType keyType, const uint8_t* keyDer, size_t keyDerLength);

    ~PrivateKey();

    KeyType getKeyType() const { return keyType_; }

    struct rsa_st* getRsaPrivateKey() const { return rsaPrivateKey_; }

    struct ec_key_st* getEcPrivateKey() const { return ecPrivateKey_; }

  private:
    KeyType keyType_;
    struct rsa_st* rsaPrivateKey_;
    struct ec_key_st* ecPrivateKey_;
  };

  std::map<std::string, ptr_lib::shared_ptr<PublicKey> > publicKeyStore_;   /**< The map key is the keyName.toUri() */
  std::map<std::string, ptr_lib::shared_ptr<PrivateKey> > privateKeyStore_; /**< The map key is the keyName.toUri() */
};

}

#endif
