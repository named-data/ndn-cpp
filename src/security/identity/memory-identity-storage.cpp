/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#if 1
#include <stdexcept>
#endif
#include <algorithm>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/certificate/identity-certificate.hpp>
#include <ndn-cpp/security/identity/memory-identity-storage.hpp>

using namespace std;

namespace ndn {

MemoryIdentityStorage::~MemoryIdentityStorage()
{
}

bool
MemoryIdentityStorage::doesIdentityExist(const Name& identityName)
{
  string identityUri = identityName.toUri();
  return find(identityStore_.begin(), identityStore_.end(), identityUri) != identityStore_.end();
}

void
MemoryIdentityStorage::addIdentity(const Name& identityName)
{
  string identityUri = identityName.toUri();
  if (find(identityStore_.begin(), identityStore_.end(), identityUri) != identityStore_.end())
    return;

  identityStore_.push_back(identityUri);
}

bool
MemoryIdentityStorage::revokeIdentity()
{
#if 1
  throw runtime_error("MemoryIdentityStorage::revokeIdentity not implemented");
#endif
}

bool
MemoryIdentityStorage::doesKeyExist(const Name& keyName)
{
  return keyStore_.find(keyName.toUri()) != keyStore_.end();
}

void
MemoryIdentityStorage::addKey(const Name& keyName, KeyType keyType, const Blob& publicKeyDer)
{
  Name identityName = keyName.getSubName(0, keyName.size() - 1);

  addIdentity(identityName);

  if (doesKeyExist(keyName))
    throw SecurityException("a key with the same name already exists!");

  keyStore_[keyName.toUri()] = ptr_lib::make_shared<KeyRecord>(keyType, publicKeyDer);
}

Blob
MemoryIdentityStorage::getKey(const Name& keyName)
{
  map<string, ptr_lib::shared_ptr<KeyRecord> >::iterator record = keyStore_.find(keyName.toUri());
  if (record == keyStore_.end())
    // Not found.  Silently return null.
    return Blob();

  return record->second->getKeyDer();
}

void
MemoryIdentityStorage::activateKey(const Name& keyName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::activateKey not implemented");
#endif
}

void
MemoryIdentityStorage::deactivateKey(const Name& keyName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::deactivateKey not implemented");
#endif
}

bool
MemoryIdentityStorage::doesCertificateExist(const Name& certificateName)
{
  return certificateStore_.find(certificateName.toUri()) != certificateStore_.end();
}

void
MemoryIdentityStorage::addCertificate(const IdentityCertificate& certificate)
{
  const Name& certificateName = certificate.getName();
  Name keyName = certificate.getPublicKeyName();

  if (!doesKeyExist(keyName))
    throw SecurityException("No corresponding Key record for certificate! " + keyName.toUri() + " " + certificateName.toUri());

  // Check if certificate already exists.
  if (doesCertificateExist(certificateName))
    throw SecurityException("Certificate has already been installed!");

  // Check if the public key of certificate is the same as the key record.
  Blob keyBlob = getKey(keyName);
  if (!keyBlob || (*keyBlob) != *(certificate.getPublicKeyInfo().getKeyDer()))
    throw SecurityException("Certificate does not match the public key!");

  // Insert the certificate.
  // wireEncode returns the cached encoding if available.
  certificateStore_[certificateName.toUri()] = certificate.wireEncode();
}

ptr_lib::shared_ptr<IdentityCertificate>
MemoryIdentityStorage::getCertificate(const Name& certificateName, bool allowAny)
{
  map<string, Blob>::iterator record = certificateStore_.find(certificateName.toUri());
  if (record == certificateStore_.end())
    // Not found.  Silently return null.
    return ptr_lib::shared_ptr<IdentityCertificate>();

  ptr_lib::shared_ptr<IdentityCertificate> data(new IdentityCertificate());
  data->wireDecode(*record->second);
  return data;
}

Name
MemoryIdentityStorage::getDefaultIdentity()
{
  if (defaultIdentity_.size() == 0)
    throw SecurityException("MemoryIdentityStorage::getDefaultIdentity: The default identity is not defined");

  return Name(defaultIdentity_);
}

Name
MemoryIdentityStorage::getDefaultKeyNameForIdentity(const Name& identityName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getDefaultKeyNameForIdentity not implemented");
#endif
}

Name
MemoryIdentityStorage::getDefaultCertificateNameForKey(const Name& keyName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getDefaultCertificateNameForKey not implemented");
#endif
}

void
MemoryIdentityStorage::getAllKeyNamesOfIdentity
  (const Name& identityName, std::vector<Name>& nameList, bool isDefault)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getAllKeyNamesOfIdentity not implemented");
#endif
}

void
MemoryIdentityStorage::setDefaultIdentity(const Name& identityName)
{
  string identityUri = identityName.toUri();
  if (find(identityStore_.begin(), identityStore_.end(), identityUri) != identityStore_.end())
    defaultIdentity_ = identityUri;
  else
    // The identity doesn't exist, so clear the default.
    defaultIdentity_.clear();
}

void
MemoryIdentityStorage::setDefaultKeyNameForIdentity(const Name& keyName, const Name& identityNameCheck)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::setDefaultKeyNameForIdentity not implemented");
#endif
}

void
MemoryIdentityStorage::setDefaultCertificateNameForKey(const Name& keyName, const Name& certificateName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::setDefaultCertificateNameForKey not implemented");
#endif
}

void
MemoryIdentityStorage::deleteCertificateInfo(const Name& certificateName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::deleteCertificateInfo is not implemented");
#endif
}

void
MemoryIdentityStorage::deletePublicKeyInfo(const Name& keyName)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::deletePublicKeyInfo is not implemented");
#endif
}

void
MemoryIdentityStorage::deleteIdentityInfo(const Name& identity)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::deleteIdentityInfo is not implemented");
#endif
}

}
