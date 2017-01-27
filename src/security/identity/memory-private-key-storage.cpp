/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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
    RsaPrivateKeyLite privateKey;
    ndn_Error error;
    if ((error = privateKey.generate(rsaParams.getKeySize())))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));

    // Get the encoding length and encode the public key.
    size_t encodingLength;
    if ((error = privateKey.encodePublicKey(0, encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
    publicKeyDer.resize(encodingLength);
    if ((error = privateKey.encodePublicKey(&publicKeyDer[0], encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));

    // Get the encoding length and encode the private key.
    if ((error = privateKey.encodePrivateKey(0, encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
    privateKeyDer.resize(encodingLength);
    if ((error = privateKey.encodePrivateKey(&privateKeyDer[0], encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
  }
  else if (params.getKeyType() == KEY_TYPE_ECDSA) {
    const EcdsaKeyParams& ecdsaParams = static_cast<const EcdsaKeyParams&>(params);

    EcPrivateKeyLite privateKey;
    ndn_Error error;
    if ((error = privateKey.generate(ecdsaParams.getKeySize())))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));

    // Find the entry in EC_KEY_INFO.
    OID parametersOid;
    for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
      const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
      if (info->keySize == ecdsaParams.getKeySize()) {
        parametersOid.setIntegerList
          (info->oidIntegerList, info->oidIntegerListLength);

        break;
      }
    }
    if (parametersOid.getIntegerList().size() == 0)
      // We don't expect this to happen since generate succeeded.
      throw SecurityException("Unsupported keySize for KEY_TYPE_ECDSA");

    // Get the encoding length and encode the public key.
    size_t encodingLength;
    if ((error = privateKey.encodePublicKey(true, 0, encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
    vector<uint8_t> opensslPublicKeyDer(encodingLength);
    if ((error = privateKey.encodePublicKey
         (true, &opensslPublicKeyDer[0], encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
    // Convert the openssl style to ndn-cxx which has the simple AlgorithmIdentifier.
    // Find the bit string which is the second child.
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (&opensslPublicKeyDer[0], 0);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
      parsedNode->getChildren();
    publicKeyDer = *encodeSubjectPublicKeyInfo
      (OID(EC_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerOid>(parametersOid), children[1]);

    // Get the encoding length and encode the private key.
    if ((error = privateKey.encodePrivateKey(true, 0, encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
    privateKeyDer.resize(encodingLength);
    if ((error = privateKey.encodePrivateKey
         (true, &privateKeyDer[0], encodingLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage: ") + ndn_getErrorString(error));
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

  // TODO: use RSA_size to get the proper size of the signature buffer.
  uint8_t signatureBits[1000];
  size_t signatureBitsLength;
  ndn_Error error;

  // Find the private key and sign.
  map<string, ptr_lib::shared_ptr<PrivateKey> >::iterator privateKey = privateKeyStore_.find(keyName.toUri());
  if (privateKey == privateKeyStore_.end())
    throw SecurityException(string("MemoryPrivateKeyStorage: Cannot find private key ") + keyName.toUri());
#if NDN_CPP_HAVE_LIBCRYPTO
  if (privateKey->second->getKeyType() == KEY_TYPE_RSA) {
    if ((error =  privateKey->second->getRsaPrivateKey().signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage::sign: ") + ndn_getErrorString(error));
  }
  else if (privateKey->second->getKeyType() == KEY_TYPE_ECDSA) {
    if ((error =  privateKey->second->getEcPrivateKey().signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw SecurityException
        (string("MemoryPrivateKeyStorage::sign: ") + ndn_getErrorString(error));
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
  ndn_Error error;
  keyType_ = keyType;

#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType == KEY_TYPE_RSA) {
    if ((error = rsaPrivateKey_.decode(keyDer, keyDerLength)))
      throw SecurityException
        (string("PrivateKey constructor: ") + ndn_getErrorString(error));
  }
  else if (keyType == KEY_TYPE_ECDSA) {
    if ((error = ecPrivateKey_.decode(keyDer, keyDerLength)))
      throw SecurityException
        (string("PrivateKey constructor: ") + ndn_getErrorString(error));
  }
  else
#endif
    throw SecurityException("PrivateKey constructor: Unrecognized keyType");
}

}
