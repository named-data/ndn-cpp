/**
 * Copyright (C) 2014-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/v2/validator.t.cpp
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
#include "validator-fixture.hpp"
#include <ndn-cpp/security/v2/validation-policy-simple-hierarchy.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class TestValidator : public ::testing::Test {
public:
  TestValidator()
  : fixture_(ptr_lib::make_shared<ValidationPolicySimpleHierarchy>())
  {
  }

  /**
   * Call fixture_.validator_.validate and if it calls the failureCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test fails.
   */
  void
  validateExpectSuccess(const Data& data, const std::string& message)
  {
    fixture_.validator_.validate
      (data,
       bind(&TestValidator::dataShouldSucceed, this, _1),
       bind(&TestValidator::dataShouldNotFail, this, _1, _2, message));
  }

  /**
   * Call fixture_.validator_.validate and if it calls the successCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test succeeds.
   */
  void
  validateExpectFailure(const Data& data, const std::string& message)
  {
    fixture_.validator_.validate
      (data,
       bind(&TestValidator::dataShouldNotSucceed, this, _1, message),
       bind(&TestValidator::dataShouldFail, this, _1, _2));
  }

  void
  dataShouldSucceed(const Data& data)
  {
  }

  void
  dataShouldNotSucceed(const Data& data, const std::string& message)
  {
    FAIL() << message;
  }

  void
  dataShouldFail(const Data& data, const ValidationError& error)
  {
  }

  void
  dataShouldNotFail
    (const Data& data, const ValidationError& error,
     const std::string& message)
  {
    FAIL() << message;
  }

  void
  processInterestAsInfiniteCertificateChain
    (const ndn::Interest& interest, const ndn::OnData& onData,
     const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack)
  {
    // Create another key for the same identity and sign it properly.
    ptr_lib::shared_ptr<PibKey> parentKey =
      fixture_.keyChain_.createKey(*fixture_.subIdentity_);
    ptr_lib::shared_ptr<PibKey> requestedKey =
      fixture_.subIdentity_->getKey(interest.getName());

    Name certificateName = requestedKey->getName();
    certificateName.append("looper").appendVersion(1);
    ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());
    certificate->setName(certificateName);

    // Set the MetaInfo.
    certificate->getMetaInfo().setType(ndn_ContentType_KEY);
    // Set the freshness period to one hour.
    certificate->getMetaInfo().setFreshnessPeriod(3600 * 1000.0);

    // Set the content.
    certificate->setContent(requestedKey->getPublicKey());

    // Set SigningInfo.
    SigningInfo params(parentKey);
    // Validity period from 10 days before to 10 days after now.
    MillisecondsSince1970 now = ndn_getNowMilliseconds();
    params.setValidityPeriod(ValidityPeriod
      (now - 10 * 24 * 3600 * 1000.0, now + 10 * 24 * 3600 * 1000.0));

    fixture_.keyChain_.sign(*certificate, params);
    onData(ptr_lib::make_shared<Interest>(interest), certificate);
  }

  void
  makeCertificate(PibKey& key, const ptr_lib::shared_ptr<PibKey>& signer)
  {
    // Copy the default certificate.
    CertificateV2 request(*key.getDefaultCertificate());
    request.setName(Name(key.getName()).append("looper").appendVersion(1));

    // Set SigningInfo.
    SigningInfo params(signer);
    // Validity period from 100 days before to 100 days after now.
    MillisecondsSince1970 now = ndn_getNowMilliseconds();
    params.setValidityPeriod(ValidityPeriod
      (now - 100 * 24 * 3600 * 1000.0, now + 100 * 24 * 3600 * 1000.0));
    fixture_.keyChain_.sign(request, params);
    fixture_.keyChain_.addCertificate(key, request);

    fixture_.cache_.insert(request);
  };

  HierarchicalValidatorFixture fixture_;
};

TEST_F(TestValidator, ConstructorSetValidator)
{
  Validator& validator = fixture_.validator_;

  ptr_lib::shared_ptr<ValidationPolicy> middlePolicy
    (new ValidationPolicySimpleHierarchy());
  ptr_lib::shared_ptr<ValidationPolicy> innerPolicy
    (new ValidationPolicySimpleHierarchy());

  validator.getPolicy().setInnerPolicy(middlePolicy);
  validator.getPolicy().setInnerPolicy(innerPolicy);

  ASSERT_TRUE(!!validator.getPolicy().validator_);
  ASSERT_TRUE(!!validator.getPolicy().getInnerPolicy().validator_);
  ASSERT_TRUE(!!validator.getPolicy().getInnerPolicy().getInnerPolicy().validator_);
}

TEST_F(TestValidator, Timeouts)
{
  // Disable responses from the simulated Face.
  fixture_.face_.processInterest_ = 0;

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  validateExpectFailure(data, "Should fail to retrieve certificate");
  // There should be multiple expressed interests due to retries.
  ASSERT_TRUE(fixture_.face_.sentInterests_.size() > 1);
}

static void
processInterestAsNetworkNack
  (const ndn::Interest& interest, const ndn::OnData& onData,
   const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack)
{
  NetworkNackLite networkNackLite;
  networkNackLite.setReason(ndn_NetworkNackReason_NO_ROUTE);
  ptr_lib::shared_ptr<NetworkNack> networkNack(new NetworkNack());
  networkNack->set(networkNackLite);

  onNetworkNack(ptr_lib::make_shared<Interest>(interest), networkNack);
}

TEST_F(TestValidator, NackedInterests)
{
  fixture_.face_.processInterest_ = &processInterestAsNetworkNack;

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  validateExpectFailure(data, "All interests should get NACKed");
  // There should be multiple expressed interests due to retries.
  ASSERT_TRUE(fixture_.face_.sentInterests_.size() > 1);
}

static void
processInterestWithCertificate
  (const ndn::Interest& interest, const ndn::OnData& onData,
   const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack,
   const ValidatorFixture::TestFace::ProcessInterest originalProcessInterest,
   const ptr_lib::shared_ptr<Data>& certificate)
{
  if (interest.getName().isPrefixOf(certificate->getName()))
    onData(ptr_lib::make_shared<Interest>(interest), certificate);
  else
    originalProcessInterest(interest, onData, onTimeout, onNetworkNack);
}

TEST_F(TestValidator, MalformedCertificate)
{
  // Copy the default certificate.
  ptr_lib::shared_ptr<Data> malformedCertificate(new Data
    (*fixture_.subIdentity_->getDefaultKey()->getDefaultCertificate()));
  malformedCertificate->getMetaInfo().setType(ndn_ContentType_BLOB);
  fixture_.keyChain_.sign(*malformedCertificate, SigningInfo(fixture_.identity_));
  // It has the wrong content type and a missing ValidityPeriod.
  ASSERT_THROW
    (CertificateV2(*malformedCertificate).wireEncode(), CertificateV2::Error);

  fixture_.face_.processInterest_ =
    bind(&processInterestWithCertificate, _1, _2, _3, _4,
         fixture_.face_.processInterest_, malformedCertificate);

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  validateExpectFailure(data, "Signed by a malformed certificate");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
}

TEST_F(TestValidator, ExpiredCertificate)
{
  // Copy the default certificate.
  ptr_lib::shared_ptr<Data> expiredCertificate(new Data
    (*fixture_.subIdentity_->getDefaultKey()->getDefaultCertificate()));
  SigningInfo info(fixture_.identity_);
  // Validity period from 2 hours ago do 1 hour ago.
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  info.setValidityPeriod
    (ValidityPeriod(now - 2 * 3600 * 1000, now - 3600 * 1000.0));
  fixture_.keyChain_.sign(*expiredCertificate, info);
  ASSERT_NO_THROW(CertificateV2(*expiredCertificate).wireEncode());

  fixture_.face_.processInterest_ =
    bind(&processInterestWithCertificate, _1, _2, _3, _4,
         fixture_.face_.processInterest_, expiredCertificate);

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  validateExpectFailure(data, "Signed by an expired certificate");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
}

TEST_F(TestValidator, ResetAnchors)
{
  fixture_.validator_.resetAnchors();

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));
  validateExpectFailure(data, "Should fail, as no anchors are configured");
}

TEST_F(TestValidator, TrustedCertificateCaching)
{
  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  validateExpectSuccess
    (data, "Should get accepted, as signed by the policy-compliant certificate");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  // Disable responses from the simulated Face.
  fixture_.face_.processInterest_ = 0;

  validateExpectSuccess
    (data, "Should get accepted, based on the cached trusted certificate");
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  // Make the trusted cache simulate a time 2 hours later, after expiration.
  fixture_.validator_.setCacheNowOffsetMilliseconds_(2 * 3600 * 1000.0);

  validateExpectFailure(data, "Should try and fail to retrieve certificates");
  // There should be multiple expressed interests due to retries.
  ASSERT_TRUE(fixture_.face_.sentInterests_.size() > 1);
  fixture_.face_.sentInterests_.clear();
}

TEST_F(TestValidator, ResetVerifiedCertificates)
{
  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));
  validateExpectSuccess
    (data, "Should get accepted, as signed by the policy-compliant certificate");

  // Reset the anchors.
  fixture_.validator_.resetAnchors();
  validateExpectSuccess
    (data, "Should get accepted, as signed by the certificate in the trusted cache");

  // Reset the trusted cache.
  fixture_.validator_.resetVerifiedCertificates();
  validateExpectFailure
    (data, "Should fail, as there is no trusted cache or anchors");
}

TEST_F(TestValidator, UntrustedCertificateCaching)
{
  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subSelfSignedIdentity_));

  validateExpectFailure
    (data, "Should fail, as signed by the policy-violating certificate");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  validateExpectFailure
    (data, "Should fail again, but no network operations are expected");
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  // Make the trusted cache simulate a time 20 minutes later, to expire the
  // untrusted cache (which has a lifetime of 5 minutes).
  fixture_.validator_.setCacheNowOffsetMilliseconds_(20 * 60 * 1000.0);

  // Disable responses from the simulated Face.
  fixture_.face_.processInterest_ = 0;

  validateExpectFailure(data, "Should try and fail to retrieve certificates");
  ASSERT_TRUE(fixture_.face_.sentInterests_.size() > 1);
  fixture_.face_.sentInterests_.clear();
}

TEST_F(TestValidator, InfiniteCertificateChain)
{
  fixture_.face_.processInterest_ =
    bind(&TestValidator::processInterestAsInfiniteCertificateChain,
         this, _1, _2, _3, _4);

  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));

  fixture_.validator_.setMaxDepth(40);
  ASSERT_EQ(40, fixture_.validator_.getMaxDepth());
  validateExpectFailure(data, "Should fail since the certificate should be looped");
  ASSERT_EQ(40, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  // Make the trusted cache simulate a time 5 hours later, after expiration.
  fixture_.validator_.setCacheNowOffsetMilliseconds_(5 * 3600 * 1000.0);

  fixture_.validator_.setMaxDepth(30);
  ASSERT_EQ(30, fixture_.validator_.getMaxDepth());
  validateExpectFailure(data, "Should fail since the certificate chain is infinite");
  ASSERT_EQ(30, fixture_.face_.sentInterests_.size());
}

TEST_F(TestValidator, LoopedCertificateChain)
{
  ptr_lib::shared_ptr<PibIdentity> identity1 = fixture_.addIdentity(Name("/loop"));
  ptr_lib::shared_ptr<PibKey> key1 =
    fixture_.keyChain_.createKey(*identity1, RsaKeyParams(Name::Component("key1")));
  ptr_lib::shared_ptr<PibKey> key2 =
    fixture_.keyChain_.createKey(*identity1, RsaKeyParams(Name::Component("key2")));
  ptr_lib::shared_ptr<PibKey> key3 =
    fixture_.keyChain_.createKey(*identity1, RsaKeyParams(Name::Component("key3")));

  makeCertificate(*key1, key2);
  makeCertificate(*key2, key3);
  makeCertificate(*key3, key1);

  Data data(Name("/loop/Data"));
  fixture_.keyChain_.sign(data, SigningInfo(key1));
  validateExpectFailure(data, "Should fail since the certificate chain loops");
  ASSERT_EQ(3, fixture_.face_.sentInterests_.size());
}

class ValidationPolicySimpleHierarchyForInterestOnly
  : public ValidationPolicySimpleHierarchy
{
public:
  void
  checkPolicy
    (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation)
  {
    continueValidation(ptr_lib::shared_ptr<CertificateRequest>(), state);
  }
};

class TestValidatorInterestOnly : public ::testing::Test {
public:
  TestValidatorInterestOnly()
  : fixture_(ptr_lib::make_shared<ValidationPolicySimpleHierarchyForInterestOnly>())
  {
  }

  /**
   * Call fixture_.validator_.validate and if it calls the failureCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test fails.
   */
  void
  validateExpectSuccess(const Data& data, const std::string& message)
  {
    fixture_.validator_.validate
      (data,
       bind(&TestValidatorInterestOnly::dataShouldSucceed, this, _1),
       bind(&TestValidatorInterestOnly::dataShouldNotFail, this, _1, _2, message));
  }

  /**
   * Call fixture_.validator_.validate and if it calls the failureCallback then
   * fail the test with the given message.
   * @param interest The Interest to validate.
   * @param message The message to show if the test fails.
   */
  void
  validateExpectSuccess(const Interest& interest, const std::string& message)
  {
    fixture_.validator_.validate
      (interest,
       bind(&TestValidatorInterestOnly::interestShouldSucceed, this, _1),
       bind(&TestValidatorInterestOnly::interestShouldNotFail, this, _1, _2, message));
  }

  /**
   * Call fixture_.validator_.validate and if it calls the successCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test succeeds.
   */
  void
  validateExpectFailure(const Data& data, const std::string& message)
  {
    fixture_.validator_.validate
      (data,
       bind(&TestValidatorInterestOnly::dataShouldNotSucceed, this, _1, message),
       bind(&TestValidatorInterestOnly::dataShouldFail, this, _1, _2));
  }

  /**
   * Call fixture_.validator_.validate and if it calls the successCallback then
   * fail the test with the given message.
   * @param interest The Interest to validate.
   * @param message The message to show if the test succeeds.
   */
  void
  validateExpectFailure(const Interest& interest, const std::string& message)
  {
    fixture_.validator_.validate
      (interest,
       bind(&TestValidatorInterestOnly::interestShouldNotSucceed, this, _1, message),
       bind(&TestValidatorInterestOnly::interestShouldFail, this, _1, _2));
  }

  void
  dataShouldSucceed(const Data& data)
  {
  }

  void
  dataShouldNotSucceed(const Data& data, const std::string& message)
  {
    FAIL() << message;
  }

  void
  dataShouldFail(const Data& data, const ValidationError& error)
  {
  }

  void
  dataShouldNotFail
    (const Data& data, const ValidationError& error,
     const std::string& message)
  {
    FAIL() << message;
  }

  void
  interestShouldSucceed(const Interest& interest)
  {
  }

  void
  interestShouldNotSucceed(const Interest& interest, const std::string& message)
  {
    FAIL() << message;
  }

  void
  interestShouldFail(const Interest& interest, const ValidationError& error)
  {
  }

  void
  interestShouldNotFail
    (const Interest& interest, const ValidationError& error,
     const std::string& message)
  {
    FAIL() << message;
  }

  HierarchicalValidatorFixture fixture_;
};

TEST_F(TestValidatorInterestOnly, ValidateInterestsButBypassForData)
{
  Interest interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  Data data(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));

  validateExpectFailure(interest, "Unsigned");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  interest = Interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  fixture_.keyChain_.sign
    (interest, SigningInfo(SigningInfo::SignerType::SIGNER_TYPE_SHA256));
  fixture_.keyChain_.sign
    (data, SigningInfo(SigningInfo::SignerType::SIGNER_TYPE_SHA256));
  validateExpectFailure(interest,
    "Required KeyLocator/Name is missing (not passed to the policy)");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  interest = Interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  fixture_.keyChain_.sign(interest, SigningInfo(fixture_.identity_));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.identity_));
  validateExpectSuccess(interest,
    "Should be successful since it is signed by the anchor");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  interest = Interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  fixture_.keyChain_.sign(interest, SigningInfo(fixture_.subIdentity_));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subIdentity_));
  validateExpectFailure(interest,
    "Should fail since the policy is not allowed to create new trust anchors");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  interest = Interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  fixture_.keyChain_.sign(interest, SigningInfo(fixture_.otherIdentity_));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.otherIdentity_));
  validateExpectFailure(interest,
    "Should fail since it is signed by a policy-violating certificate");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  // No network operations are expected since the certificate is not validated
  // by the policy.
  ASSERT_EQ(0, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();

  // Make the trusted cache simulate a time 2 hours later, after expiration.
  fixture_.validator_.setCacheNowOffsetMilliseconds_(2 * 3600 * 1000.0);

  interest = Interest(Name("/Security/V2/ValidatorFixture/Sub1/Sub2/Interest"));
  fixture_.keyChain_.sign(interest, SigningInfo(fixture_.subSelfSignedIdentity_));
  fixture_.keyChain_.sign(data, SigningInfo(fixture_.subSelfSignedIdentity_));
  validateExpectFailure(interest,
   "Should fail since the policy is not allowed to create new trust anchors");
  validateExpectSuccess(data, "The policy requests to bypass validation for all data");
  ASSERT_EQ(1, fixture_.face_.sentInterests_.size());
  fixture_.face_.sentInterests_.clear();
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
