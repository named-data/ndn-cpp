/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/identity-management-fixture.cpp
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

#include <fstream>
#include "../../src/encoding/base64.hpp"
#include "identity-management-fixture.hpp"

using namespace std;
using namespace ndn;

IdentityManagementFixture::~IdentityManagementFixture()
{
  // Remove created certificate files.
  for (set<string>::iterator it = certificateFiles_.begin();
       it != certificateFiles_.end(); ++it) {
    remove(it->c_str());
  }
}

bool
IdentityManagementFixture::saveCertificateToFile
  (const Data& data, const string& filePath)
{
  certificateFiles_.insert(filePath);

  try {
    Blob encoding = data.wireEncode();
    string encodedCertificate = toBase64(encoding.buf(), encoding.size(), true);
    ofstream certificateFile(filePath.c_str());
    certificateFile << encodedCertificate;
    return true;
  }
  catch (const exception&) {
    return false;
  }
}

ptr_lib::shared_ptr<PibIdentity>
IdentityManagementFixture::addIdentity
  (const Name& identityName, const KeyParams& params)
{
  ptr_lib::shared_ptr<PibIdentity> identity = keyChain_.createIdentityV2
    (identityName, params);
  identityNames_.insert(identityName);
  return identity;
}

bool
IdentityManagementFixture::saveCertificate
  (PibIdentity identity, const string& filePath)
{
  try {
    const ptr_lib::shared_ptr<CertificateV2>& certificate =
      identity.getDefaultKey()->getDefaultCertificate();
    return saveCertificateToFile(*certificate, filePath);
  }
  catch (const Pib::Error&) {
    return false;
  }
}

ptr_lib::shared_ptr<PibIdentity>
IdentityManagementFixture::addSubCertificate
  (const Name& subIdentityName,
   const ndn::ptr_lib::shared_ptr<ndn::PibIdentity>& issuer,
   const KeyParams& params)
{
  ptr_lib::shared_ptr<PibIdentity> subIdentity =
    addIdentity(subIdentityName, params);

  ptr_lib::shared_ptr<CertificateV2> request =
    subIdentity->getDefaultKey()->getDefaultCertificate();

  request->setName(request->getKeyName().append("parent").appendVersion(1));

  SigningInfo certificateParams(issuer);
  // Validity period of 20 years.
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  certificateParams.setValidityPeriod
    (ValidityPeriod(now, now + 20 * 365 * 24 * 3600 * 1000.0));

  // Skip the AdditionalDescription.

  keyChain_.sign(*request, certificateParams);
  keyChain_.setDefaultCertificate(*subIdentity->getDefaultKey(), *request);

  return subIdentity;
}

ptr_lib::shared_ptr<CertificateV2>
IdentityManagementFixture::addCertificate
  (ptr_lib::shared_ptr<PibKey>& key, const string& issuerId)
{
  Name certificateName = key->getName();
  certificateName.append(issuerId).appendVersion(3);
  ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());
  certificate->setName(certificateName);

  // Set the MetaInfo.
  certificate->getMetaInfo().setType(ndn_ContentType_KEY);
  // One hour.
  certificate->getMetaInfo().setFreshnessPeriod(3600 * 1000.);

  // Set the content.
  certificate->setContent(key->getPublicKey());

  SigningInfo params(key);
  // Validity period of 10 days.
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  params.setValidityPeriod(ValidityPeriod(now, now + 10 * 24 * 3600 * 1000.0));

  keyChain_.sign(*certificate, params);
  return certificate;
}
