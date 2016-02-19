/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2016 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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
#include "../../encoding/der/der-node.hpp"
#include <ndn-cpp/security/security-exception.hpp>
#if NDN_CPP_HAVE_LIBCRYPTO
#include <openssl/ssl.h>
#include <openssl/ec.h>
#endif
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>

using namespace std;

namespace ndn {

static const char *EC_ENCRYPTION_OID = "1.2.840.10045.2.1";

MemoryPrivateKeyStorage::~MemoryPrivateKeyStorage()
{
}

void
MemoryPrivateKeyStorage::setPublicKeyForKeyName
  (const Name& keyName, KeyType keyType, const uint8_t* publicKeyDer,
   size_t publicKeyDerLength)
{
  publicKeyStore_[keyName.toUri()] = ptr_lib::shared_ptr<PublicKey>(new PublicKey
    (Blob(publicKeyDer, publicKeyDerLength)));
}

void
MemoryPrivateKeyStorage::setPrivateKeyForKeyName
  (const Name& keyName, KeyType keyType, const uint8_t* privateKeyDer,
   size_t privateKeyDerLength)
{
  privateKeyStore_[keyName.toUri()] = ptr_lib::make_shared<PrivateKey>
    (keyType, privateKeyDer, privateKeyDerLength);
}

void
MemoryPrivateKeyStorage::generateKeyPair
  (const Name& keyName, const KeyParams& params)
{
  if (doesKeyExist(keyName, KEY_CLASS_PUBLIC))
    throw SecurityException("Public Key already exists");
  if (doesKeyExist(keyName, KEY_CLASS_PRIVATE))
    throw SecurityException("Private Key already exists");

  vector<uint8_t> publicKeyDer;
  vector<uint8_t> privateKeyDer;

#if NDN_CPP_HAVE_LIBCRYPTO
  if (params.getKeyType() == KEY_TYPE_RSA) {
    const RsaKeyParams& rsaParams = static_cast<const RsaKeyParams&>(params);
    BIGNUM* exponent = 0;
    RSA* rsa = 0;
    bool success = false;

    exponent = BN_new();
    if (BN_set_word(exponent, RSA_F4) == 1) {
      rsa = RSA_new();
      if (RSA_generate_key_ex(rsa, rsaParams.getKeySize(), exponent, NULL) == 1) {
        // Encode the public key.
        int length = i2d_RSA_PUBKEY(rsa, NULL);
        publicKeyDer.resize(length);
        uint8_t* derPointer = &publicKeyDer[0];
        i2d_RSA_PUBKEY(rsa, &derPointer);

        // Encode the private key.
        length = i2d_RSAPrivateKey(rsa, NULL);
        privateKeyDer.resize(length);
        derPointer = &privateKeyDer[0];
        i2d_RSAPrivateKey(rsa, &derPointer);
        success = true;
      }
    }

    BN_free(exponent);
    RSA_free(rsa);
    if (!success)
      throw SecurityException("FilePrivateKeyStorage: Error generating RSA key pair");
  }
  else if (params.getKeyType() == KEY_TYPE_ECDSA) {
    const EcdsaKeyParams& ecdsaParams = static_cast<const EcdsaKeyParams&>(params);

    OID parametersOid;
    int curveId = -1;

    // Find the entry in EC_KEY_INFO.
    for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
      const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
      if (info->keySize == ecdsaParams.getKeySize()) {
        curveId = info->curveId;
        parametersOid.setIntegerList
          (info->oidIntegerList, info->oidIntegerListLength);

        break;
      }
    }
    if (curveId == -1)
      throw SecurityException("Unsupported keySize for KEY_TYPE_ECDSA");

    bool success = false;
    EC_KEY* ecKey = EC_KEY_new_by_curve_name(curveId);
    if (ecKey != NULL) {
      if (EC_KEY_generate_key(ecKey) == 1) {
        // Encode the public key.
        int length = i2d_EC_PUBKEY(ecKey, NULL);
        vector<uint8_t> opensslPublicKeyDer(length);
        uint8_t* derPointer = &opensslPublicKeyDer[0];
        i2d_EC_PUBKEY(ecKey, &derPointer);
        // Convert the openssl style to ndn-cxx which has the simple AlgorithmIdentifier.
        // Find the bit string which is the second child.
        ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
          (&opensslPublicKeyDer[0], 0);
        const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
          parsedNode->getChildren();
        publicKeyDer = *encodeSubjectPublicKeyInfo
          (OID(EC_ENCRYPTION_OID),
           ptr_lib::make_shared<DerNode::DerOid>(parametersOid), children[1]);

        // Encode the private key.
        length = i2d_ECPrivateKey(ecKey, NULL);
        privateKeyDer.resize(length);
        derPointer = &privateKeyDer[0];
        i2d_ECPrivateKey(ecKey, &derPointer);
        cout << "debug privateKeyDer " << Blob(privateKeyDer).toHex() << endl;
        success = true;
      }
    }

    EC_KEY_free(ecKey);
    if (!success)
      throw SecurityException("FilePrivateKeyStorage: Error generating EC key pair");
  }
  else
#endif
    throw SecurityException("Unsupported key type");

  setKeyPairForKeyName
    (keyName, params.getKeyType(), &publicKeyDer[0], publicKeyDer.size(),
     &privateKeyDer[0], privateKeyDer.size());
}

void
MemoryPrivateKeyStorage::deleteKeyPair(const Name& keyName)
{
  string keyUri = keyName.toUri();

  publicKeyStore_.erase(keyUri);
  privateKeyStore_.erase(keyUri);
}

ptr_lib::shared_ptr<PublicKey>
MemoryPrivateKeyStorage::getPublicKey(const Name& keyName)
{
  map<string, ptr_lib::shared_ptr<PublicKey> >::iterator publicKey = publicKeyStore_.find(keyName.toUri());
  if (publicKey == publicKeyStore_.end())
    throw SecurityException(string("MemoryPrivateKeyStorage: Cannot find public key ") + keyName.toUri());
  return publicKey->second;
}

Blob
MemoryPrivateKeyStorage::sign(const uint8_t* data, size_t dataLength, const Name& keyName, DigestAlgorithm digestAlgorithm)
{
  if (digestAlgorithm != DIGEST_ALGORITHM_SHA256)
    throw SecurityException
      ("MemoryPrivateKeyStorage::sign: Unsupported digest algorithm");

  uint8_t digest[ndn_SHA256_DIGEST_SIZE];
  ndn_digestSha256(data, dataLength, digest);
  // TODO: use RSA_size to get the proper size of the signature buffer.
  uint8_t signatureBits[1000];
  unsigned int signatureBitsLength;

  // Find the private key and sign.
  map<string, ptr_lib::shared_ptr<PrivateKey> >::iterator privateKey = privateKeyStore_.find(keyName.toUri());
  if (privateKey == privateKeyStore_.end())
    throw SecurityException(string("MemoryPrivateKeyStorage: Cannot find private key ") + keyName.toUri());
#if NDN_CPP_HAVE_LIBCRYPTO
  if (privateKey->second->getKeyType() == KEY_TYPE_RSA) {
    if (!RSA_sign(NID_sha256, digest, sizeof(digest), signatureBits,
                  &signatureBitsLength, privateKey->second->getRsaPrivateKey()))
      throw SecurityException("Error in RSA_sign");
  }
  else if (privateKey->second->getKeyType() == KEY_TYPE_ECDSA) {
    if (!ECDSA_sign(NID_sha256, digest, sizeof(digest), signatureBits,
                  &signatureBitsLength, privateKey->second->getEcPrivateKey()))
      throw SecurityException("Error in RSA_sign");
  }
  else
    // We don't expect this to happen.
#endif
    throw SecurityException("Unrecognized private key type");

  return Blob(signatureBits, (size_t)signatureBitsLength);
}

Blob
MemoryPrivateKeyStorage::decrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric)
{
  throw runtime_error("MemoryPrivateKeyStorage::decrypt not implemented");
}

Blob
MemoryPrivateKeyStorage::encrypt(const Name& keyName, const uint8_t* data, size_t dataLength, bool isSymmetric)
{
  throw runtime_error("MemoryPrivateKeyStorage::encrypt not implemented");
}

void
MemoryPrivateKeyStorage::generateKey(const Name& keyName, const KeyParams& params)
{
  throw runtime_error("MemoryPrivateKeyStorage::generateKey not implemented");
}

bool
MemoryPrivateKeyStorage::doesKeyExist(const Name& keyName, KeyClass keyClass)
{
  if (keyClass == KEY_CLASS_PUBLIC)
    return publicKeyStore_.find(keyName.toUri()) != publicKeyStore_.end();
  else if (keyClass == KEY_CLASS_PRIVATE)
    return privateKeyStore_.find(keyName.toUri()) != privateKeyStore_.end();
  else
    // KEY_CLASS_SYMMETRIC not implemented yet.
    return false;
}

MemoryPrivateKeyStorage::PrivateKey::PrivateKey
  (KeyType keyType, const uint8_t* keyDer, size_t keyDerLength)
{
  keyType_ = keyType;
  rsaPrivateKey_ = 0;
  ecPrivateKey_ = 0;

  // Use a temporary pointer since d2i updates it.
  const uint8_t *derPointer = keyDer;
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType == KEY_TYPE_RSA) {
    rsaPrivateKey_ = d2i_RSAPrivateKey(NULL, &derPointer, keyDerLength);
    if (!rsaPrivateKey_)
      throw SecurityException("PrivateKey constructor: Error decoding RSA private key DER");
  }
  else if (keyType == KEY_TYPE_ECDSA) {
    ecPrivateKey_ = d2i_ECPrivateKey(NULL, &derPointer, keyDerLength);
    if (!ecPrivateKey_)
      throw SecurityException("PrivateKey constructor: Error decoding EC private key DER");
  }
  else
#endif
    throw SecurityException("PrivateKey constructor: Unrecognized keyType");
}

MemoryPrivateKeyStorage::PrivateKey::~PrivateKey()
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (rsaPrivateKey_)
    RSA_free(rsaPrivateKey_);
  if (ecPrivateKey_)
    EC_KEY_free(ecPrivateKey_);
#endif
}

}
