/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016 Regents of the University of California.
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
#if NDN_CPP_HAVE_LIBCRYPTO
#include <openssl/ssl.h>
#endif
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;
static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";

DecryptKey
RsaAlgorithm::generateKey(const RsaKeyParams& params)
{
  BIGNUM* exponent = 0;
  RSA* rsa = 0;
  bool success = false;
  Blob privateKeyDer;

  exponent = BN_new();
  if (BN_set_word(exponent, RSA_F4) == 1) {
    rsa = RSA_new();
    if (RSA_generate_key_ex(rsa, params.getKeySize(), exponent, NULL) == 1) {
      // Encode the private key.
      int length = i2d_RSAPrivateKey(rsa, NULL);
      vector<uint8_t> pkcs1PrivateKeyDer(length);
      uint8_t* derPointer = &pkcs1PrivateKeyDer[0];
      i2d_RSAPrivateKey(rsa, &derPointer);
      privateKeyDer = PrivateKeyStorage::encodePkcs8PrivateKey
        (pkcs1PrivateKeyDer, OID(RSA_ENCRYPTION_OID),
         ptr_lib::make_shared<DerNode::DerNull>());
      success = true;
    }
  }

  BN_free(exponent);
  RSA_free(rsa);
  if (!success)
    // We don't expect this to happen.
    throw SecurityException("RsaAlgorithm: Error generating RSA key pair");

  return DecryptKey(privateKeyDer);
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
  int padding;

  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_RsaPkcs)
    padding = RSA_PKCS1_PADDING;
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_RsaOaep)
    padding = RSA_PKCS1_OAEP_PADDING;
  else
    throw runtime_error("RsaAlgorithm: Unsupported padding scheme");

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

  // Use a temporary pointer since d2i updates it.
  const uint8_t* derPointer = rsaPrivateKeyDer.buf();
  rsa_st* privateKey = d2i_RSAPrivateKey(NULL, &derPointer, rsaPrivateKeyDer.size());
  if (!privateKey)
    throw UnrecognizedKeyFormatException
      ("RsaAlgorithm: Error decoding public key in d2i_RSAPublicKey");

  // TODO: use RSA_size, etc. to get the proper size of the output buffer.
  ptr_lib::shared_ptr<vector<uint8_t> > plainData(new vector<uint8_t>(1000));
  int outputLength = RSA_private_decrypt
    ((int)encryptedData.size(), (unsigned char *)encryptedData.buf(),
     (unsigned char*)&plainData->front(), privateKey, padding);
  // Free the private key before checking for success.
  RSA_free(privateKey);

  if (outputLength < 0)
    throw SecurityException("RsaAlgorithm: Error in RSA_private_decrypt");
  
  plainData->resize(outputLength);
  return Blob(plainData, false);
}

Blob
RsaAlgorithm::encrypt
  (const Blob& keyBits, const Blob& plainData, const EncryptParams& params)
{
  int padding;

  if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_RsaPkcs)
    padding = RSA_PKCS1_PADDING;
  else if (params.getAlgorithmType() == ndn_EncryptAlgorithmType_RsaOaep)
    padding = RSA_PKCS1_OAEP_PADDING;
  else
    throw runtime_error("RsaAlgorithm: Unsupported padding scheme");

  // Use a temporary pointer since d2i updates it.
  const uint8_t *derPointer = keyBits.buf();
  RSA *publicKey = d2i_RSA_PUBKEY(NULL, &derPointer, keyBits.size());
  if (!publicKey)
    throw UnrecognizedKeyFormatException
      ("RsaAlgorithm: Error decoding public key in d2i_RSAPublicKey");
  
  // TODO: use RSA_size, etc. to get the proper size of the output buffer.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedData(new vector<uint8_t>(1000));
  int outputLength = RSA_public_encrypt
    ((int)plainData.size(), (unsigned char *)plainData.buf(),
     (unsigned char*)&encryptedData->front(), publicKey, padding);
  // Free the public key before checking for success.
  RSA_free(publicKey);

  if (outputLength < 0)
    throw SecurityException("RsaAlgorithm: Error in RSA_public_encrypt");

  encryptedData->resize(outputLength);
  return Blob(encryptedData, false);
}
#endif

}