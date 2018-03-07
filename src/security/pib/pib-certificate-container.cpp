/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/certificate-container.cpp
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

#include <ndn-cpp/security/pib/pib-impl.hpp>
#include <ndn-cpp/security/pib/pib-certificate-container.hpp>

using namespace std;

namespace ndn {

PibCertificateContainer::PibCertificateContainer
  (const Name& keyName, const ptr_lib::shared_ptr<PibImpl>& pibImpl)
  // This copies the Name.
: keyName_(keyName), pibImpl_(pibImpl)
{
  if (!pibImpl)
    throw runtime_error("The pibImpl is null");

  certificateNames_ = pibImpl_->getCertificatesOfKey(keyName);
}

void
PibCertificateContainer::add(const CertificateV2& certificate)
{
  if (keyName_ != certificate.getKeyName())
    throw invalid_argument
      ("The certificate name `" + certificate.getKeyName().toUri() +
       "` does not match the key name");

  const Name& certificateName = certificate.getName();
  certificateNames_.insert(certificateName);
  // Copy the certificate.
  certificates_[certificateName] = ptr_lib::make_shared<CertificateV2>
    (certificate);
  pibImpl_->addCertificate(certificate);
}

void
PibCertificateContainer::remove(const Name& certificateName)
{
  if (!CertificateV2::isValidName(certificateName) ||
      CertificateV2::extractKeyNameFromCertName(certificateName) != keyName_)
    throw invalid_argument
      ("Certificate name `" + certificateName.toUri() +
        "` is invalid or does not match key name");

  certificateNames_.erase(certificateName);
  certificates_.erase(certificateName);
  pibImpl_->removeCertificate(certificateName);
}

ptr_lib::shared_ptr<CertificateV2>
PibCertificateContainer::get(const Name& certificateName)
{
  map<Name, ptr_lib::shared_ptr<CertificateV2>>::const_iterator it =
    certificates_.find(certificateName);

  if (it != certificates_.end())
    // Make a copy.
    // TODO: Copy is expensive. Can we just tell the caller not to modify it?
    return ptr_lib::make_shared<CertificateV2>(*it->second);

  // Get from the PIB and cache.
  if (!CertificateV2::isValidName(certificateName) ||
      CertificateV2::extractKeyNameFromCertName(certificateName) != keyName_)
    throw invalid_argument
      ("Certificate name `" + certificateName.toUri() +
       "` is invalid or does not match key name");

  ptr_lib::shared_ptr<CertificateV2> certificate =
    pibImpl_->getCertificate(certificateName);
  certificates_[certificateName] = certificate;
  // Make a copy.
  // TODO: Copy is expensive. Can we just tell the caller not to modify it?
  return ptr_lib::make_shared<CertificateV2>(*certificate);
}

bool
PibCertificateContainer::isConsistent() const
{
  return certificateNames_ == pibImpl_->getCertificatesOfKey(keyName_);
}

}
