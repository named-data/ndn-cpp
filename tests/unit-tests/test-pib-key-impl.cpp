/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/detail/key-impl.t.cpp
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

#include "gtest/gtest.h"
#include <algorithm>
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "../../src/security/pib/detail/pib-key-impl.hpp"
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class TestPibKeyImpl : public ::testing::Test {
public:
  PibDataFixture fixture;
};

TEST_F(TestPibKeyImpl, Basic)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());
  PibKeyImpl key11
    (fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size(), pibImpl);

  ASSERT_EQ(fixture.id1Key1Name, key11.getName());
  ASSERT_EQ(fixture.id1, key11.getIdentityName());
  ASSERT_EQ(KEY_TYPE_RSA, key11.getKeyType());
  ASSERT_TRUE(key11.getPublicKey().equals(fixture.id1Key1));

  PibKeyImpl key11FromBackend(fixture.id1Key1Name, pibImpl);
  ASSERT_EQ(fixture.id1Key1Name, key11FromBackend.getName());
  ASSERT_EQ(fixture.id1, key11FromBackend.getIdentityName());
  ASSERT_EQ(KEY_TYPE_RSA, key11FromBackend.getKeyType());
  ASSERT_TRUE(key11FromBackend.getPublicKey().equals(fixture.id1Key1));
}

TEST_F(TestPibKeyImpl, CertificateOperation)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());
  PibKeyImpl key11
    (fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size(), pibImpl);
  ASSERT_NO_THROW(PibKeyImpl(fixture.id1Key1Name, pibImpl));

  // The key should not have any certificates.
  ASSERT_EQ(0, key11.certificates_.size());

  // Getting a non-existing certificate should throw Pib::Error.
  ASSERT_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()), Pib::Error);
  // Getting the non-existing default certificate should throw Pib::Error.
  ASSERT_THROW(key11.getDefaultCertificate(), Pib::Error);
  // Setting a non-existing certificate as the default should throw Pib::Error.
  ASSERT_THROW(key11.setDefaultCertificate(fixture.id1Key1Cert1->getName()), Pib::Error);

  // Add a certificate.
  key11.addCertificate(*fixture.id1Key1Cert1);
  ASSERT_NO_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()));

  // The new certificate becomes the default when there was no default.
  ASSERT_NO_THROW(key11.getDefaultCertificate());
  const ptr_lib::shared_ptr<CertificateV2>& defaultCert0 =
    key11.getDefaultCertificate();
  ASSERT_EQ(fixture.id1Key1Cert1->getName(), defaultCert0->getName());
  // Use the wire encoding to check equivalence.
  ASSERT_TRUE(fixture.id1Key1Cert1->wireEncode().equals
              (defaultCert0->wireEncode()));

  // Remove the certificate.
  key11.removeCertificate(fixture.id1Key1Cert1->getName());
  ASSERT_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()), Pib::Error);
  ASSERT_THROW(key11.getDefaultCertificate(), Pib::Error);

  // Set the default certificate directly.
  ASSERT_NO_THROW(key11.setDefaultCertificate(*fixture.id1Key1Cert1));
  ASSERT_NO_THROW(key11.getDefaultCertificate());
  ASSERT_NO_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()));

  // Check the default cert.
  const ptr_lib::shared_ptr<CertificateV2>& defaultCert1 =
    key11.getDefaultCertificate();
  ASSERT_EQ(fixture.id1Key1Cert1->getName(), defaultCert1->getName());
  ASSERT_TRUE(defaultCert1->wireEncode().equals(fixture.id1Key1Cert1->wireEncode()));

  // Add another certificate.
  key11.addCertificate(*fixture.id1Key1Cert2);
  ASSERT_EQ(2, key11.certificates_.size());

  // Set the default certificate using a name.
  ASSERT_NO_THROW(key11.setDefaultCertificate(fixture.id1Key1Cert2->getName()));
  ASSERT_NO_THROW(key11.getDefaultCertificate());
  const ptr_lib::shared_ptr<CertificateV2>& defaultCert2 =
    key11.getDefaultCertificate();
  ASSERT_EQ(fixture.id1Key1Cert2->getName(), defaultCert2->getName());
  ASSERT_TRUE(defaultCert2->wireEncode().equals(fixture.id1Key1Cert2->wireEncode()));

  // Remove a certificate.
  key11.removeCertificate(fixture.id1Key1Cert1->getName());
  ASSERT_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()), Pib::Error);
  ASSERT_EQ(1, key11.certificates_.size());

  // Set the default certificate directly again, which should change the default.
  ASSERT_NO_THROW(key11.setDefaultCertificate(*fixture.id1Key1Cert1));
  const ptr_lib::shared_ptr<CertificateV2>& defaultCert3 =
    key11.getDefaultCertificate();
  ASSERT_EQ(fixture.id1Key1Cert1->getName(), defaultCert3->getName());
  ASSERT_TRUE(defaultCert3->wireEncode().equals(fixture.id1Key1Cert1->wireEncode()));
  ASSERT_EQ(2, key11.certificates_.size());

  // Remove all certificates.
  key11.removeCertificate(fixture.id1Key1Cert1->getName());
  ASSERT_THROW(key11.getCertificate(fixture.id1Key1Cert1->getName()), Pib::Error);
  ASSERT_EQ(1, key11.certificates_.size());
  key11.removeCertificate(fixture.id1Key1Cert2->getName());
  ASSERT_THROW(key11.getCertificate(fixture.id1Key1Cert2->getName()), Pib::Error);
  ASSERT_THROW(key11.getDefaultCertificate(), Pib::Error);
  ASSERT_EQ(0, key11.certificates_.size());
}

TEST_F(TestPibKeyImpl, Overwrite)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  ASSERT_THROW(PibKeyImpl(fixture.id1Key1Name, pibImpl), Pib::Error);
  PibKeyImpl
    (fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size(), pibImpl);
  PibKeyImpl key1(fixture.id1Key1Name, pibImpl);

  // Overwriting the key should work.
  PibKeyImpl
    (fixture.id1Key1Name, fixture.id1Key2.buf(), fixture.id1Key2.size(), pibImpl);
  PibKeyImpl key2(fixture.id1Key1Name, pibImpl);

  // key1 should have cached the original public key.
  ASSERT_TRUE(!key1.getPublicKey().equals(key2.getPublicKey()));
  ASSERT_TRUE(key2.getPublicKey().equals(fixture.id1Key2));

  key1.addCertificate(*fixture.id1Key1Cert1);
  // Use the wire encoding to check equivalence.
  ASSERT_TRUE
    (key1.getCertificate(fixture.id1Key1Cert1->getName())->wireEncode().equals
     (fixture.id1Key1Cert1->wireEncode()));

  ptr_lib::shared_ptr<CertificateV2> otherCert
    (new CertificateV2(*fixture.id1Key1Cert1));
  dynamic_cast<Sha256WithRsaSignature*>(otherCert->getSignature())
    ->getValidityPeriod().setPeriod
      (ndn_getNowMilliseconds(), ndn_getNowMilliseconds() + 1000);
  // Don't bother resigning so we don't have to load a private key.

  ASSERT_EQ(fixture.id1Key1Cert1->getName(), otherCert->getName());
  ASSERT_TRUE(otherCert->getContent().equals(fixture.id1Key1Cert1->getContent()));
  ASSERT_FALSE(otherCert->wireEncode().equals(fixture.id1Key1Cert1->wireEncode()));

  key1.addCertificate(*otherCert);

  ASSERT_TRUE
    (key1.getCertificate(fixture.id1Key1Cert1->getName())->wireEncode().equals
     (otherCert->wireEncode()));
}

TEST_F(TestPibKeyImpl, Errors)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  ASSERT_THROW(PibKeyImpl(fixture.id1Key1Name, pibImpl), Pib::Error);
  PibKeyImpl key11
    (fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size(), pibImpl);

  ASSERT_THROW(PibKeyImpl(Name("/wrong"), pibImpl), invalid_argument);
  ASSERT_THROW(PibKeyImpl
    (Name("/wrong"), fixture.id1Key1.buf(), fixture.id1Key1.size(), pibImpl),
    invalid_argument);
  Blob wrongKey;
  ASSERT_THROW(PibKeyImpl
    (fixture.id1Key2Name, wrongKey.buf(), wrongKey.size(), pibImpl),
    invalid_argument);

  key11.addCertificate(*fixture.id1Key1Cert1);
  ASSERT_THROW(key11.addCertificate(*fixture.id1Key2Cert1), invalid_argument);
  ASSERT_THROW
    (key11.removeCertificate(fixture.id1Key2Cert1->getName()), invalid_argument);
  ASSERT_THROW
    (key11.getCertificate(fixture.id1Key2Cert1->getName()), invalid_argument);
  ASSERT_THROW
    (key11.setDefaultCertificate(*fixture.id1Key2Cert1), invalid_argument);
  ASSERT_THROW
    (key11.setDefaultCertificate(fixture.id1Key2Cert1->getName()),
     invalid_argument);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

