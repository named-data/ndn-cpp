/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/algo/rsa https://github.com/named-data/ndn-group-encrypt
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

// (This is ported from ndn::gep::algo::Rsa, and named RsaAlgorithm because
// "Rsa" is very short and not all the Common Client Libraries have namespaces.)

#ifndef NDN_RSA_ALGORITHM_HPP
#define NDN_RSA_ALGORITHM_HPP

#include "../../security/key-params.hpp"
#include "encrypt-params.hpp"
#include "../encrypt-key.hpp"
#include "../decrypt-key.hpp"

namespace ndn {

/**
 * The RsaAlgorithm class provides static methods to manipulate keys, encrypt
 * and decrypt using RSA.
 * @note This class is an experimental feature. The API may change.
 */
class RsaAlgorithm {
public:
  /**
   * Generate a new random decrypt key for RSA based on the given params.
   * @param params The key params with the key size (in bits).
   * @return The new decrypt key (PKCS8-encoded private key).
   */
  static DecryptKey
  generateKey(const RsaKeyParams& params);

  /**
   * Derive a new encrypt key from the given decrypt key value.
   * @param keyBits The key value of the decrypt key (PKCS8-encoded private
   * key).
   * @return The new encrypt key (DER-encoded public key).
   */
  static EncryptKey
  deriveEncryptKey(const Blob& keyBits);

  /**
   * Decrypt the encryptedData using the keyBits according the encrypt params.
   * @param keyBits The key value (PKCS8-encoded private key).
   * @param encryptedData The data to decrypt.
   * @param params This decrypts according to params.getAlgorithmType().
   * @return The decrypted data.
   */
  static Blob
  decrypt
    (const Blob& keyBits, const Blob& encryptedData, const EncryptParams& params);

  /**
   * Encrypt the plainData using the keyBits according the encrypt params.
   * @param keyBits The key value (DER-encoded public key).
   * @param plainData The data to encrypt.
   * @param params This encrypts according to params.getAlgorithmType().
   * @return The encrypted data.
   */
  static Blob
  encrypt
    (const Blob& keyBits, const Blob& plainData, const EncryptParams& params);
};

}

#endif
