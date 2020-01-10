/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/tpm/tpm.hpp
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

#ifndef NDN_TPM_HPP
#define NDN_TPM_HPP

#include <stdexcept>
#include <map>
#include "../../name.hpp"
#include "../key-params.hpp"

namespace ndn {

class TpmKeyHandle;
class TpmBackEnd;

/**
 * The TPM (Trusted Platform Module) stores the private portion of a user's
 * cryptography keys. The format and location of stored information is indicated
 * by the TPM locator. The TPM is designed to work with a PIB (Public
 * Information Base) which stores public keys and related information such as
 * certificates.
 *
 * The TPM also provides functionalities of cryptographic transformation, such
 * as signing and decryption.
 *
 * A TPM consists of a unified front-end interface and a backend implementation.
 * The front-end caches the handles of private keys which are provided by the
 * backend implementation.
 *
 * Note: A Tpm instance is created and managed only by the KeyChain. It is
 * returned by the KeyChain getTpm() method, through which it is possible to
 * check for the existence of private keys, get public keys for the private
 * keys, sign, and decrypt the supplied buffers using managed private keys.
 */
class Tpm {
public:
  /**
   * A Tpm::Error extends runtime_error and represents a semantic error in TPM
   * processing.
   */
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  std::string
  getTpmLocator() const;

  /**
   * Check if the key with name keyName exists in the TPM.
   * @param keyName The name of the key.
   * @return True if the key exists.
   */
  bool
  hasKey(const Name& keyName) const;

  /**
   * Get the public portion of an asymmetric key pair with name keyName.
   * @param keyName The name of the key.
   * @return The encoded public key, or an isNull Blob if the key does not exist.
   */
  Blob
  getPublicKey(const Name& keyName) const;

  /**
   * Compute a digital signature from the byte array using the key with name
   * keyName.
   * @param data The input byte array.
   * @param dataLength The length of the byte array.
   * @param keyName The name of the key.
   * @param digestAlgorithm The digest algorithm for the signature.
   * @return The signature Blob, or an isNull Blob if the key does not exist, or
   * for an unrecognized digestAlgorithm.
   */
  Blob
  sign
    (const uint8_t* data, size_t dataLength, const Name& keyName,
     DigestAlgorithm digestAlgorithm) const;

  /**
   * Return the plain text which is decrypted from cipherText using the key
   * with name keyName.
   * @param cipherText The cipher text byte array.
   * @param cipherTextLength The length of the cipher text byte array.
   * @param keyName The name of the key.
   * @return The decrypted data, or an isNull Blob if the key does not exist.
   */
  Blob
  decrypt
   (const uint8_t* cipherText, size_t cipherTextLength,
    const Name& keyName) const;

  // TPM Management

  /**
   * Check if the TPM is in terminal mode.
   * @return True if in terminal mode.
   */
  bool
  isTerminalMode() const;

  /**
   * Set the terminal mode of the TPM. In terminal mode, the TPM will not ask
   * for a password from the GUI.
   * @param isTerminal True to enable terminal mode.
   */
  void
  setTerminalMode(bool isTerminal) const;

  /**
   * Check if the TPM is locked.
   * @return True if the TPM is locked, otherwise false.
   */
  bool
  isTpmLocked() const;

  /**
   * Unlock the TPM. If !isTerminalMode(), prompt for a password from the GUI.
   * @param password The password to unlock TPM.
   * @param passwordLength The length of the password.
   * @return True if the TPM was unlocked.
   */
  bool
  unlockTpm(const uint8_t* password, size_t passwordLength) const;

private:
  friend class KeyChain;
  friend class SafeBag;

  /*
   * Create a new TPM instance with the specified location. This constructor
   * should only be called by KeyChain.
   * @param scheme The scheme for the TPM.
   * @param location The location for the TPM.
   * @param backEnd The TPM back-end implementation.
   */
  Tpm(const std::string& scheme, const std::string& location,
      const ptr_lib::shared_ptr<TpmBackEnd>& backEnd);

  TpmBackEnd*
  getBackEnd() { return backEnd_.get(); }

  /**
   * Create a key for the identityName according to params. The created key is
   * named /<identityName>/[keyId]/KEY .
   * @param identityName The name if the identity.
   * @param params The KeyParams for creating the key.
   * @return The name of the created key.
   * @throws Tpm::Error if params is invalid or the key type is unsupported.
   * @throws TpmBackEnd::Error if the key already exists or cannot be created.
   */
  Name
  createKey(const Name& identityName, const KeyParams& params);

  /**
   * Delete the key with name keyName. If the key doesn't exist, do nothing.
   * Note: Continuing to use existing Key handles on a deleted key results in
   * undefined behavior.
   * @throws TpmBackEnd::Error if the deletion fails.
   */
  void
  deleteKey(const Name& keyName);

  /**
   * Get the encoded private key with name keyName in PKCS #8 format, possibly
   * encrypted.
   * @param keyName The name of the key in the TPM.
   * @param password The password for encrypting the private key, which should
   * have characters in the range of 1 to 127. If the password is supplied, use
   * it to return a PKCS #8 EncryptedPrivateKeyInfo. If the password is null,
   * return an unencrypted PKCS #8 PrivateKeyInfo.
   * @param passwordLength The length of the password. If password is null, this
   * is ignored.
   * @return The private key encoded in PKCS #8 format.
   * @throws TpmBackEnd::Error if the key does not exist or if the key cannot be
   * exported, e.g., insufficient privileges.
   */
  Blob
  exportPrivateKey
    (const Name& keyName, const uint8_t* password, size_t passwordLength);

  /**
   * Import an encoded private key with name keyName in PKCS #8 format, possibly
   * password-encrypted.
   * @param keyName The name of the key to use in the TPM.
   * @param pkcs8 The input byte array. If the password is supplied, this is a
   * PKCS #8 EncryptedPrivateKeyInfo. If the password is null, this is an
   * unencrypted PKCS #8 PrivateKeyInfo.
   * @param pkcs8Length The length of the input byte array.
   * @param password The password for decrypting the private key, which should
   * have characters in the range of 1 to 127. If the password is supplied, use
   * it to decrypt the PKCS #8 EncryptedPrivateKeyInfo. If the password is null,
   * import an unencrypted PKCS #8 PrivateKeyInfo.
   * @param passwordLength The length of the password. If password is null, this
   * is ignored.
   * @throws TpmBackEnd::Error if the key cannot be imported.
   */
  void
  importPrivateKey
    (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
     const uint8_t* password, size_t passwordLength);

  /**
   * Get the TpmKeyHandle with name keyName, using backEnd_->getKeyHandle if it
   * is not already cached in keys_.
   * @param keyName The name of the key.
   * @return A pointer to the key handle in the keys_ cache, or null if no key
   * exists with name keyName.
   */
  const TpmKeyHandle*
  findKey(const Name& keyName) const;

  // Disable the copy constructor and assignment operator.
  Tpm(const Tpm& other);
  Tpm& operator=(const Tpm& other);

  std::string scheme_;
  std::string location_;

  std::map<Name, ptr_lib::shared_ptr<TpmKeyHandle>> keys_;

  ptr_lib::shared_ptr<TpmBackEnd> backEnd_;
};

}

#endif
