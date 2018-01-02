/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/v2/key-chain.t.cpp
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
#include <unistd.h>
#include <fstream>
#include <cstdio>
#include "identity-management-fixture.hpp"
#include <ndn-cpp/security/key-chain.hpp>

using namespace std;
using namespace ndn;

class TestKeyChain : public ::testing::Test {
public:
  IdentityManagementFixture fixture_;
};

TEST_F(TestKeyChain, Management)
{
  Name identityName("/test/id");
  Name identity2Name("/test/id2");

  ASSERT_EQ(0, fixture_.keyChain_.getPib().identities_.size());
  ASSERT_THROW(fixture_.keyChain_.getPib().getDefaultIdentity(), Pib::Error);

  // Create an identity.
  ptr_lib::shared_ptr<PibIdentity> id = fixture_.keyChain_.createIdentityV2(identityName);
  ASSERT_TRUE(!!id);
  ASSERT_TRUE(fixture_.keyChain_.getPib().identities_.identities_.find(identityName) !=
              fixture_.keyChain_.getPib().identities_.identities_.end());
  // The first added identity becomes the default identity.
  ASSERT_NO_THROW(fixture_.keyChain_.getPib().getDefaultIdentity());
  // The default key of the added identity must exist.
  ptr_lib::shared_ptr<PibKey> key;
  ASSERT_NO_THROW(key = id->getDefaultKey());
  // The default certificate of the default key must exist.
  ASSERT_NO_THROW(key->getDefaultCertificate());

  // Delete the key.
  Name key1Name = key->getName();
  ASSERT_NO_THROW(id->getKey(key1Name));
  ASSERT_EQ(1, id->getKeys().size());
  fixture_.keyChain_.deleteKey(*id, *key);
/* TODO: Implement key validity.
  // The key instance should not be valid anymore.
  ASSERT_TRUE(!key);
*/
  ASSERT_THROW(id->getKey(key1Name), Pib::Error);
  ASSERT_EQ(0, id->getKeys().size());

  // Create another key.
  fixture_.keyChain_.createKey(*id);
  // The added key becomes the default key.
  ASSERT_NO_THROW(id->getDefaultKey());
  ptr_lib::shared_ptr<PibKey> key2 = id->getDefaultKey();
  ASSERT_TRUE(!!key2);
  ASSERT_TRUE(!key2->getName().equals(key1Name));
  ASSERT_EQ(1, id->getKeys().size());
  ASSERT_NO_THROW(key2->getDefaultCertificate());

  // Create a third key.
  ptr_lib::shared_ptr<PibKey> key3 = fixture_.keyChain_.createKey(*id);
  ASSERT_TRUE(!key3->getName().equals(key2->getName()));
  // The added key will not be the default key, because the default key already exists.
  ASSERT_TRUE(id->getDefaultKey()->getName().equals(key2->getName()));
  ASSERT_EQ(2, id->getKeys().size());
  ASSERT_NO_THROW(key3->getDefaultCertificate());

  // Delete the certificate.
  ASSERT_EQ(1, key3->getCertificates().size());
  ptr_lib::shared_ptr<CertificateV2> key3Cert1 = 
    key3->getCertificates().certificates_.begin()->second;
  Name key3CertName = key3Cert1->getName();
  fixture_.keyChain_.deleteCertificate(*key3, key3CertName);
  ASSERT_EQ(0, key3->getCertificates().size());
  ASSERT_THROW(key3->getDefaultCertificate(), Pib::Error);

  // Add a certificate.
  fixture_.keyChain_.addCertificate(*key3, *key3Cert1);
  ASSERT_EQ(1, key3->getCertificates().size());
  ASSERT_NO_THROW(key3->getDefaultCertificate());
  // Overwriting the certificate should work.
  fixture_.keyChain_.addCertificate(*key3, *key3Cert1);
  ASSERT_EQ(1, key3->getCertificates().size());
  // Add another certificate.
  ptr_lib::shared_ptr<CertificateV2> key3Cert2(new CertificateV2(*key3Cert1));
  Name key3Cert2Name = key3->getName();
  key3Cert2Name.append("Self");
  key3Cert2Name.appendVersion(1);
  key3Cert2->setName(key3Cert2Name);
  fixture_.keyChain_.addCertificate(*key3, *key3Cert2);
  ASSERT_EQ(2, key3->getCertificates().size());

  // Set the default certificate.
  ASSERT_TRUE(key3->getDefaultCertificate()->getName().equals(key3CertName));
  fixture_.keyChain_.setDefaultCertificate(*key3, *key3Cert2);
  ASSERT_TRUE(key3->getDefaultCertificate()->getName().equals(key3Cert2Name));

  // Set the default key.
  ASSERT_TRUE(id->getDefaultKey()->getName().equals(key2->getName()));
  fixture_.keyChain_.setDefaultKey(*id, *key3);
  ASSERT_TRUE(id->getDefaultKey()->getName().equals(key3->getName()));

  // Set the default identity.
  ptr_lib::shared_ptr<PibIdentity> id2 = fixture_.keyChain_.createIdentityV2
    (identity2Name);
  ASSERT_TRUE(fixture_.keyChain_.getPib().getDefaultIdentity()->getName().equals
              (id->getName()));
  fixture_.keyChain_.setDefaultIdentity(*id2);
  ASSERT_TRUE(fixture_.keyChain_.getPib().getDefaultIdentity()->getName().equals
              (id2->getName()));

  // Delete an identity.
  fixture_.keyChain_.deleteIdentity(*id);
/* TODO: Implement identity validity.
  // The identity instance should not be valid any more.
  BOOST_CHECK(!id);
*/
  ASSERT_THROW(fixture_.keyChain_.getPib().getIdentity(identityName), Pib::Error);
  ASSERT_TRUE(fixture_.keyChain_.getPib().identities_.identities_.find(identityName) ==
              fixture_.keyChain_.getPib().identities_.identities_.end());
}

TEST_F(TestKeyChain, SelfSignedCertValidity)
{
  ptr_lib::shared_ptr<CertificateV2> certificate = fixture_.addIdentity
    ("/Security/V2/TestKeyChain/SelfSignedCertValidity")->getDefaultKey
     ()->getDefaultCertificate();
  ASSERT_TRUE(certificate->isValid());
  // Check 10 years from now.
  ASSERT_TRUE(certificate->isValid
    (ndn_getNowMilliseconds() + 10 * 365 * 24 * 3600 * 1000.0));
  // Check that notAfter is later than 10 years from now.
  ASSERT_TRUE(certificate->getValidityPeriod().getNotAfter() >
    ndn_getNowMilliseconds() + 10 * 365 * 24 * 3600 * 1000.0);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

