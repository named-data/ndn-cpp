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
  return identityStore_.find(identityName.toUri()) != identityStore_.end();
}

void
MemoryIdentityStorage::addIdentity(const Name& identityName)
{
  string identityUri = identityName.toUri();
  if (identityStore_.find(identityUri) != identityStore_.end())
    return;

  identityStore_[identityUri] = IdentityRecord();
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
  if (keyName.size() == 0)
    return;

  if (doesKeyExist(keyName))
    return;

  Name identityName = keyName.getSubName(0, keyName.size() - 1);

  addIdentity(identityName);

  keyStore_[keyName.toUri()] = ptr_lib::make_shared<KeyRecord>(keyType, publicKeyDer);
}

Blob
MemoryIdentityStorage::getKey(const Name& keyName)
{
  if (keyName.size() == 0)
    throw SecurityException("MemoryIdentityStorage::getKey: Empty keyName");

  map<string, ptr_lib::shared_ptr<KeyRecord> >::iterator record = keyStore_.find(keyName.toUri());
  if (record == keyStore_.end())
    throw SecurityException("MemoryIdentityStorage::getKey: The key does not exist");

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

  addKey(keyName, certificate.getPublicKeyInfo().getKeyType(),
         certificate.getPublicKeyInfo().getKeyDer());

  if (doesCertificateExist(certificateName))
    return;

  // Insert the certificate.
  // wireEncode returns the cached encoding if available.
  certificateStore_[certificateName.toUri()] = certificate.wireEncode();
}

ptr_lib::shared_ptr<IdentityCertificate>
MemoryIdentityStorage::getCertificate(const Name& certificateName)
{
  map<string, Blob>::iterator record = certificateStore_.find(certificateName.toUri());
  if (record == certificateStore_.end())
    throw SecurityException
      ("MemoryIdentityStorage::getCertificate: The certificate does not exist");

  ptr_lib::shared_ptr<IdentityCertificate> certificate(new IdentityCertificate());
  try {
    certificate->wireDecode(*record->second);
  } catch (...) {
    throw SecurityException
      ("MemoryIdentityStorage::getCertificate: The certificate cannot be decoded");
  }
  return certificate;
}

string
MemoryIdentityStorage::getTpmLocator() { return "tpm-memory:"; }

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
  string identityUri = identityName.toUri();
  map<string, IdentityRecord>::iterator record = identityStore_.find(identityUri);
  if (record != identityStore_.end()) {
    if (record->second.hasDefaultKey())
      return record->second.getDefaultKey();
    else
      throw SecurityException("No default key set.");
  }
  else
    throw SecurityException("Identity not found.");
}

Name
MemoryIdentityStorage::getDefaultCertificateNameForKey(const Name& keyName)
{
  string keyUri = keyName.toUri();
  map<string, ptr_lib::shared_ptr<KeyRecord> >::iterator record =
    keyStore_.find(keyUri);
  if (record != keyStore_.end()) {
    if (record->second->hasDefaultCertificate())
      return record->second->getDefaultCertificate();
    else
      throw SecurityException("No default certificate set.");
  }
  else
    throw SecurityException("Key not found.");
}

void
MemoryIdentityStorage::getAllIdentities(vector<Name>& nameList, bool isDefault)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getAllIdentities not implemented");
#endif
}

void
MemoryIdentityStorage::getAllKeyNamesOfIdentity
  (const Name& identityName, vector<Name>& nameList, bool isDefault)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getAllKeyNamesOfIdentity not implemented");
#endif
}

void
MemoryIdentityStorage::getAllCertificateNamesOfKey
  (const Name& keyName, vector<Name>& nameList, bool isDefault)
{
#if 1
  throw runtime_error("MemoryIdentityStorage::getAllCertificateNamesOfKey not implemented");
#endif
}

void
MemoryIdentityStorage::setDefaultIdentity(const Name& identityName)
{
  string identityUri = identityName.toUri();
  if (identityStore_.find(identityUri) != identityStore_.end())
    defaultIdentity_ = identityUri;
  else
    // The identity doesn't exist, so clear the default.
    defaultIdentity_.clear();
}

void
MemoryIdentityStorage::setDefaultKeyNameForIdentity
  (const Name& keyName, const Name& identityNameCheck)
{
  Name identityName = keyName.getPrefix(-1);

  if (identityNameCheck.size() > 0 && identityNameCheck != identityName)
    throw SecurityException
      ("The specified identity name does not match the key name");

  string identityUri = identityName.toUri();
  map<string, IdentityRecord>::iterator record = identityStore_.find(identityUri);
  if (record != identityStore_.end())
    record->second.setDefaultKey(ptr_lib::make_shared<Name>(keyName));
}

void
MemoryIdentityStorage::setDefaultCertificateNameForKey(const Name& keyName, const Name& certificateName)
{
  string keyUri = keyName.toUri();
  map<string, ptr_lib::shared_ptr<KeyRecord> >::iterator record =
    keyStore_.find(keyUri);
  if (record != keyStore_.end())
    record->second->setDefaultCertificate
      (ptr_lib::make_shared<Name>(certificateName));
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
