/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/tpm/back-end.hpp
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

#ifndef NDN_TPM_BACK_END_HPP
#define NDN_TPM_BACK_END_HPP

#include <stdexcept>
#include "../../name.hpp"
#include "../key-params.hpp"

namespace ndn {

class TpmKeyHandle;

/**
 * TpmBackEnd is an abstract base class for a TPM backend implementation which
 * provides a TpmKeyHandle to the TPM front end. This class defines the
 * interface that an actual TPM backend implementation should provide, for
 * example TpmBackEndMemory.
 */
class TpmBackEnd {
public:
  /**
   * A TpmBackEnd::Error extends runtime_error and represents a non-semantic
   * error in backend TPM processing.
   */
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  virtual
  ~TpmBackEnd();

  /**
   * Check if the key with name keyName exists in the TPM.
   * @param keyName The name of the key.
   * @return True if the key exists.
   */
  bool
  hasKey(const Name& keyName) const { return doHasKey(keyName); }

  /**
   * Get the handle of the key with name keyName.
   * Calling getKeyHandle multiple times with the same keyName will return
   * different TpmKeyHandle objects that all refer to the same key.
   * @param keyName The name of the key.
   * @return The handle of the key, or null if the key does not exist.
   */
  ptr_lib::shared_ptr<TpmKeyHandle>
  getKeyHandle(const Name& keyName) const { return doGetKeyHandle(keyName); }

  /**
   * Create a key for the identityName according to params.
   * @param identityName The name if the identity.
   * @param params The KeyParams for creating the key.
   * @return The handle of the created key.
   * @throws Tpm::Error if params is invalid.
   * @throws TpmBackEnd::Error if the key cannot be created.
   */
  ptr_lib::shared_ptr<TpmKeyHandle>
  createKey(const Name& identityName, const KeyParams& params);

  /**
   * Delete the key with name keyName. If the key doesn't exist, do nothing.
   * Note: Continuing to use existing Key handles on a deleted key results in
   * undefined behavior.
   * @param keyName The name of the key to delete.
   * @throws TpmBackEnd::Error if the deletion fails.
   */
  void
  deleteKey(const Name& keyName) { doDeleteKey(keyName); }

  /**
   * Get the encoded private key with name keyName in PKCS #8 format, possibly
   * password-encrypted.
   * @param keyName The name of the key in the TPM.
   * @param password The password for encrypting the private key, which should
   * have characters in the range of 1 to 127. If the password is supplied, use
   * it to return a PKCS #8 EncryptedPrivateKeyInfo. If the password is null,
   * return an unencrypted PKCS #8 PrivateKeyInfo.
   * @param passwordLength The length of the password. If password is null, this
   * is ignored.
   * @return The encoded private key.
   * @throws TpmBackEnd::Error if the key does not exist or if the key cannot be
   * exported, e.g., insufficient privileges.
   */
  Blob
  exportKey(const Name& keyName, const uint8_t* password, size_t passwordLength);

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
   * @throws TpmBackEnd::Error if a key with name keyName already exists, or for
   * an error importing the key.
   */
  void
  importKey
    (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
     const uint8_t* password, size_t passwordLength);

  /**
   * Check if the TPM is in terminal mode. The default implementation always
   * returns true.
   * @return True if in terminal mode.
   */
  virtual bool
  isTerminalMode() const;

  /**
   * Set the terminal mode of the TPM. In terminal mode, the TPM will not ask
   * for a password from the GUI. The default implementation does nothing.
   * @param isTerminal True to enable terminal mode.
   */
  virtual void
  setTerminalMode(bool isTerminal) const;

  /**
   * Check if the TPM is locked. The default implementation returns false.
   * @return True if the TPM is locked, otherwise false.
   */
  virtual bool
  isTpmLocked() const;

  /**
   * Unlock the TPM. If !isTerminalMode(), prompt for a password from the GUI.
   * The default implementation does nothing and returns !isTpmLocked().
   * @param password The password to unlock TPM.
   * @param passwordLength The length of the password.
   * @return True if the TPM was unlocked.
   */
  virtual bool
  unlockTpm(const uint8_t* password, size_t passwordLength) const;

protected:
  TpmBackEnd() {}

  /**
   * Set the key name in keyHandle according to identityName and params.
   */
  static void
  setKeyName
    (TpmKeyHandle& keyHandle, const Name& identityName, const KeyParams& params);

private:
  /**
   * Check if the key with name keyName exists in the TPM.
   * @param keyName The name of the key.
   * @return True if the key exists.
   */
  virtual bool
  doHasKey(const Name& keyName) const = 0;

  /**
   * Get the handle of the key with name keyName.
   * @param keyName The name of the key.
   * @return The handle of the key, or null if the key does not exist.
   */
  virtual ptr_lib::shared_ptr<TpmKeyHandle>
  doGetKeyHandle(const Name& keyName) const = 0;

  /**
   * Create a key for identityName according to params. The created key is
   * named as: /<identityName>/[keyId]/KEY . The key name is set in the returned
   * TpmKeyHandle.
   * @param identityName The name if the identity.
   * @param params The KeyParams for creating the key.
   * @return The handle of the created key.
   * @throws TpmBackEnd::Error if the key cannot be created.
   */
  virtual ptr_lib::shared_ptr<TpmKeyHandle>
  doCreateKey(const Name& identityName, const KeyParams& params) = 0;

  /**
   * Delete the key with name keyName. If the key doesn't exist, do nothing.
   * @param keyName The name of the key to delete.
   * @throws TpmBackEnd::Error if the deletion fails.
   */
  virtual void
  doDeleteKey(const Name& keyName) = 0;

  /**
   * Get the encoded private key with name keyName in PKCS #8 format, possibly
   * password-encrypted.
   * @param keyName The name of the key in the TPM.
   * @param password The password for encrypting the private key, which should
   * have characters in the range of 1 to 127. If the password is supplied, use
   * it to return a PKCS #8 EncryptedPrivateKeyInfo. If the password is null,
   * return an unencrypted PKCS #8 PrivateKeyInfo.
   * @param passwordLength The length of the password. If password is null, this
   * is ignored.
   * @return The encoded private key.
   * @throws TpmBackEnd::Error if the key does not exist or if the key cannot be
   * exported, e.g., insufficient privileges.
   */
  virtual Blob
  doExportKey(const Name& keyName, const uint8_t* password, size_t passwordLength);

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
   * @throws TpmBackEnd::Error for an error importing the key.
   */
  virtual void
  doImportKey
    (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
     const uint8_t* password, size_t passwordLength);

  // Disable the copy constructor and assignment operator.
  TpmBackEnd(const TpmBackEnd& other);
  TpmBackEnd& operator=(const TpmBackEnd& other);
};

}

#endif
