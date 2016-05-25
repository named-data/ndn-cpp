/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016 Regents of the University of California.
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
#include "../../c/util/crypto.h"
#if NDN_CPP_HAVE_LIBCRYPTO
#include <openssl/evp.h>
#endif
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>

using namespace std;

namespace ndn {

DecryptKey
AesAlgorithm::generateKey(const AesKeyParams& params)
{
  // Convert the key bit size to bytes.
  ptr_lib::shared_ptr<vector<uint8_t> > key
    (new vector<uint8_t>(params.getKeySize() / 8));
  ndn_generateRandomBytes(&key->front(), key->size());

  DecryptKey decryptKey(Blob(key, false));
  return decryptKey;
}

#if NDN_CPP_HAVE_LIBCRYPTO
Blob
AesAlgorithm::decrypt
  (const Blob& keyBits, const Blob& encryptedData, const EncryptParams& params)
{
  EVP_CIPHER_CTX ctx;

  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesEcb)
    EVP_DecryptInit
      (&ctx, EVP_aes_128_ecb(), (const unsigned char*)keyBits.buf(), 0);
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesCbc) {
    if (params.getInitialVector().size() != BLOCK_SIZE)
      throw runtime_error("incorrect initial vector size");

    EVP_DecryptInit
      (&ctx, EVP_aes_128_cbc(), (const unsigned char*)keyBits.buf(),
       (const unsigned char*)params.getInitialVector().buf());
  }
  else
    throw runtime_error("unsupported encryption mode");

  ptr_lib::shared_ptr<vector<uint8_t> > plainData
    (new vector<uint8_t>(encryptedData.size()));
  int outLength1;
  EVP_DecryptUpdate
    (&ctx, (unsigned char*)&plainData->front(), &outLength1,
     (const unsigned char*)encryptedData.buf(), encryptedData.size());
  int outLength2;
  EVP_DecryptFinal
    (&ctx, (unsigned char*)&plainData->front() + outLength1, &outLength2);
  EVP_CIPHER_CTX_cleanup(&ctx);

  plainData->resize(outLength1 + outLength2);
  return Blob(plainData, false);
}

Blob
AesAlgorithm::encrypt
  (const Blob& keyBits, const Blob& plainData, const EncryptParams& params)
{
  EVP_CIPHER_CTX ctx;

  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesEcb)
    // TODO: Select aes_128, aes_256, etc. based on the keyBits size.
    EVP_EncryptInit
      (&ctx, EVP_aes_128_ecb(), (const unsigned char*)keyBits.buf(), 0);
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_AesCbc) {
    if (params.getInitialVector().size() != BLOCK_SIZE)
      throw runtime_error("incorrect initial vector size");

    EVP_EncryptInit
      (&ctx, EVP_aes_128_cbc(), (const unsigned char*)keyBits.buf(),
       (const unsigned char*)params.getInitialVector().buf());
  }
  else
    throw runtime_error("unsupported encryption mode");

  // Add room for the padding.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedData
    (new vector<uint8_t>(plainData.size() + BLOCK_SIZE));
  int outLength1;
  EVP_EncryptUpdate
    (&ctx, (unsigned char*)&encryptedData->front(), &outLength1,
     (const unsigned char*)plainData.buf(), plainData.size());
  int outLength2;
  EVP_EncryptFinal
    (&ctx, (unsigned char*)&encryptedData->front() + outLength1, &outLength2);
  EVP_CIPHER_CTX_cleanup(&ctx);

  encryptedData->resize(outLength1 + outLength2);
  return Blob(encryptedData, false);
}
#endif

size_t AesAlgorithm::BLOCK_SIZE = 16;

}