/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/tpm/key-handle.hpp
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

#ifndef NDN_TPM_KEY_HANDLE_HPP
#define NDN_TPM_KEY_HANDLE_HPP

#include "../../name.hpp"
#include "../security-common.hpp"

namespace ndn {

/**
 * TpmKeyHandle is an abstract base class for a TPM key handle, which provides
 * an interface to perform cryptographic operations with a key in the TPM.
 */
class TpmKeyHandle {
public:
  virtual
  ~TpmKeyHandle();

  /**
   * Compute a digital signature from the byte array using this key with
   * digestAlgorithm.
   * @param digestAlgorithm The digest algorithm.
   * @param data The input byte array.
   * @param dataLength The length of the byte array.
   * @return The signature Blob, or an isNull Blob for an unrecognized
   * digestAlgorithm.
   */
  Blob
  sign
    (DigestAlgorithm digestAlgorithm, const uint8_t* data, size_t dataLength) const;

  /**
   * Return the plain text which is decrypted from cipherText using this key.
   * @param cipherText The cipher text byte array.
   * @param cipherTextLength The length of the cipher text byte array.
   * @return The decrypted data.
   */
  Blob
  decrypt(const uint8_t* cipherText, size_t cipherTextLength) const;

  /**
   * Get the encoded public key derived from this key.
   * @return The public key encoding Blob.
   */
  Blob
  derivePublicKey() const;

  void
  setKeyName(const Name& keyName) { keyName_ = keyName; }

  const Name&
  getKeyName() const { return keyName_; }

protected:
  TpmKeyHandle() {}

private:
  virtual Blob
  doSign
    (DigestAlgorithm digestAlgorithm, const uint8_t* data, size_t dataLength) const = 0;

  virtual Blob
  doDecrypt(const uint8_t* cipherText, size_t cipherTextLength) const = 0;

  virtual Blob
  doDerivePublicKey() const = 0;

  // Disable the copy constructor and assignment operator.
  TpmKeyHandle(const TpmKeyHandle& other);
  TpmKeyHandle& operator=(const TpmKeyHandle& other);

  Name keyName_;
};

}

#endif
