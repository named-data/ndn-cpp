/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2018 Regents of the University of California.
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

#include <stdexcept>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>

using namespace std;

namespace ndn {

DecryptKey
RsaAlgorithm::generateKey(const RsaKeyParams& params)
{
  ptr_lib::shared_ptr<TpmPrivateKey> privateKey =
    TpmPrivateKey::generatePrivateKey(params);
  return privateKey->toPkcs8();
}

EncryptKey
RsaAlgorithm::deriveEncryptKey(const Blob& keyBits)
{
  TpmPrivateKey privateKey;
  privateKey.loadPkcs8(keyBits.buf(), keyBits.size());
  return EncryptKey(privateKey.derivePublicKey());
}

#if NDN_CPP_HAVE_LIBCRYPTO
Blob
RsaAlgorithm::decrypt
  (const Blob& keyBits, const Blob& encryptedData, const EncryptParams& params)
{
  TpmPrivateKey privateKey;
  privateKey.loadPkcs8(keyBits.buf(), keyBits.size());
  return privateKey.decrypt
    (encryptedData.buf(), encryptedData.size(), params.getAlgorithmType());
}

Blob
RsaAlgorithm::encrypt
  (const Blob& keyBits, const Blob& plainData, const EncryptParams& params)
{
  RsaPublicKeyLite publicKey;
  if (publicKey.decode(keyBits) != NDN_ERROR_success)
    throw UnrecognizedKeyFormatException
      ("RsaAlgorithm: Error decoding public key");
  
  // TODO: use RSA_size, etc. to get the proper size of the output buffer.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedData(new vector<uint8_t>(1000));
  size_t encryptedDataLength;
  ndn_Error error;
  if ((error = publicKey.encrypt
       (plainData, params.getAlgorithmType(), &encryptedData->front(),
        encryptedDataLength))) {
    if (error == NDN_ERROR_Unsupported_algorithm_type)
      throw runtime_error("RsaAlgorithm: Unsupported padding scheme");
    else
      throw SecurityException(string("RsaAlgorithm: ") + ndn_getErrorString(error));
  }

  encryptedData->resize(encryptedDataLength);
  return Blob(encryptedData, false);
}
#endif

}
