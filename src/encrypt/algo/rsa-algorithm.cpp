/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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
#include "../../encoding/der/der-node.hpp"
#include "../../encoding/der/der-exception.hpp"
#include <ndn-cpp/security/identity/private-key-storage.hpp>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/lite/security/rsa-private-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;
static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";

DecryptKey
RsaAlgorithm::generateKey(const RsaKeyParams& params)
{
  RsaPrivateKeyLite privateKey;
  ndn_Error error;
  if ((error = privateKey.generate(params.getKeySize())))
    throw SecurityException(string("RsaAlgorithm: ") + ndn_getErrorString(error));

  // Get the encoding length and encode.
  size_t encodingLength;
  if ((error = privateKey.encodePrivateKey(0, encodingLength)))
    throw SecurityException(string("RsaAlgorithm: ") + ndn_getErrorString(error));
  vector<uint8_t> pkcs1PrivateKeyDer(encodingLength);
  if ((error = privateKey.encodePrivateKey(&pkcs1PrivateKeyDer[0], encodingLength)))
    throw SecurityException(string("RsaAlgorithm: ") + ndn_getErrorString(error));

  return DecryptKey(PrivateKeyStorage::encodePkcs8PrivateKey
    (pkcs1PrivateKeyDer, OID(RSA_ENCRYPTION_OID),
     ptr_lib::make_shared<DerNode::DerNull>()));
}

EncryptKey
RsaAlgorithm::deriveEncryptKey(const Blob& keyBits)
{
  // Decode the PKCS #8 private key.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(keyBits.buf(), 0);
  const std::vector<ptr_lib::shared_ptr<DerNode> >& pkcs8Children =
    parsedNode->getChildren();
  const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
    DerNode::getSequence(pkcs8Children, 1).getChildren();
  string oidString
    (dynamic_cast<DerNode::DerOid&>(*algorithmIdChildren[0]).toVal().toRawStr());
  Blob rsaPrivateKeyDer = pkcs8Children[2]->toVal();

  if (oidString != RSA_ENCRYPTION_OID)
    throw DerDecodingException
      ("RsaAlgorithm: The PKCS #8 private key is not RSA_ENCRYPTION");

  // Decode the PKCS #1 RSAPrivateKey.
  parsedNode = DerNode::parse(rsaPrivateKeyDer.buf(), 0);
  const std::vector<ptr_lib::shared_ptr<DerNode> >& rsaPrivateKeyChildren =
    parsedNode->getChildren();
  ptr_lib::shared_ptr<DerNode> modulus = rsaPrivateKeyChildren[1];
  ptr_lib::shared_ptr<DerNode> publicExponent = rsaPrivateKeyChildren[2];

  // Encode the PKCS #1 RSAPublicKey.
  ptr_lib::shared_ptr<DerSequence> rsaPublicKey(new DerSequence());
  rsaPublicKey->addChild(modulus);
  rsaPublicKey->addChild(publicExponent);
  Blob rsaPublicKeyDer = rsaPublicKey->encode();

  // Encode the SubjectPublicKeyInfo.
  Blob publicKeyDer = PrivateKeyStorage::encodeSubjectPublicKeyInfo
    (OID(RSA_ENCRYPTION_OID), ptr_lib::make_shared<DerNode::DerNull>(),
     ptr_lib::make_shared<DerNode::DerBitString>
       (rsaPublicKeyDer.buf(), rsaPublicKeyDer.size(), 0));

  return EncryptKey(publicKeyDer);
}

#if NDN_CPP_HAVE_LIBCRYPTO
Blob
RsaAlgorithm::decrypt
  (const Blob& keyBits, const Blob& encryptedData, const EncryptParams& params)
{
  // Decode the PKCS #8 private key.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(keyBits.buf(), 0);
  const std::vector<ptr_lib::shared_ptr<DerNode> >& pkcs8Children =
    parsedNode->getChildren();
  const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
    DerNode::getSequence(pkcs8Children, 1).getChildren();
  string oidString
    (dynamic_cast<DerNode::DerOid&>(*algorithmIdChildren[0]).toVal().toRawStr());
  Blob rsaPrivateKeyDer = pkcs8Children[2]->toVal();

  if (oidString != RSA_ENCRYPTION_OID)
    throw DerDecodingException
      ("RsaAlgorithm: The PKCS #8 private key is not RSA_ENCRYPTION");

  RsaPrivateKeyLite privateKey;
  if (privateKey.decode(rsaPrivateKeyDer) != NDN_ERROR_success)
    throw UnrecognizedKeyFormatException
      ("RsaAlgorithm: Error decoding the private key");

  // TODO: use RSA_size, etc. to get the proper size of the output buffer.
  ptr_lib::shared_ptr<vector<uint8_t> > plainData(new vector<uint8_t>(1000));
  size_t plainDataLength;
  ndn_Error error;
  if ((error = privateKey.decrypt
       (encryptedData, params.getAlgorithmType(), &plainData->front(),
        plainDataLength))) {
    if (error == NDN_ERROR_Unsupported_algorithm_type)
      throw runtime_error("RsaAlgorithm: Unsupported padding scheme");
    else
      throw SecurityException(string("RsaAlgorithm: ") + ndn_getErrorString(error));
  }
  
  plainData->resize(plainDataLength);
  return Blob(plainData, false);
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
