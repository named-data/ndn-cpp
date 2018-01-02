/**
 * Copyright (C) 2014-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/v2/trust-anchor-container.t.cpp
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>
#include <stdexcept>
#include "identity-management-fixture.hpp"
#include <ndn-cpp/security/v2/trust-anchor-container.hpp>

using namespace std;
using namespace ndn;

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

class TestTrustAnchorContainer : public ::testing::Test {
public:
  TestTrustAnchorContainer()
  {
    // Create a directory and prepares two certificates.
    certificateDirectoryPath = getPolicyConfigDirectory() + "/test-cert-dir";
    ::system(("mkdir -p \"" + certificateDirectoryPath + "\"").c_str());

    certificatePath1 = certificateDirectoryPath + "/trust-anchor-1.cert";
    certificatePath2 = certificateDirectoryPath + "/trust-anchor-2.cert";

    identity1 = fixture.addIdentity("/TestAnchorContainer/First");
    certificate1 = identity1->getDefaultKey()->getDefaultCertificate();
    fixture.saveCertificateToFile(*certificate1, certificatePath1);

    identity2 = fixture.addIdentity("/TestAnchorContainer/Second");
    certificate2 = identity2->getDefaultKey()->getDefaultCertificate();
    fixture.saveCertificateToFile(*certificate2, certificatePath2);
  }

  virtual void
  TearDown()
  {
    remove(certificatePath1.c_str());
    remove(certificatePath2.c_str());
  }

  TrustAnchorContainer anchorContainer;

  string certificateDirectoryPath;
  string certificatePath1;
  string certificatePath2;

  ptr_lib::shared_ptr<PibIdentity> identity1;
  ptr_lib::shared_ptr<PibIdentity> identity2;

  ptr_lib::shared_ptr<CertificateV2> certificate1;
  ptr_lib::shared_ptr<CertificateV2> certificate2;
  IdentityManagementFixture fixture;
};

TEST_F(TestTrustAnchorContainer, Insert)
{
  // Static
  anchorContainer.insert("group1", *certificate1);
  ASSERT_TRUE(!!anchorContainer.find(certificate1->getName()));
  ASSERT_TRUE(!!anchorContainer.find(identity1->getName()));
  ptr_lib::shared_ptr<CertificateV2> certificate =
    anchorContainer.find(certificate1->getName());
  // Re-inserting the same certificate should do nothing.
  ASSERT_NO_THROW(anchorContainer.insert("group1", *certificate1));
  // It should still be the same instance of the certificate.
  ASSERT_EQ(certificate, anchorContainer.find(certificate1->getName()));
  // Cannot add a dynamic group when the static already exists.
  ASSERT_THROW(anchorContainer.insert
    ("group1", certificatePath1, 400.0), TrustAnchorContainer::Error);
  ASSERT_EQ(1, anchorContainer.getGroup("group1").size());
  ASSERT_EQ(1, anchorContainer.size());

  // From file
  anchorContainer.insert("group2", certificatePath2, 400.0);
  ASSERT_TRUE(!!anchorContainer.find(certificate2->getName()));
  ASSERT_TRUE(!!anchorContainer.find(identity2->getName()));
  ASSERT_THROW(anchorContainer.insert("group2", *certificate2),
               TrustAnchorContainer::Error);
  ASSERT_THROW(anchorContainer.insert("group2", certificatePath2, 400.0),
               TrustAnchorContainer::Error);
  ASSERT_EQ(1, anchorContainer.getGroup("group2").size());
  ASSERT_EQ(2, anchorContainer.size());

  remove(certificatePath2.c_str());
  // Wait for the refresh period to expire.
  usleep(500000);

  ASSERT_TRUE(!anchorContainer.find(identity2->getName()));
  ASSERT_TRUE(!anchorContainer.find(certificate2->getName()));
  ASSERT_EQ(0, anchorContainer.getGroup("group2").size());
  ASSERT_EQ(1, anchorContainer.size());

  TrustAnchorGroup& group = anchorContainer.getGroup("group1");
  StaticTrustAnchorGroup* staticGroup =
    dynamic_cast<StaticTrustAnchorGroup*>(&group);
  ASSERT_TRUE(staticGroup);
  ASSERT_EQ(1, staticGroup->size());
  staticGroup->remove(certificate1->getName());
  ASSERT_EQ(0, staticGroup->size());
  ASSERT_EQ(0, anchorContainer.size());

  ASSERT_THROW(anchorContainer.getGroup("non-existing-group"),
               TrustAnchorContainer::Error);
}

TEST_F(TestTrustAnchorContainer, DynamicAnchorFromDirectory)
{
  remove(certificatePath2.c_str());

  anchorContainer.insert("group", certificateDirectoryPath, 400.0, true);

  ASSERT_TRUE(!!anchorContainer.find(identity1->getName()));
  ASSERT_TRUE(!anchorContainer.find(identity2->getName()));
  ASSERT_EQ(1, anchorContainer.getGroup("group").size());

  fixture.saveCertificateToFile(*certificate2, certificatePath2);

  // Wait for the refresh period to expire. The dynamic anchors should remain.
  usleep(500000);

  ASSERT_TRUE(!!anchorContainer.find(identity1->getName()));
  ASSERT_TRUE(!!anchorContainer.find(identity2->getName()));
  ASSERT_EQ(2, anchorContainer.getGroup("group").size());

  ::system(("rm -rf \"" + certificateDirectoryPath + "\"").c_str());

  // Wait for the refresh period to expire. The dynamic anchors should be gone.
  usleep(500000);

  ASSERT_TRUE(!anchorContainer.find(identity1->getName()));
  ASSERT_TRUE(!anchorContainer.find(identity2->getName()));
  ASSERT_EQ(0, anchorContainer.getGroup("group").size());
}

TEST_F(TestTrustAnchorContainer, FindByInterest)
{
  anchorContainer.insert("group1", certificatePath1, 400.0);
  Interest interest(identity1->getName());
  ASSERT_TRUE(!!anchorContainer.find(interest));
  Interest interest1(identity1->getName().getPrefix(-1));
  ASSERT_TRUE(!!anchorContainer.find(interest1));
  Interest interest2(Name(identity1->getName()).appendVersion(1));
  ASSERT_TRUE(!anchorContainer.find(interest2));

  ptr_lib::shared_ptr<CertificateV2> certificate3 =
    fixture.addCertificate(identity1->getDefaultKey(), "3");
  ptr_lib::shared_ptr<CertificateV2> certificate4 = 
    fixture.addCertificate(identity1->getDefaultKey(), "4");
  ptr_lib::shared_ptr<CertificateV2> certificate5 =
    fixture.addCertificate(identity1->getDefaultKey(), "5");

  CertificateV2 certificate3Copy(*certificate3);
  anchorContainer.insert("group2", certificate3Copy);
  anchorContainer.insert("group3", *certificate4);
  anchorContainer.insert("group4", *certificate5);

  Interest interest3(certificate3->getKeyName());
  ptr_lib::shared_ptr<CertificateV2> foundCertificate = anchorContainer.find
    (interest3);
  ASSERT_TRUE(!!foundCertificate);
  ASSERT_TRUE(interest3.getName().isPrefixOf(foundCertificate->getName()));
  ASSERT_TRUE(certificate3->getName().equals(foundCertificate->getName()));

  interest3.getExclude().appendComponent
    (certificate3->getName().get(CertificateV2::ISSUER_ID_OFFSET));
  foundCertificate = anchorContainer.find(interest3);
  ASSERT_TRUE(!!foundCertificate);
  ASSERT_TRUE(interest3.getName().isPrefixOf(foundCertificate->getName()));
  ASSERT_TRUE(!foundCertificate->getName().equals(certificate3->getName()));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
