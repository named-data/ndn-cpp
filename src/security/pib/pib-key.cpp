/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/key.cpp
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

#include <ndn-cpp/security/v2/certificate-v2.hpp>
#include "detail/pib-key-impl.hpp"
#include <ndn-cpp/security/pib/pib-key.hpp>

using namespace std;

namespace ndn {

PibKey::PibKey(ptr_lib::weak_ptr<PibKeyImpl> impl)
: impl_(impl)
{
}

const Name&
PibKey::getName() const { return lock()->getName(); }

const Name&
PibKey::getIdentityName() const { return lock()->getIdentityName(); }

KeyType
PibKey::getKeyType() const { return lock()->getKeyType(); }

const Blob&
PibKey::getPublicKey() const { return lock()->getPublicKey(); }

ptr_lib::shared_ptr<CertificateV2>
PibKey::getCertificate(const Name& certificateName) const
{
  return lock()->getCertificate(certificateName);
}

const ptr_lib::shared_ptr<CertificateV2>&
PibKey::getDefaultCertificate() const { return lock()->getDefaultCertificate(); }

ptr_lib::shared_ptr<PibKeyImpl>
PibKey::lock() const
{
  // TODO: What is this needed for? Use something more general than weak_ptr?
  ptr_lib::shared_ptr<PibKeyImpl> impl = impl_.lock();

  if (!impl)
    throw domain_error("Invalid key instance");

  return impl;
}

Name
PibKey::constructKeyName(const Name& identityName, const Name::Component& keyId)
{
  Name keyName = identityName;
  keyName.append(CertificateV2::getKEY_COMPONENT()).append(keyId);

  return keyName;
}

bool
PibKey::isValidKeyName(const Name& keyName)
{
  return (keyName.size() > CertificateV2::MIN_KEY_NAME_LENGTH &&
          keyName.get(-CertificateV2::MIN_KEY_NAME_LENGTH) ==
            CertificateV2::getKEY_COMPONENT());
}

Name
PibKey::extractIdentityFromKeyName(const Name& keyName)
{
  if (!isValidKeyName(keyName))
    throw invalid_argument
      ("Key name `" + keyName.toUri() +
       "` does not follow the naming conventions");

  // Trim everything after and including "KEY".
  return keyName.getPrefix
    (-CertificateV2::MIN_KEY_NAME_LENGTH);
}

}
