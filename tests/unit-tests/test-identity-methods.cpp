/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
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
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/policy/self-verify-policy-manager.hpp>
#include <ndn-cpp/security/identity/basic-identity-storage.hpp>
#include <ndn-cpp/security/identity/file-private-key-storage.hpp>
#include "../../src/encoding/base64.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static double
getNowSeconds()
{
  struct timeval t;
  gettimeofday(&t, 0);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

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

const char* RSA_DER =
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuFoDcNtffwbfFix64fw0\
hI2tKMkFrc6Ex7yw0YLMK9vGE8lXOyBl/qXabow6RCz+GldmFN6E2Qhm1+AX3Zm5\
sj3H53/HPtzMefvMQ9X7U+lK8eNMWawpRzvBh4/36VrK/awlkNIVIQ9aXj6q6BVe\
zL+zWT/WYemLq/8A1/hHWiwCtfOH1xQhGqWHJzeSgwIgOOrzxTbRaCjhAb1u2TeV\
yx/I9H/DV+AqSHCaYbB92HDcDN0kqwSnUf5H1+osE9MR5DLBLhXdSiULSgxT3Or/\
y2QgsgUK59WrjhlVMPEiHHRs15NZJbL1uQFXjgScdEarohcY3dilqotineFZCeN8\
DwIDAQAB";

class VerifyCounter
{
public:
  VerifyCounter()
  {
    onVerifiedCallCount_ = 0;
    onVerifyFailedCallCount_ = 0;
  }

  void
  onVerified(const ptr_lib::shared_ptr<Data>& data)
  {
    ++onVerifiedCallCount_;
  }

  void
  onVerifyFailed(const ptr_lib::shared_ptr<Data>& data)
  {
    ++onVerifyFailedCallCount_;
  }

  int onVerifiedCallCount_;
  int onVerifyFailedCallCount_;
};

class TestSqlIdentityStorage : public ::testing::Test {
public:
  TestSqlIdentityStorage()
  {
    databaseFilePath = getPolicyConfigDirectory() + "/test-public-info.db";
    remove(databaseFilePath.c_str());

    identityStorage.reset(new BasicIdentityStorage(databaseFilePath));
    identityManager.reset(new IdentityManager
      (identityStorage, ptr_lib::make_shared<FilePrivateKeyStorage>()));
    policyManager.reset(new SelfVerifyPolicyManager(identityStorage.get()));
    keyChain.reset(new KeyChain(identityManager, policyManager));
  }

  virtual void
  TearDown()
  {
    remove(databaseFilePath.c_str());
  }

  string databaseFilePath;
  ptr_lib::shared_ptr<IdentityStorage> identityStorage;
  ptr_lib::shared_ptr<IdentityManager> identityManager;
  ptr_lib::shared_ptr<PolicyManager> policyManager;
  ptr_lib::shared_ptr<KeyChain> keyChain;
};

TEST_F(TestSqlIdentityStorage, IdentityCreateDelete)
{
  Name identityName = Name("/TestIdentityStorage/Identity").appendVersion
    (getNowSeconds());

  Name keyName = keyChain->createIdentity(identityName);

  ASSERT_TRUE(identityStorage->doesIdentityExist(identityName)) <<
      "Identity was not added to IdentityStorage";
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName)) <<
      "Key was not added to IdentityStorage";
  Name certificateName = identityManager->getDefaultCertificateNameForIdentity
    (identityName);
  
  keyChain->deleteIdentity(identityName);
  ASSERT_FALSE(identityStorage->doesIdentityExist(identityName)) <<
      "Identity still in IdentityStorage after revoking";
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName)) <<
      "Key still in IdentityStorage after identity was deletedInfo";
  ASSERT_FALSE(identityStorage->doesCertificateExist(certificateName)) <<
      "Certificate still in IdentityStorage after identity was deletedInfo";

  ASSERT_THROW
    (identityManager->getDefaultCertificateNameForIdentity(identityName),
     SecurityException);
}

TEST_F(TestSqlIdentityStorage, KeyCreateDelete)
{
  Name identityName = Name("/TestIdentityStorage/Identity").appendVersion
    ((uint64_t)getNowSeconds());

  Name keyName1 = keyChain->generateRSAKeyPair(identityName, true);
  keyChain->getIdentityManager()->setDefaultKeyForIdentity(keyName1);

  Name keyName2 = keyChain->generateRSAKeyPair(identityName, false);

  ASSERT_EQ(identityManager->getDefaultKeyNameForIdentity(identityName),
    keyName1) << "Default key name was changed without explicit request";
  ASSERT_NE(identityManager->getDefaultKeyNameForIdentity(identityName),
    keyName2) << "Newly created key replaced default key without explicit request";

  identityStorage->deletePublicKeyInfo(keyName2);

  ASSERT_FALSE(identityStorage->doesKeyExist(keyName2));
  identityStorage->deleteIdentityInfo(identityName);
}

TEST_F(TestSqlIdentityStorage, AutoCreateIdentity)
{
  Name keyName1("/TestSqlIdentityStorage/KeyType/RSA/ksk-12345");
  Name identityName = keyName1.getPrefix(-1);

  vector<uint8_t> decodedKey;
  fromBase64(RSA_DER, decodedKey);
  identityStorage->addKey(keyName1, KEY_TYPE_RSA, Blob(decodedKey));
  identityStorage->setDefaultKeyNameForIdentity(keyName1);

  ASSERT_TRUE(identityStorage->doesKeyExist(keyName1)) <<
    "Key was not added";
  ASSERT_TRUE(identityStorage->doesIdentityExist(identityName)) <<
    "Identity for key was not automatically created";

  ASSERT_EQ(identityManager->getDefaultKeyNameForIdentity(identityName),
    keyName1) << "Default key was not set on identity creation";

  ASSERT_THROW
    (identityStorage->getDefaultCertificateNameForKey(keyName1),
     SecurityException);

  // we have no private key for signing
  ASSERT_THROW(identityManager->selfSign(keyName1), SecurityException);

  ASSERT_THROW
    (identityStorage->getDefaultCertificateNameForKey(keyName1),
     SecurityException);

  ASSERT_THROW
    (identityManager->getDefaultCertificateNameForIdentity(identityName),
     SecurityException);

  Name keyName2 = identityManager->generateRSAKeyPairAsDefault(identityName);
  ptr_lib::shared_ptr<IdentityCertificate> cert =
    identityManager->selfSign(keyName2);
  identityManager->addCertificateAsIdentityDefault(*cert);

  Name certName1 = identityManager->getDefaultCertificateNameForIdentity(identityName);
  Name certName2 = identityStorage->getDefaultCertificateNameForKey(keyName2);

  ASSERT_EQ(certName1, certName2) <<
    "Key-certificate mapping and identity-certificate mapping are not consistent";

  keyChain->deleteIdentity(identityName);
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName1));
}

TEST_F(TestSqlIdentityStorage, CertificateAddDelete)
{
  Name identityName = Name("/TestIdentityStorage/Identity").appendVersion
    ((uint64_t)getNowSeconds());

  identityManager->createIdentity(identityName, KeyChain::DEFAULT_KEY_PARAMS);
  Name keyName1 = identityManager->getDefaultKeyNameForIdentity(identityName);
  ptr_lib::shared_ptr<IdentityCertificate> cert2 =
    identityManager->selfSign(keyName1);
  identityStorage->addCertificate(*cert2);
  Name certName2 = cert2->getName();

  Name certName1 = identityManager->getDefaultCertificateNameForIdentity(identityName);
  ASSERT_NE(certName1, certName2) <<
    "New certificate was set as default without explicit request";

  identityStorage->deleteCertificateInfo(certName1);
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName2));
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName1));

  keyChain->deleteIdentity(identityName);
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName2));
}

TEST_F(TestSqlIdentityStorage, Stress)
{
  Name identityName = Name("/TestSecPublicInfoSqlite3/Delete").appendVersion
    ((uint64_t)getNowSeconds());

  // ndn-cxx returns the cert name, but the IndentityManager docstring
  // specifies a key.
  Name keyName1 = keyChain->createIdentity(identityName);
  Name certName1 = identityStorage->getDefaultCertificateNameForKey(keyName1);
  Name keyName2 = keyChain->generateRSAKeyPairAsDefault(identityName);

  ptr_lib::shared_ptr<IdentityCertificate> cert2 =
    identityManager->selfSign(keyName2);
  Name certName2 = cert2->getName();
  identityManager->addCertificateAsDefault(*cert2);

  Name keyName3 = keyChain->generateRSAKeyPairAsDefault(identityName);
  ptr_lib::shared_ptr<IdentityCertificate> cert3 =
    identityManager->selfSign(keyName3);
  Name certName3 = cert3->getName();
  identityManager->addCertificateAsDefault(*cert3);

  ptr_lib::shared_ptr<IdentityCertificate> cert4 =
    identityManager->selfSign(keyName3);
  identityManager->addCertificateAsDefault(*cert4);
  Name certName4 = cert4->getName();

  ptr_lib::shared_ptr<IdentityCertificate> cert5 =
    identityManager->selfSign(keyName3);
  identityManager->addCertificateAsDefault(*cert5);
  Name certName5 = cert5->getName();

  ASSERT_TRUE(identityStorage->doesIdentityExist(identityName));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName1));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName2));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName3));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName1));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName2));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName3));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName4));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName5));

  identityStorage->deleteCertificateInfo(certName5);
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName5));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName4));
  ASSERT_TRUE(identityStorage->doesCertificateExist(certName3));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName2));

  identityStorage->deletePublicKeyInfo(keyName3);
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName4));
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName3));
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName3));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName2));
  ASSERT_TRUE(identityStorage->doesKeyExist(keyName1));
  ASSERT_TRUE(identityStorage->doesIdentityExist(identityName));

  keyChain->deleteIdentity(identityName);
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName2));
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName2));
  ASSERT_FALSE(identityStorage->doesCertificateExist(certName1));
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName1));
  ASSERT_FALSE(identityStorage->doesIdentityExist(identityName));
}

TEST_F(TestSqlIdentityStorage, EcdsaIdentity)
{
  Name identityName("/TestSqlIdentityStorage/KeyType/ECDSA");
  Name keyName = identityManager->generateEcdsaKeyPairAsDefault(identityName);
  ptr_lib::shared_ptr<IdentityCertificate> cert =
    identityManager->selfSign(keyName);
  identityManager->addCertificateAsIdentityDefault(*cert);

  // Check the self-signature.
  VerifyCounter counter;
  Name certName = identityStorage->getDefaultCertificateNameForKey(keyName);
  keyChain->verifyData
    (cert, bind(&VerifyCounter::onVerified, &counter, _1),
     bind(&VerifyCounter::onVerifyFailed, &counter, _1));
  ASSERT_EQ(counter.onVerifiedCallCount_, 1) << "Verification callback was not used.";

  keyChain->deleteIdentity(identityName);
  ASSERT_FALSE(identityStorage->doesKeyExist(keyName));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
