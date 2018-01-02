/**
 * Copyright (C) 2014-2018 Regents of the University of California.
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
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <fstream>
#include <ndn-cpp/security/v2/certificate-fetcher-offline.hpp>
#include <ndn-cpp/security/validator-config.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

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

class TestValidationResult {
public:
  /**
   * Create a TestValidationResult whose state_ will reference the given Data.
   * @param data The Data packed for the state_, which must remain valid.
   */
  TestValidationResult(const Data& data)
  : data_(data)
  {
    reset();
  }

  /**
   * Reset all the results to false, to get ready for another result.
   */
  void
  reset()
  {
    state_.reset(new DataValidationState
      (data_,
       bind(&TestValidationResult::successCallback, this, _1),
       bind(&TestValidationResult::failureCallback, this, _1, _2)));

    calledSuccess_ = false;
    calledFailure_ = false;
    calledContinue_ = false;
  }

  /**
   * Call reset() then call validator.checkPolicy to set this object's results.
   * When finished, you can check calledSuccess_, etc.
   * @param validator The ValidatorConfig for calling checkPolicy.
   */
  void
  checkPolicy(ValidatorConfig& validator)
  {
    reset();
    validator.getPolicy().checkPolicy
      (data_, state_,
       bind(&TestValidationResult::continueValidation, this, _1, _2));
  }

  void
  successCallback(const Data& data)
  {
    calledSuccess_ = true;
  }

  void
  failureCallback(const Data& data, const ValidationError& error)
  {
    calledFailure_ = true;
  }

  void
  continueValidation
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state)
  {
    calledContinue_ = true;
  }

  const Data& data_;
  ptr_lib::shared_ptr<DataValidationState> state_;
  bool calledSuccess_;
  bool calledFailure_;
  bool calledContinue_;
};

class TestValidationPolicyConfig : public ::testing::Test {
public:
  TestValidationPolicyConfig()
  {
    policyConfigDirectory_ = getPolicyConfigDirectory();
  }

  string policyConfigDirectory_;
};

TEST_F(TestValidationPolicyConfig, NameRelation)
{
  // Set up the validators.
  ptr_lib::shared_ptr<CertificateFetcher> fetcher(new CertificateFetcherOffline());
  ValidatorConfig validatorPrefix(fetcher);
  ValidatorConfig validatorEqual(fetcher);
  ValidatorConfig validatorStrict(fetcher);

  validatorPrefix.load(policyConfigDirectory_ + "/relation_ruleset_prefix.conf");
  validatorEqual.load(policyConfigDirectory_ + "/relation_ruleset_equal.conf");
  validatorStrict.load(policyConfigDirectory_ + "/relation_ruleset_strict.conf");

  // Set up a Data packet and result object.
  Data data;
  KeyLocator::getFromSignature(data.getSignature()).setType(ndn_KeyLocatorType_KEYNAME);
  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/KEY/123"));
  TestValidationResult result(data);

  data.setName(Name("/TestRule1"));
  result.checkPolicy(validatorPrefix);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_)
    << "Prefix relation should match prefix name";
  result.checkPolicy(validatorEqual);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_)
    << "Equal relation should match prefix name";
  result.checkPolicy(validatorStrict);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_)
    << "Strict-prefix relation should not match prefix name";

  data.setName(Name("/TestRule1/hi"));
  result.checkPolicy(validatorPrefix);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_)
    << "Prefix relation should match longer name";
  result.checkPolicy(validatorEqual);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_)
    << "Equal relation should not match longer name";
  result.checkPolicy(validatorStrict);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_)
    << "Strict-prefix relation should match longer name";

  data.setName(Name("/Bad/TestRule1/"));
  result.checkPolicy(validatorPrefix);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_)
    << "Prefix relation should not match inner components";
  result.checkPolicy(validatorEqual);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_)
    << "Equal relation should not match inner components";
  result.checkPolicy(validatorStrict);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_)
    << "Strict-prefix relation should  not match inner components";
}

TEST_F(TestValidationPolicyConfig, SimpleRegex)
{
  // Set up the validator.
  ptr_lib::shared_ptr<CertificateFetcher> fetcher(new CertificateFetcherOffline());
  ValidatorConfig validator(fetcher);
  validator.load(policyConfigDirectory_ + "/regex_ruleset.conf");

  // Set up a Data packet and result object.
  Data data;
  KeyLocator::getFromSignature(data.getSignature()).setType(ndn_KeyLocatorType_KEYNAME);
  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/KEY/123"));
  TestValidationResult result(data);

  data.setName(Name("/SecurityTestSecRule/Basic"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);

  data.setName(Name("/SecurityTestSecRule/Basic/More"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);

  data.setName(Name("/SecurityTestSecRule/"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);

  data.setName(Name("/SecurityTestSecRule/Other/TestData"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);

  data.setName(Name("/Basic/Data"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);
}

TEST_F(TestValidationPolicyConfig, Hierarchical)
{
  // Set up the validator.
  ptr_lib::shared_ptr<CertificateFetcher> fetcher(new CertificateFetcherOffline());
  ValidatorConfig validator(fetcher);
  validator.load(policyConfigDirectory_ + "/hierarchical_ruleset.conf");

  // Set up a Data packet and result object.
  Data data;
  KeyLocator::getFromSignature(data.getSignature()).setType(ndn_KeyLocatorType_KEYNAME);
  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/Longer/KEY/123"));
  TestValidationResult result(data);

  data.setName(Name("/SecurityTestSecRule/Basic/Data1"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);

  data.setName(Name("/SecurityTestSecRule/Basic/Longer/Data2"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);

  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/KEY/123"));

  data.setName(Name("/SecurityTestSecRule/Basic/Data1"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);

  data.setName(Name("/SecurityTestSecRule/Basic/Longer/Data2"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledContinue_ && !result.calledFailure_);
}

TEST_F(TestValidationPolicyConfig, HyperRelation)
{
  // Set up the validator.
  ptr_lib::shared_ptr<CertificateFetcher> fetcher(new CertificateFetcherOffline());
  ValidatorConfig validator(fetcher);
  validator.load(policyConfigDirectory_ + "/hyperrelation_ruleset.conf");

  // Set up a Data packet and result object.
  Data data;
  KeyLocator::getFromSignature(data.getSignature()).setType(ndn_KeyLocatorType_KEYNAME);
  TestValidationResult result(data);

  data.setName(Name("/SecurityTestSecRule/Basic/Longer/Data2"));

  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/Longer/KEY/123"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);
  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/KEY/123"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);

  data.setName(Name("/SecurityTestSecRule/Basic/Other/Data1"));

  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/Longer/KEY/123"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);
  KeyLocator::getFromSignature(data.getSignature()).setKeyName
    (Name("/SecurityTestSecRule/Basic/KEY/123"));
  result.checkPolicy(validator);
  ASSERT_TRUE(result.calledFailure_ && !result.calledContinue_);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

