/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/transform/private-key.hpp
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

#ifndef NDN_TPM_PRIVATE_KEY_HPP
#define NDN_TPM_PRIVATE_KEY_HPP

#include <stdexcept>
#include "../../lite/security/ec-private-key-lite.hpp"
#include "../../lite/security/rsa-private-key-lite.hpp"
#include "../../util/blob.hpp"
#include "../key-params.hpp"
#include "../security-common.hpp"
#include "../../encoding/oid.hpp"
#include "../../c/encrypt/algo/encrypt-params-types.h"

namespace ndn {

class OID;
class DerNode;

/**
 * A TpmPrivateKey holds an in-memory private key and provides cryptographic
 * operations such as for signing by the in-memory TPM.
 */
class TpmPrivateKey {
public:
  /**
   * A TpmPrivateKey::Error extends runtime_error and represents an error in
   * private key processing.
   */
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  /**
   * Create an uninitialized TpmPrivateKey. You must call a load method to
   * initialize it, such as loadPkcs1.
   */
  TpmPrivateKey()
  : keyType_((KeyType)-1)
  {}

  /**
   * Load the unencrypted private key from a buffer with the PKCS #1 encoding.
   * This replaces any existing private key in this object.
   * @param encoding The byte array with the private key encoding.
   * @param encodingLength The length of the encoding.
   * @param keyType (potional) The KeyType, such as KEY_TYPE_RSA. If omitted,
   * then partially decode the private key to determine the key type.
   * @throws TpmPrivateKey::Error for errors decoding the key.
   */
  void
  loadPkcs1
    (const uint8_t* encoding, size_t encodingLength,
     KeyType keyType = (KeyType)-1);

  /**
   * Load the unencrypted private key from a buffer with the PKCS #8 encoding.
   * This replaces any existing private key in this object.
   * @param encoding The byte array with the private key encoding.
   * @param encodingLength The length of the encoding.
   * @throws TpmPrivateKey::Error for errors decoding the key.
   */
  void
  loadPkcs8(const uint8_t* encoding, size_t encodingLength);

  /**
   * Load the encrypted private key from a buffer with the PKCS #8 encoding of
   * the EncryptedPrivateKeyInfo.
   * This replaces any existing private key in this object.
   * @param encoding The byte array with the private key encoding.
   * @param encodingLength The length of the encoding.
   * @param password The password for decrypting the private key, which should
   * have characters in the range of 1 to 127.
   * @param passwordLength The length of the password.
   * @throws TpmPrivateKey::Error for errors decoding or decrypting the key.
   */
  void
  loadEncryptedPkcs8
    (const uint8_t* encoding, size_t encodingLength, const uint8_t* password,
     size_t passwordLength);

  /**
   * Get the encoded public key for this private key.
   * @return The public key encoding Blob.
   * @throws TpmPrivateKey::Error if no private key is loaded, or error
   * converting to a public key.
   */
  Blob
  derivePublicKey() const;

  /**
   * Decrypt the cipherText using this private key according the encryption
   * algorithmType. Only RSA encryption is supported for now.
   * @param cipherText The cipher text byte array.
   * @param cipherTextLength The length of the cipher text byte array.
   * @param algorithmType (optional) This decrypts according to algorithmType.
   * If omitted, use RsaOaep.
   * @return The decrypted data.
   * @throws TpmPrivateKey::Error if the private key is not loaded, if
   * decryption is not supported for this key type, or for error decrypting.
   */
  Blob
  decrypt
    (const uint8_t* cipherText, size_t cipherTextLength,
     ndn_EncryptAlgorithmType algorithmType = ndn_EncryptAlgorithmType_RsaOaep);

  /**
   * Sign the data with this private key, returning a signature Blob.
   * @param data The input byte array.
   * @param dataLength The length of the byte array.
   * @param digestAlgorithm the digest algorithm.
   * @return The signature Blob, or an isNull Blob if this private key is not
   * initialized.
   * @throws TpmPrivateKey::Error for unrecognized digestAlgorithm or an error in
   * signing.
   */
  Blob
  sign(const uint8_t *data, size_t dataLength, DigestAlgorithm digestAlgorithm);

  /**
   * Get the encoded unencrypted private key in PKCS #1.
   * @param includeParameters (optional) If true and this is an EC key, then
   * include the EC parameters in the encoding. If omitted, use true. Using
   * false is deprecated.
   * @return The private key encoding Blob.
   * @throws TpmPrivateKey::Error if no private key is loaded, or error encoding.
   */
  Blob
  toPkcs1(bool includeParameters = true);

  /**
   * Get the encoded unencrypted private key in PKCS #8.
   * @param includeParameters (optional) If true and this is an EC key, then
   * include the EC parameters in the encoding. If omitted, use true. Using
   * false is deprecated.
   * @return The private key encoding Blob.
   * @throws TpmPrivateKey::Error if no private key is loaded, or error encoding.
   */
  Blob
  toPkcs8(bool includeParameters = true);

  /**
   * Get the encoded encrypted private key in PKCS #8.
   * @param password The password for encrypting the private key, which should
   * have characters in the range of 1 to 127.
   * @param passwordLength The length of the password.
   * @param includeParameters (optional) If true and this is an EC key, then
   * include the EC parameters in the encoding. If omitted, use true. Using
   * false is deprecated.
   * @return The encoding Blob of the EncryptedPrivateKeyInfo.
   * @throws TpmPrivateKey::Error if no private key is loaded, or error encoding.
   */
  Blob
  toEncryptedPkcs8
    (const uint8_t* password, size_t passwordLength,
     bool includeParameters = true);

  /**
   * Generate a key pair according to keyParams and return a new TpmPrivateKey
   * with the private key. You can get the public key with derivePublicKey.
   * @param keyParams The parameters of the key.
   * @return A new TpmPrivateKey.
   * @throws std::invalid_argument if the key type is not supported.
   * @throws TpmPrivateKey::Error for an invalid key size, or an error generating.
   */
  static ptr_lib::shared_ptr<TpmPrivateKey>
  generatePrivateKey(const KeyParams& keyParams);

private:
  /**
   * Encode the private key to a PKCS #8 private key. We do this explicitly here
   * to avoid linking to extra OpenSSL libraries.
   * @param privateKeyDer The input private key DER.
   * @param oid The OID of the privateKey.
   * @param parameters The DerNode of the parameters for the OID.
   * @return The PKCS #8 private key DER.
   */
  static Blob
  encodePkcs8PrivateKey
    (const std::vector<uint8_t>& privateKeyDer, const OID& oid,
     const ptr_lib::shared_ptr<DerNode>& parameters);

  /**
   * Encode the bitString into a SubjectPublicKeyInfo.
   * @param oid The OID of the privateKey.
   * @param parameters The DerNode of the parameters for the OID.
   * @param bitstring The public key bitString which is already in a
   * DerNode::DerBitString.
   * @return The subject public key info DER.
   */
  static Blob
  encodeSubjectPublicKeyInfo
    (const OID& oid, const ptr_lib::shared_ptr<DerNode>& parameters,
     const ptr_lib::shared_ptr<DerNode>& bitString);

  /**
   * Set the EC key using the curve in the algorithmParameters, decode the
   * privateKeyDer and set the private key value. This is necessary because
   * d2i_ECPrivateKey does not seem to work with the "parameterless" private
   * key encoding produced by ndn-cxx.
   * @param algorithmParameters The parameters from the PKCS #8 AlgorithmIdentifier.
   * @param privateKeyDer The bytes of the inner PKCS #8 private key.
   * @param privateKey The EcPrivateKeyLite to set.
   * @throws TpmPrivateKey::Error if can't decode the private key.
   */
  static void
  decodeEcPrivateKey
    (const ptr_lib::shared_ptr<DerNode>& algorithmParameters,
     const Blob& privateKeyDer, EcPrivateKeyLite& privateKey);

  /**
   * Get the curve ID from ecPrivateKey and return the OID from EC_KEY_INFO.
   * @param ecPrivateKey The EcPrivateKeyLite.
   * @return The OID
   * @throws TpmPrivateKey::Error if can't get the curve ID or find it in
   * EC_KEY_INFO.
   */
  static OID
  getEcOid(const EcPrivateKeyLite& ecPrivateKey);

  // Disable the copy constructor and assignment operator.
  TpmPrivateKey(const TpmPrivateKey& other);
  TpmPrivateKey& operator=(const TpmPrivateKey& other);

  KeyType keyType_;
  ptr_lib::shared_ptr<EcPrivateKeyLite> ecPrivateKey_;
  ptr_lib::shared_ptr<RsaPrivateKeyLite> rsaPrivateKey_;
};

}

#endif
