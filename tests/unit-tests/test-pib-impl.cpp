/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/pib-impl.t.cpp
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
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include <ndn-cpp/security/pib/pib-sqlite3.hpp>
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class PibMemoryFixture : public PibDataFixture
{
public:
  PibMemoryFixture()
  {
    pib = &myPib_;
  }

private:
  PibMemory myPib_;
};

static bool
fileExists(const string& filePath)
{
  ifstream stream(filePath.c_str());
  bool result = (bool)stream;
  stream.close();
  return result;
}

static string
getPolicyConfigDirectory()
{
  string policyConfigDirectory = "policy_config";
  // Check if expected files are in this directory.
  if (!fileExists(policyConfigDirectory + "/regex_ruleset.conf")) {
    // Maybe we are running "make check" from the ndn-cpp root.  There may be
    //   a way to tell "make check" to run from tests/unit-tests, but for
    //   now just set policyConfigDirectory explicitly.
    policyConfigDirectory = "tests/unit-tests/policy_config";

    if(!fileExists(policyConfigDirectory + "/regex_ruleset.conf"))
      throw runtime_error("Cannot find the directory for policy-config");
  }

  return policyConfigDirectory;
}

class PibSqlite3Fixture : public PibDataFixture
{
public:
  PibSqlite3Fixture()
  {
    string databaseDirectoryPath = getPolicyConfigDirectory();
    string databaseFilename = "test-pib.db";
    databaseFilePath = databaseDirectoryPath + "/" + databaseFilename;
    remove(databaseFilePath.c_str());

    myPib_.reset(new PibSqlite3(databaseDirectoryPath, databaseFilename));

    pib = myPib_.get();
  }

  string databaseFilePath;
  
private:
  ptr_lib::shared_ptr<PibSqlite3> myPib_;
};

class TestPibImpl : public ::testing::Test {
public:
  TestPibImpl()
  {
    pibImpls[0] = &pibMemoryFixture;
    pibImpls[1] = &pibSqlite3Fixture;
  }

  PibMemoryFixture pibMemoryFixture;
  PibSqlite3Fixture pibSqlite3Fixture;

  PibDataFixture* pibImpls[2];

  virtual void
  TearDown()
  {
    remove(pibSqlite3Fixture.databaseFilePath.c_str());
  }
};

TEST_F(TestPibImpl, CertificateDecoding)
{
  // Use pibMemoryFixture to test.
  PibDataFixture& fixture = pibMemoryFixture;

  ASSERT_TRUE(fixture.id1Key1Cert1->getPublicKey().equals
    (fixture.id1Key1Cert2->getPublicKey()));
  ASSERT_TRUE(fixture.id1Key2Cert1->getPublicKey().equals
    (fixture.id1Key2Cert2->getPublicKey()));
  ASSERT_TRUE(fixture.id2Key1Cert1->getPublicKey().equals
    (fixture.id2Key1Cert2->getPublicKey()));
  ASSERT_TRUE(fixture.id2Key2Cert1->getPublicKey().equals
    (fixture.id2Key2Cert2->getPublicKey()));

  ASSERT_TRUE(fixture.id1Key1Cert1->getPublicKey().equals(fixture.id1Key1));
  ASSERT_TRUE(fixture.id1Key1Cert2->getPublicKey().equals(fixture.id1Key1));
  ASSERT_TRUE(fixture.id1Key2Cert1->getPublicKey().equals(fixture.id1Key2));
  ASSERT_TRUE(fixture.id1Key2Cert2->getPublicKey().equals(fixture.id1Key2));

  ASSERT_TRUE(fixture.id2Key1Cert1->getPublicKey().equals(fixture.id2Key1));
  ASSERT_TRUE(fixture.id2Key1Cert2->getPublicKey().equals(fixture.id2Key1));
  ASSERT_TRUE(fixture.id2Key2Cert1->getPublicKey().equals(fixture.id2Key2));
  ASSERT_TRUE(fixture.id2Key2Cert2->getPublicKey().equals(fixture.id2Key2));

  ASSERT_TRUE(fixture.id1Key1Cert2->getIdentity().equals(fixture.id1));
  ASSERT_TRUE(fixture.id1Key2Cert1->getIdentity().equals(fixture.id1));
  ASSERT_TRUE(fixture.id1Key2Cert2->getIdentity().equals(fixture.id1));

  ASSERT_TRUE(fixture.id2Key1Cert2->getIdentity().equals(fixture.id2));
  ASSERT_TRUE(fixture.id2Key2Cert1->getIdentity().equals(fixture.id2));
  ASSERT_TRUE(fixture.id2Key2Cert2->getIdentity().equals(fixture.id2));

  ASSERT_TRUE(fixture.id1Key1Cert2->getKeyName().equals(fixture.id1Key1Name));
  ASSERT_TRUE(fixture.id1Key2Cert2->getKeyName().equals(fixture.id1Key2Name));

  ASSERT_TRUE(fixture.id2Key1Cert2->getKeyName().equals(fixture.id2Key1Name));
  ASSERT_TRUE(fixture.id2Key2Cert2->getKeyName().equals(fixture.id2Key2Name));
}

TEST_F(TestPibImpl, TpmLocator)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    // Basic getting and setting
    ASSERT_NO_THROW(pib.getTpmLocator());

    ASSERT_NO_THROW(pib.setTpmLocator("tpmLocator"));
    ASSERT_EQ(pib.getTpmLocator(), "tpmLocator");

    // Add a certificate, and do not change the TPM locator.
    pib.addCertificate(*fixture.id1Key1Cert1);
    ASSERT_TRUE(pib.hasIdentity(fixture.id1));
    ASSERT_TRUE(pib.hasKey(fixture.id1Key1Name));
    ASSERT_TRUE(pib.hasCertificate(fixture.id1Key1Cert1->getName()));

    // Set the TPM locator to the same value. Nothing should change.
    pib.setTpmLocator("tpmLocator");
    ASSERT_TRUE(pib.hasIdentity(fixture.id1));
    ASSERT_TRUE(pib.hasKey(fixture.id1Key1Name));
    ASSERT_TRUE(pib.hasCertificate(fixture.id1Key1Cert1->getName()));

    // Change the TPM locator. (The contents of the PIB should not change.)
    pib.setTpmLocator("newTpmLocator");
    ASSERT_TRUE(pib.hasIdentity(fixture.id1));
    ASSERT_TRUE(pib.hasKey(fixture.id1Key1Name));
    ASSERT_TRUE(pib.hasCertificate(fixture.id1Key1Cert1->getName()));
  }
}

TEST_F(TestPibImpl, IdentityManagement)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    // No default identity is set. This should throw an Error.
    ASSERT_THROW(pib.getDefaultIdentity(), Pib::Error);

    // Check for id1, which should not exist.
    ASSERT_EQ(false, pib.hasIdentity(fixture.id1));

    // Add id1, which should be the default.
    pib.addIdentity(fixture.id1);
    ASSERT_EQ(true, pib.hasIdentity(fixture.id1));
    ASSERT_NO_THROW(pib.getDefaultIdentity());
    ASSERT_EQ(fixture.id1, pib.getDefaultIdentity());

    // Add id2, which should not be the default.
    pib.addIdentity(fixture.id2);
    ASSERT_EQ(true, pib.hasIdentity(fixture.id2));
    ASSERT_EQ(fixture.id1, pib.getDefaultIdentity());

    // Explicitly set id2 as the default.
    pib.setDefaultIdentity(fixture.id2);
    ASSERT_EQ(fixture.id2, pib.getDefaultIdentity());

    // Remove id2. The PIB should not have a default identity.
    pib.removeIdentity(fixture.id2);
    ASSERT_EQ(false, pib.hasIdentity(fixture.id2));
    ASSERT_THROW(pib.getDefaultIdentity(), Pib::Error);

    // Set id2 as the default. This should add id2 again.
    pib.setDefaultIdentity(fixture.id2);
    ASSERT_EQ(fixture.id2, pib.getDefaultIdentity());

    // Get all the identities, which should have id1 and id2.
    std::set<Name> idNames = pib.getIdentities();
    ASSERT_EQ(2, idNames.size());
    ASSERT_EQ(1, idNames.count(fixture.id1));
    ASSERT_EQ(1, idNames.count(fixture.id2));
  }
}

TEST_F(TestPibImpl, ClearIdentities)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    pib.setTpmLocator("tpmLocator");

    // Add id, key, and cert.
    pib.addCertificate(*fixture.id1Key1Cert1);
    ASSERT_TRUE(pib.hasIdentity(fixture.id1));
    ASSERT_TRUE(pib.hasKey(fixture.id1Key1Name));
    ASSERT_TRUE(pib.hasCertificate(fixture.id1Key1Cert1->getName()));

    // Clear identities.
    pib.clearIdentities();
    ASSERT_EQ(0, pib.getIdentities().size());
    ASSERT_EQ(0, pib.getKeysOfIdentity(fixture.id1).size());
    ASSERT_EQ(0, pib.getCertificatesOfKey(fixture.id1Key1Name).size());
    ASSERT_EQ("tpmLocator", pib.getTpmLocator());
  }
}

TEST_F(TestPibImpl, KeyManagement)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    // There is no default setting. This should throw an Error.
    ASSERT_THROW(pib.getDefaultKeyOfIdentity(fixture.id1), Pib::Error);

    // Check for id1Key1, which should not exist. Neither should id1.
    ASSERT_EQ(false, pib.hasKey(fixture.id1Key1Name));
    ASSERT_EQ(false, pib.hasIdentity(fixture.id1));

    // Add id1Key1, which should be the default. id1 should be added implicitly.
    pib.addKey
      (fixture.id1, fixture.id1Key1Name, fixture.id1Key1.buf(),
       fixture.id1Key1.size());
    ASSERT_EQ(true, pib.hasKey(fixture.id1Key1Name));
    ASSERT_EQ(true, pib.hasIdentity(fixture.id1));
    Blob keyBits = pib.getKeyBits(fixture.id1Key1Name);
    ASSERT_TRUE(keyBits.equals(fixture.id1Key1));
    ASSERT_NO_THROW(pib.getDefaultKeyOfIdentity(fixture.id1));
    ASSERT_EQ(fixture.id1Key1Name, pib.getDefaultKeyOfIdentity(fixture.id1));

    // Add id1Key2, which should not be the default.
    pib.addKey
      (fixture.id1, fixture.id1Key2Name, fixture.id1Key2.buf(),
       fixture.id1Key2.size());
    ASSERT_EQ(true, pib.hasKey(fixture.id1Key2Name));
    ASSERT_EQ(fixture.id1Key1Name, pib.getDefaultKeyOfIdentity(fixture.id1));

    // Explicitly Set id1Key2 as the default.
    pib.setDefaultKeyOfIdentity(fixture.id1, fixture.id1Key2Name);
    ASSERT_EQ(fixture.id1Key2Name, pib.getDefaultKeyOfIdentity(fixture.id1));

    // Set a non-existing key as the default. This should throw an Error.
    ASSERT_THROW(pib.setDefaultKeyOfIdentity(fixture.id1, Name("/non-existing")),
                      Pib::Error);

    // Remove id1Key2. The PIB should not have a default key.
    pib.removeKey(fixture.id1Key2Name);
    ASSERT_EQ(false, pib.hasKey(fixture.id1Key2Name));
    ASSERT_THROW(pib.getKeyBits(fixture.id1Key2Name), Pib::Error);
    ASSERT_THROW(pib.getDefaultKeyOfIdentity(fixture.id1), Pib::Error);

    // Add id1Key2 back, which should be the default.
    pib.addKey
      (fixture.id1, fixture.id1Key2Name, fixture.id1Key2.buf(),
       fixture.id1Key2.size());
    ASSERT_NO_THROW(pib.getKeyBits(fixture.id1Key2Name));
    ASSERT_EQ(fixture.id1Key2Name, pib.getDefaultKeyOfIdentity(fixture.id1));

    // Get all the keys, which should have id1Key1 and id1Key2.
    std::set<Name> keyNames = pib.getKeysOfIdentity(fixture.id1);
    ASSERT_EQ(2, keyNames.size());
    ASSERT_EQ(1, keyNames.count(fixture.id1Key1Name));
    ASSERT_EQ(1, keyNames.count(fixture.id1Key2Name));

    // Remove id1, which should remove all the keys.
    pib.removeIdentity(fixture.id1);
    keyNames = pib.getKeysOfIdentity(fixture.id1);
    ASSERT_EQ(0, keyNames.size());
  }
}

TEST_F(TestPibImpl, CertificateManagement)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    // There is no default setting. This should throw an Error.
    ASSERT_THROW(pib.getDefaultCertificateOfKey(fixture.id1Key1Name), Pib::Error);

    // Check for id1Key1Cert1, which should not exist. Neither should id1 or id1Key1.
    ASSERT_EQ(false, pib.hasCertificate(fixture.id1Key1Cert1->getName()));
    ASSERT_EQ(false, pib.hasIdentity(fixture.id1));
    ASSERT_EQ(false, pib.hasKey(fixture.id1Key1Name));

    // Add id1Key1Cert1, which should be the default.
    // id1 and id1Key1 should be added implicitly.
    pib.addCertificate(*fixture.id1Key1Cert1);
    ASSERT_EQ(true, pib.hasCertificate(fixture.id1Key1Cert1->getName()));
    ASSERT_EQ(true, pib.hasIdentity(fixture.id1));
    ASSERT_EQ(true, pib.hasKey(fixture.id1Key1Name));
    ASSERT_TRUE(pib.getCertificate(fixture.id1Key1Cert1->getName())->wireEncode()
                .equals(fixture.id1Key1Cert1->wireEncode()));
    ASSERT_NO_THROW(pib.getDefaultCertificateOfKey(fixture.id1Key1Name));
    // Use the wire encoding to check equivalence.
    ASSERT_TRUE(fixture.id1Key1Cert1->wireEncode().equals
                (pib.getDefaultCertificateOfKey(fixture.id1Key1Name)->wireEncode()));

    // Add id1Key1Cert2, which should not be the default.
    pib.addCertificate(*fixture.id1Key1Cert2);
    ASSERT_EQ(true, pib.hasCertificate(fixture.id1Key1Cert2->getName()));
    ASSERT_TRUE(fixture.id1Key1Cert1->wireEncode().equals
                (pib.getDefaultCertificateOfKey(fixture.id1Key1Name)->wireEncode()));

    // Explicitly set id1Key1Cert2 as the default.
    pib.setDefaultCertificateOfKey(fixture.id1Key1Name, fixture.id1Key1Cert2->getName());
    ASSERT_TRUE(fixture.id1Key1Cert2->wireEncode().equals
                (pib.getDefaultCertificateOfKey(fixture.id1Key1Name)->wireEncode()));

    // Set a non-existing certificate as the default. This should throw an Error.
    ASSERT_THROW(pib.setDefaultCertificateOfKey(fixture.id1Key1Name, Name("/non-existing")),
                      Pib::Error);

    // Remove id1Key1Cert2, which should not have a default certificate.
    pib.removeCertificate(fixture.id1Key1Cert2->getName());
    ASSERT_EQ(false, pib.hasCertificate(fixture.id1Key1Cert2->getName()));
    ASSERT_THROW(pib.getCertificate(fixture.id1Key1Cert2->getName()), Pib::Error);
    ASSERT_THROW(pib.getDefaultCertificateOfKey(fixture.id1Key1Name), Pib::Error);

    // Add id1Key1Cert2, which should be the default.
    pib.addCertificate(*fixture.id1Key1Cert2);
    ASSERT_NO_THROW(pib.getCertificate(fixture.id1Key1Cert1->getName()));
    ASSERT_TRUE(fixture.id1Key1Cert2->wireEncode().equals
                (pib.getDefaultCertificateOfKey(fixture.id1Key1Name)->wireEncode()));

    // Get all certificates, which should have id1Key1Cert1 and id1Key1Cert2.
    std::set<Name> certNames = pib.getCertificatesOfKey(fixture.id1Key1Name);
    ASSERT_EQ(2, certNames.size());
    ASSERT_EQ(1, certNames.count(fixture.id1Key1Cert1->getName()));
    ASSERT_EQ(1, certNames.count(fixture.id1Key1Cert2->getName()));

    // Remove id1Key1, which should remove all the certificates.
    pib.removeKey(fixture.id1Key1Name);
    certNames = pib.getCertificatesOfKey(fixture.id1Key1Name);
    ASSERT_EQ(0, certNames.size());
  }
}

TEST_F(TestPibImpl, DefaultsManagement)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    pib.addIdentity(fixture.id1);
    ASSERT_EQ(fixture.id1, pib.getDefaultIdentity());

    pib.addIdentity(fixture.id2);
    ASSERT_EQ(fixture.id1, pib.getDefaultIdentity());

    pib.removeIdentity(fixture.id1);
    ASSERT_THROW(pib.getDefaultIdentity(), Pib::Error);

    pib.addKey(fixture.id2, fixture.id2Key1Name, fixture.id2Key1.buf(), fixture.id2Key1.size());
    ASSERT_EQ(fixture.id2, pib.getDefaultIdentity());
    ASSERT_EQ(fixture.id2Key1Name, pib.getDefaultKeyOfIdentity(fixture.id2));

    pib.addKey(fixture.id2, fixture.id2Key2Name, fixture.id2Key2.buf(), fixture.id2Key2.size());
    ASSERT_EQ(fixture.id2Key1Name, pib.getDefaultKeyOfIdentity(fixture.id2));

    pib.removeKey(fixture.id2Key1Name);
    ASSERT_THROW(pib.getDefaultKeyOfIdentity(fixture.id2), Pib::Error);

    pib.addCertificate(*fixture.id2Key2Cert1);
    ASSERT_EQ(fixture.id2Key2Name, pib.getDefaultKeyOfIdentity(fixture.id2));
    ASSERT_EQ(fixture.id2Key2Cert1->getName(),
                      pib.getDefaultCertificateOfKey(fixture.id2Key2Name)->getName());

    pib.addCertificate(*fixture.id2Key2Cert2);
    ASSERT_EQ(fixture.id2Key2Cert1->getName(),
                      pib.getDefaultCertificateOfKey(fixture.id2Key2Name)->getName());

    pib.removeCertificate(fixture.id2Key2Cert2->getName());
    ASSERT_EQ(fixture.id2Key2Cert1->getName(),
                      pib.getDefaultCertificateOfKey(fixture.id2Key2Name)->getName());
  }
}

TEST_F(TestPibImpl, Overwrite)
{
  for (size_t i = 0; i < sizeof(pibImpls) / sizeof(pibImpls[0]); ++i) {
    PibDataFixture& fixture = *pibImpls[i];
    PibImpl& pib = *fixture.pib;

    // Check for id1Key1, which should not exist.
    pib.removeIdentity(fixture.id1);
    ASSERT_EQ(false, pib.hasKey(fixture.id1Key1Name));

    // Add id1Key1.
    pib.addKey(fixture.id1, fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size());
    ASSERT_EQ(true, pib.hasKey(fixture.id1Key1Name));
    Blob keyBits = pib.getKeyBits(fixture.id1Key1Name);
    ASSERT_TRUE(keyBits.equals(fixture.id1Key1));

    // To check overwrite, add a key with the same name.
    pib.addKey(fixture.id1, fixture.id1Key1Name, fixture.id1Key2.buf(), fixture.id1Key2.size());
    Blob keyBits2 = pib.getKeyBits(fixture.id1Key1Name);
    ASSERT_TRUE(keyBits2.equals(fixture.id1Key2));

    // Check for id1Key1Cert1, which should not exist.
    pib.removeIdentity(fixture.id1);
    ASSERT_EQ(false, pib.hasCertificate(fixture.id1Key1Cert1->getName()));

    // Add id1Key1Cert1.
    pib.addKey(fixture.id1, fixture.id1Key1Name, fixture.id1Key1.buf(), fixture.id1Key1.size());
    pib.addCertificate(*fixture.id1Key1Cert1);
    ASSERT_EQ(true, pib.hasCertificate(fixture.id1Key1Cert1->getName()));

    CertificateV2 cert = *pib.getCertificate(fixture.id1Key1Cert1->getName());
    ASSERT_TRUE(cert.wireEncode().equals(fixture.id1Key1Cert1->wireEncode()));

    // Create a fake certificate with the same name.
    CertificateV2 cert2 = *fixture.id1Key2Cert1;
    cert2.setName(fixture.id1Key1Cert1->getName());
    cert2.setSignature(*fixture.id1Key2Cert1->getSignature());
    pib.addCertificate(cert2);

    CertificateV2 cert3 = *pib.getCertificate(fixture.id1Key1Cert1->getName());
    ASSERT_TRUE(cert3.wireEncode().equals(cert2.wireEncode()));

    // Check that both the key and certificate are overwritten.
    Blob keyBits3 = pib.getKeyBits(fixture.id1Key1Name);
    ASSERT_TRUE(keyBits3.equals(fixture.id1Key2));
  }
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

