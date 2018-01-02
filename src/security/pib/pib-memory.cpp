/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/pib-memory.cpp
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

#include <ndn-cpp/security/pib/pib-key.hpp>
#include <ndn-cpp/security/pib/pib-memory.hpp>

using namespace std;

namespace ndn {

PibMemory::~PibMemory() {}

string
PibMemory::getScheme() { return "pib-memory"; }

void
PibMemory::setTpmLocator(const string& tpmLocator) { tpmLocator_ = tpmLocator; }

string
PibMemory::getTpmLocator() const { return tpmLocator_; }

bool
PibMemory::hasIdentity(const Name& identityName) const
{
  return identityNames_.count(identityName) > 0;
}

void
PibMemory::addIdentity(const Name& identityName)
{
  // This copies the name.
  identityNames_.insert(identityName);

  if (!hasDefaultIdentity_) {
    defaultIdentityName_ = identityName;
    hasDefaultIdentity_ = true;
  }
}

void
PibMemory::removeIdentity(const Name& identityName)
{
  identityNames_.erase(identityName);
  if (identityName == defaultIdentityName_) {
    hasDefaultIdentity_ = false;
    defaultIdentityName_.clear();
  }

  set<Name> keyNames = getKeysOfIdentity(identityName);
  for (set<Name>::iterator it = keyNames.begin(); it != keyNames.end(); ++it)
    removeKey(*it);
}

void
PibMemory::clearIdentities()
{
  hasDefaultIdentity_ = false;
  defaultIdentityName_.clear();
  identityNames_.clear();
  defaultKeyNames_.clear();
  keys_.clear();
  defaultCertificateNames_.clear();
  certificates_.clear();
}

set<Name>
PibMemory::getIdentities() const
{
  // The set copy constructor will copy the Name objects.
  return identityNames_;
}

void
PibMemory::setDefaultIdentity(const Name& identityName)
{
  addIdentity(identityName);
  // This copies the name.
  defaultIdentityName_ = identityName;
  hasDefaultIdentity_ = true;
}

Name
PibMemory::getDefaultIdentity() const
{
  if (hasDefaultIdentity_)
    // This copies the name.
    return defaultIdentityName_;

  throw Pib::Error("No default identity");
}

bool
PibMemory::hasKey(const Name& keyName) const
{
  return keys_.count(keyName) > 0;
}

void
PibMemory::addKey
  (const Name& identityName, const Name& keyName, const uint8_t* key,
   size_t keyLength)
{
  addIdentity(identityName);

  keys_[keyName] = Blob(key, keyLength);

  if (defaultKeyNames_.count(identityName) == 0)
    // This copies the name.
    defaultKeyNames_[identityName] = keyName;
}

void
PibMemory::removeKey(const Name& keyName)
{
  Name identityName = PibKey::extractIdentityFromKeyName(keyName);

  keys_.erase(keyName);
  defaultKeyNames_.erase(identityName);

  set<Name> certificateNames = getCertificatesOfKey(keyName);
  for (set<Name>::iterator it = certificateNames.begin();
       it != certificateNames.end(); ++it)
    removeCertificate(*it);
}

Blob
PibMemory::getKeyBits(const Name& keyName) const
{
  if (!hasKey(keyName))
    throw Pib::Error("Key `" + keyName.toUri() + "` not found");

  map<Name, Blob>::const_iterator key = keys_.find(keyName);
  if (key == keys_.end())
    // We don't expect this since we just checked hasKey.
    throw runtime_error("keyName not found");
  return key->second;
}

set<Name>
PibMemory::getKeysOfIdentity(const Name& identityName) const
{
  set<Name> ids;
  for (map<Name, Blob>::const_iterator it = keys_.begin();
       it != keys_.end(); ++it) {
    if (identityName == PibKey::extractIdentityFromKeyName(it->first))
      // This copies the name.
      ids.insert(it->first);
  }

  return ids;
}

void
PibMemory::setDefaultKeyOfIdentity(const Name& identityName, const Name& keyName)
{
  if (!hasKey(keyName))
    throw Pib::Error("Key `" + keyName.toUri() + "` not found");

  // This copies the name.
  defaultKeyNames_[identityName] = keyName;
}

Name
PibMemory::getDefaultKeyOfIdentity(const Name& identityName) const
{
  map<Name, Name>::const_iterator defaultKey = defaultKeyNames_.find(identityName);
  if (defaultKey == defaultKeyNames_.end())
    throw Pib::Error("No default key for identity `" + identityName.toUri() + "`");

  // This copies the name.
  return defaultKey->second;
}

bool
PibMemory::hasCertificate(const Name& certificateName) const
{
  return certificates_.count(certificateName) > 0;
}

void
PibMemory::addCertificate(const CertificateV2& certificate)
{
  Name certificateName = certificate.getName();
  Name keyName = certificate.getKeyName();
  Name identity = certificate.getIdentity();

  addKey
    (identity, keyName, certificate.getContent().buf(),
     certificate.getContent().size());

  certificates_[certificateName] = ptr_lib::make_shared<CertificateV2>
    (certificate);
  if (defaultCertificateNames_.count(keyName) == 0)
    defaultCertificateNames_[keyName] = certificateName;
}

void
PibMemory::removeCertificate(const Name& certificateName)
{
  certificates_.erase(certificateName);
  map<Name, Name>::const_iterator defaultCertificate = defaultCertificateNames_.find
    (CertificateV2::extractKeyNameFromCertName(certificateName));
  if (defaultCertificate != defaultCertificateNames_.end() &&
      defaultCertificate->second == certificateName)
    defaultCertificateNames_.erase(defaultCertificate);
}

ptr_lib::shared_ptr<CertificateV2>
PibMemory::getCertificate(const Name& certificateName) const
{
  if (!hasCertificate(certificateName))
    throw Pib::Error("Certificate `" + certificateName.toUri() +  "` does not exist");

  map<Name, ptr_lib::shared_ptr<CertificateV2>>::const_iterator it =
    certificates_.find(certificateName);
  return ptr_lib::make_shared<CertificateV2>(*it->second);
}

std::set<Name>
PibMemory::getCertificatesOfKey(const Name& keyName) const
{
  set<Name> certificateNames;
  for (map<Name, ptr_lib::shared_ptr<CertificateV2>>::const_iterator it =
       certificates_.begin(); it != certificates_.end(); ++it) {
    if (CertificateV2::extractKeyNameFromCertName(it->second->getName()) == keyName)
      certificateNames.insert(it->first);
  }

  return certificateNames;
}

void
PibMemory::setDefaultCertificateOfKey
  (const Name& keyName, const Name& certificateName)
{
  if (!hasCertificate(certificateName))
    throw Pib::Error("Certificate `" + certificateName.toUri() +  "` does not exist");

  defaultCertificateNames_[keyName] = certificateName;
}

ptr_lib::shared_ptr<CertificateV2>
PibMemory::getDefaultCertificateOfKey(const Name& keyName) const
{
  map<Name, Name>::const_iterator it = defaultCertificateNames_.find(keyName);
  if (it == defaultCertificateNames_.end())
    throw Pib::Error("No default certificate for key `" + keyName.toUri() + "`");

  map<Name, ptr_lib::shared_ptr<CertificateV2>>::const_iterator certIt =
    certificates_.find(it->second);
  if (certIt == certificates_.end())
    // We don't expect this since we just checked hasKey.
    throw runtime_error("certificate not found");
  return ptr_lib::make_shared<CertificateV2>(*certIt->second);
}

}
