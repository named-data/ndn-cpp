/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/algo/aes https://github.com/named-data/ndn-group-encrypt
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
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/encrypt/algo/aes-algorithm-lite.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>

using namespace std;

namespace ndn {

DecryptKey
AesAlgorithm::generateKey(const AesKeyParams& params)
{
  // Convert the key bit size to bytes.
  ptr_lib::shared_ptr<vector<uint8_t> > key
    (new vector<uint8_t>(params.getKeySize() / 8));
  ndn_Error error;
  if ((error = CryptoLite::generateRandomBytes(&key->front(), key->size())))
    throw runtime_error(ndn_getErrorString(error));

  DecryptKey decryptKey(Blob(key, false));
  return decryptKey;
}

Blob
AesAlgorithm::decrypt
  (const Blob& keyBits, const Blob& encryptedData, const EncryptParams& params)
{
  ndn_Error error;

  // For now, only support 128-bit.
  ptr_lib::shared_ptr<vector<uint8_t> > plainData
    (new vector<uint8_t>(encryptedData.size()));
  size_t plainDataLength;
  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesEcb) {
    if ((error = AesAlgorithmLite::decrypt128Ecb
         (keyBits, encryptedData, &plainData->front(), plainDataLength)))
      throw runtime_error(string("AesAlgorithm: ") + ndn_getErrorString(error));
  }
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesCbc) {
    if ((error = AesAlgorithmLite::decrypt128Cbc
         (keyBits, params.getInitialVector(), encryptedData,
          &plainData->front(), plainDataLength)))
      throw runtime_error(string("AesAlgorithm: ") + ndn_getErrorString(error));
  }
  else
    throw runtime_error("unsupported encryption mode");

  plainData->resize(plainDataLength);
  return Blob(plainData, false);
}

Blob
AesAlgorithm::encrypt
  (const Blob& keyBits, const Blob& plainData, const EncryptParams& params)
{
  ndn_Error error;

  // For now, only support 128-bit.
  // Add room for the padding.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedData
    (new vector<uint8_t>(plainData.size() + BLOCK_SIZE));
  size_t encryptedDataLength;
  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesEcb) {
    if ((error = AesAlgorithmLite::encrypt128Ecb
         (keyBits, plainData, &encryptedData->front(), encryptedDataLength)))
      throw runtime_error(string("AesAlgorithm: ") + ndn_getErrorString(error));
  }
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesCbc) {
    if ((error = AesAlgorithmLite::encrypt128Cbc
         (keyBits, params.getInitialVector(), plainData,
          &encryptedData->front(), encryptedDataLength)))
      throw runtime_error(string("AesAlgorithm: ") + ndn_getErrorString(error));
  }
  else
    throw runtime_error("unsupported encryption mode");

  encryptedData->resize(encryptedDataLength);
  return Blob(encryptedData, false);
}

size_t AesAlgorithm::BLOCK_SIZE = ndn_AES_128_BLOCK_SIZE;

}
