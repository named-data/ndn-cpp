/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/encryptor https://github.com/named-data/ndn-group-encrypt
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

#include <stdexcept>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/encrypt/encrypted-content.hpp>
#include <ndn-cpp/encoding/tlv-wire-format.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "../../c/util/ndn_memory.h"
#include <ndn-cpp/encrypt/algo/encryptor.hpp>

using namespace std;

namespace ndn {

/**
 * Encrypt the payload using the symmetric key according to params, and return
 * an EncryptedContent.
 * @param payload The data to encrypt.
 * @param key The key value.
 * @param keyName The key name for the EncryptedContent key locator.
 * @param params The parameters for encryption.
 * @return The EncryptedContent.
 */
static EncryptedContent
encryptSymmetric
  (const Blob& payload, const Blob& key, const Name& keyName,
   const EncryptParams& params)
{
  ndn_EncryptAlgorithmType algorithmType = params.getAlgorithmType();
  Blob initialVector = params.getInitialVector();
  EncryptedContent result;
  result.getKeyLocator().setType(ndn_KeyLocatorType_KEYNAME);
  result.getKeyLocator().setKeyName(keyName);

  if (algorithmType == ndn_EncryptAlgorithmType_AesCbc ||
      algorithmType == ndn_EncryptAlgorithmType_AesEcb) {
    if (algorithmType == ndn_EncryptAlgorithmType_AesCbc) {
      if (initialVector.size() != AesAlgorithm::BLOCK_SIZE)
        throw runtime_error("incorrect initial vector size");
    }

    Blob encryptedPayload = AesAlgorithm::encrypt(key, payload, params);

    result.setAlgorithmType(algorithmType);
    result.setPayload(encryptedPayload);
    result.setInitialVector(initialVector);
    return result;
  }
  else
    throw runtime_error("Unsupported encryption method");
}

/**
 * Encrypt the payload using the asymmetric key according to params, and
 * return an EncryptedContent.
 * @param payload The data to encrypt. The size should be within range of the
 * key.
 * @param key The key value.
 * @param keyName The key name for the EncryptedContent key locator.
 * @param params The parameters for encryption.
 * @return A new EncryptedContent.
 */
static EncryptedContent
encryptAsymmetric
  (const Blob& payload, const Blob& key, const Name& keyName,
   const EncryptParams& params)
{
  ndn_EncryptAlgorithmType algorithmType = params.getAlgorithmType();
  EncryptedContent result;
  result.getKeyLocator().setType(ndn_KeyLocatorType_KEYNAME);
  result.getKeyLocator().setKeyName(keyName);

  if (algorithmType == ndn_EncryptAlgorithmType_RsaPkcs ||
      algorithmType == ndn_EncryptAlgorithmType_RsaOaep) {
    Blob encryptedPayload = RsaAlgorithm::encrypt(key, payload, params);

    result.setAlgorithmType(algorithmType);
    result.setPayload(encryptedPayload);
    return result;
  }
  else
    throw runtime_error("Unsupported encryption method");
}

void
Encryptor::encryptData
  (Data& data, const Blob& payload, const Name& keyName, const Blob& key,
   const EncryptParams& params)
{
  data.getName().append(getNAME_COMPONENT_FOR()).append(keyName);

  ndn_EncryptAlgorithmType algorithmType = params.getAlgorithmType();

  if (algorithmType == ndn_EncryptAlgorithmType_AesCbc ||
      algorithmType == ndn_EncryptAlgorithmType_AesEcb) {
    EncryptedContent content = encryptSymmetric(payload, key, keyName, params);
    data.setContent(content.wireEncode(*TlvWireFormat::get()));
  }
  else if (algorithmType == ndn_EncryptAlgorithmType_RsaPkcs ||
           algorithmType == ndn_EncryptAlgorithmType_RsaOaep) {
    // Openssl doesn't have an easy way to get the maximum plain text size, so
    // try to encrypt the payload first and catch the error if it is too big.
    try {
      EncryptedContent content = encryptAsymmetric(payload, key, keyName, params);
      data.setContent(content.wireEncode(*TlvWireFormat::get()));
      return;
    } catch (SecurityException&) {
      // The payload is larger than the maximum plaintext size. Continue.
    }

    // 128-bit nonce.
    ptr_lib::shared_ptr<vector<uint8_t> > nonceKeyBuffer(new vector<uint8_t>(16));
    ndn_Error error;
    if ((error = CryptoLite::generateRandomBytes
         (&nonceKeyBuffer->front(), nonceKeyBuffer->size())))
      throw runtime_error(ndn_getErrorString(error));
    Blob nonceKey(nonceKeyBuffer, false);

    Name nonceKeyName(keyName);
    nonceKeyName.append("nonce");

    EncryptParams symmetricParams
      (ndn_EncryptAlgorithmType_AesCbc, AesAlgorithm::BLOCK_SIZE);

    EncryptedContent nonceContent = encryptSymmetric
      (payload, nonceKey, nonceKeyName, symmetricParams);

    EncryptedContent payloadContent = encryptAsymmetric
      (nonceKey, key, keyName, params);

    Blob nonceContentEncoding = nonceContent.wireEncode();
    Blob payloadContentEncoding = payloadContent.wireEncode();
    ptr_lib::shared_ptr<vector<uint8_t> > content(new vector<uint8_t>
      (nonceContentEncoding.size() + payloadContentEncoding.size()));
    ndn_memcpy(&content->front(), payloadContentEncoding.buf(),
               payloadContentEncoding.size());
    ndn_memcpy(&content->front() + payloadContentEncoding.size(),
               nonceContentEncoding.buf(), nonceContentEncoding.size());

    data.setContent(Blob(content, false));
  }
  else
    throw runtime_error("Unsupported encryption method");
}

Encryptor::Values* Encryptor::values_ = 0;

}
