/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2019 Regents of the University of California.
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

#ifndef NDN_FILE_PRIVATE_KEY_STORAGE_HPP
#define NDN_FILE_PRIVATE_KEY_STORAGE_HPP

#include <string>
#include "private-key-storage.hpp"

namespace ndn {

class DerNode;

/**
 * FilePrivateKeyStorage extends PrivateKeyStorage to implement private key
 * storage using files.
 */
class FilePrivateKeyStorage : public PrivateKeyStorage {
public:
  /**
   * Create a new FilePrivateKeyStorage to connect to the given or default
   * directory.
   * @param keyStoreDirectoryPath (optional) The path of the directory to store
   * private keys. This creates the directory if it doesn't exist. If omitted or
   * empty use $HOME/.ndn/ndnsec-tpm-file .
   */
  FilePrivateKeyStorage(const std::string& keyStoreDirectoryPath = "");

  /**
   * The virtual Destructor.
   */
  virtual
  ~FilePrivateKeyStorage();

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
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric encryption is used.
   * @return The decrypted data.
   */
  virtual Blob
  decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric = false);

  /**
   * Encrypt data.
   * @param keyName The name of the encrypting key.
   * @param data The byte to be encrypted.
   * @param dataLength the length of data.
   * @param isSymmetric If true symmetric encryption is used, otherwise asymmetric encryption is used.
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

private:
  std::string
  nameTransform(const std::string& keyName, const std::string& extension);

  /**
   * Use nameTransform to get the file path for keyName (without the extension)
   * and also add to the mapping.txt file.
   * @param keyName The key name which is transformed to a file path.
   * @return The key file path without the extension.
   */
  std::string
  maintainMapping(const std::string& keyName);

  std::string keyStorePath_;
};

}

#endif
