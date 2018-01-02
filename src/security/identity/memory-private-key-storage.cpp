/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2018 Regents of the University of California.
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
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>

using namespace std;

namespace ndn {

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
  try {
    ptr_lib::shared_ptr<TpmPrivateKey> privateKey(new TpmPrivateKey);
    privateKey->loadPkcs1(privateKeyDer, privateKeyDerLength, keyType);
    privateKeyStore_[keyName.toUri()] = privateKey;
  } catch (TpmPrivateKey::Error& ex) {
    throw SecurityException(ex.what());
  }
}

void
MemoryPrivateKeyStorage::generateKeyPair
  (const Name& keyName, const KeyParams& params)
{
  if (doesKeyExist(keyName, KEY_CLASS_PUBLIC))
    throw SecurityException("Public Key already exists");
  if (doesKeyExist(keyName, KEY_CLASS_PRIVATE))
    throw SecurityException("Private Key already exists");

  try {
    ptr_lib::shared_ptr<TpmPrivateKey> privateKey =
      TpmPrivateKey::generatePrivateKey(params);
    privateKeyStore_[keyName.toUri()] = privateKey;
    publicKeyStore_[keyName.toUri()] = ptr_lib::shared_ptr<PublicKey>(new PublicKey
      (privateKey->derivePublicKey()));
  } catch (TpmPrivateKey::Error& ex) {
    throw SecurityException(ex.what());
  }
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
  map<string, ptr_lib::shared_ptr<TpmPrivateKey> >::iterator privateKey =
    privateKeyStore_.find(keyName.toUri());
  if (privateKey == privateKeyStore_.end())
    throw SecurityException
      (string("MemoryPrivateKeyStorage: Cannot find private key ") + keyName.toUri());

  try {
    return privateKey->second->sign(data, dataLength, digestAlgorithm);
  } catch (TpmPrivateKey::Error& ex) {
    throw SecurityException(ex.what());
  }
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

}
