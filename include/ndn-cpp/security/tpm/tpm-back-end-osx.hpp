/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/tpm/back-end-osx.hpp
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

#ifndef NDN_TPM_BACK_END_OSX_HPP
#define NDN_TPM_BACK_END_OSX_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_OSX_SECURITY 1.
#include "../../ndn-cpp-config.h"
#if NDN_CPP_HAVE_OSX_SECURITY

#include "helper-osx.hpp"
#include "tpm-back-end.hpp"

namespace ndn {

/**
 * TpmBackEndOsx extends TpmBackEnd to implement a TPM back-end using the macOS
 * Keychain services.
 */
class TpmBackEndOsx : public TpmBackEnd {
public:
  /**
   * A TpmBackEndOsx::Error extends TpmBackEnd::Error and represents a
   * non-semantic error in the backend TPM for the macOS Keychain services.
   */
  class Error : public TpmBackEnd::Error
  {
  public:
    Error(const std::string& what)
    : TpmBackEnd::Error(what)
    {
    }
  };

  /**
   * Create TpmBackEndOsx to use the macOS KeyChain service.
   * @param location This is not used. (It is required by the TPM-registration
   * interface.)
   */
  TpmBackEndOsx(const std::string& location = "");

  static std::string
  getScheme() { return "tpm-osxkeychain"; }

  // Management

  /**
   * Check if the TPM is in terminal mode.
   * @return True if in terminal mode.
   */
  virtual bool
  isTerminalMode() const;

  /**
   * Set the terminal mode of the TPM. In terminal mode, the TPM will not ask
   * for a password from the GUI.
   * @param isTerminal True to enable terminal mode.
   */
  virtual void
  setTerminalMode(bool isTerminal) const;

  /**
   * Check if the TPM is locked.
   * @return True if the TPM is locked, otherwise false.
   */
  virtual bool
  isTpmLocked() const;

  /**
   * Unlock the TPM. If !isTerminalMode(), prompt for a password from the GUI.
   * @param password The password to unlock TPM.
   * @param passwordLength The length of the password.
   * @return True if the TPM was unlocked.
   */
  virtual bool
  unlockTpm(const uint8_t* password, size_t passwordLength) const;

  // Cryptographic transformation

  /**
   * Sign the data byte array with key using digestAlgorithm.
   */
  static Blob
  sign
    (const KeyRefOsx& key, DigestAlgorithm digestAlgorithm,
     const uint8_t* data, size_t dataLength);

  static Blob
  decrypt
    (const KeyRefOsx& key, const uint8_t* cipherText, size_t cipherTextLength);

  static Blob
  derivePublicKey(const KeyRefOsx& key);

private:
  /**
   * Check if the key with name keyName exists in the TPM.
   * @param keyName The name of the key.
   * @return True if the key exists.
   */
  virtual bool
  doHasKey(const Name& keyName) const;

  /**
   * Get the handle of the key with name keyName.
   * @param keyName The name of the key.
   * @return The handle of the key, or null if the key does not exist.
   */
  virtual ptr_lib::shared_ptr<TpmKeyHandle>
  doGetKeyHandle(const Name& keyName) const;

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
  doCreateKey(const Name& identityName, const KeyParams& params);

  /**
   * Delete the key with name keyName. If the key doesn't exist, do nothing.
   * @param keyName The name of the key to delete.
   * @throws TpmBackEnd::Error if the deletion fails.
   */
  virtual void
  doDeleteKey(const Name& keyName);

  // TODO: doExportKey.
  // TODO: doImportKey.

  /**
   * Get the private key reference with name keyName.
   * @param keyName The name of the key.
   * @returns A reference to the key.
   */
  static CFReleaser<SecKeychainItemRef>
  getKey(const Name& keyName);

  SecKeychainRef keyChainRef_;
  bool isTerminalMode_;
};

}

#endif // NDN_CPP_HAVE_OSX_SECURITY

#endif
