/**
 * Copyright (C) 2014-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/validation-policy-command-interest.t.cpp
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
#include <ndn-cpp/encoding/tlv-wire-format.hpp>
#include <ndn-cpp/security/command-interest-signer.hpp>
#include <ndn-cpp/security/v2/validation-policy-simple-hierarchy.hpp>
#include <ndn-cpp/security/v2/validation-policy-command-interest.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class ValidationPolicyCommandInterestFixture : public HierarchicalValidatorFixture {
public:
  ValidationPolicyCommandInterestFixture
    (const ValidationPolicyCommandInterest::Options& options =
       ValidationPolicyCommandInterest::Options())
  : HierarchicalValidatorFixture(ptr_lib::make_shared<ValidationPolicyCommandInterest>
      (ptr_lib::make_shared<ValidationPolicySimpleHierarchy>(), options)),
    signer_(keyChain_)
  {}

  ptr_lib::shared_ptr<Interest>
  makeCommandInterest(const ptr_lib::shared_ptr<PibIdentity>& identity)
  {
    return signer_.makeCommandInterest
      (Name(identity->getName()).append("CMD"), SigningInfo(identity));
  }

  /**
   * Set the offset for the validation policy and signer.
   * @param nowOffsetMilliseconds The offset in milliseconds.
   */
  void
  setNowOffsetMilliseconds(Milliseconds nowOffsetMilliseconds)
  {
    dynamic_cast<ValidationPolicyCommandInterest&>
     (validator_.getPolicy()).setNowOffsetMilliseconds_
       (nowOffsetMilliseconds);
    validator_.setCacheNowOffsetMilliseconds_(nowOffsetMilliseconds);
    signer_.setNowOffsetMilliseconds_(nowOffsetMilliseconds);
  }

  CommandInterestSigner signer_;
};

class TestValidationPolicyCommandInterest : public ::testing::Test {
public:
  TestValidationPolicyCommandInterest()
  : fixture_(new ValidationPolicyCommandInterestFixture())
  {}

  /**
   * Call fixture_->validator_.validate and if it calls the failureCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test fails.
   */
  void
  validateExpectSuccess(const Data& data, const std::string& message)
  {
    fixture_->validator_.validate
      (data,
       bind(&TestValidationPolicyCommandInterest::dataShouldSucceed, this, _1),
       bind(&TestValidationPolicyCommandInterest::dataShouldNotFail, this, _1, _2, message));
  }

  /**
   * Call fixture_->validator_.validate and if it calls the failureCallback then
   * fail the test with the given message.
   * @param interest The Interest to validate.
   * @param message The message to show if the test fails.
   */
  void
  validateExpectSuccess(const Interest& interest, const std::string& message)
  {
    fixture_->validator_.validate
      (interest,
       bind(&TestValidationPolicyCommandInterest::interestShouldSucceed, this, _1),
       bind(&TestValidationPolicyCommandInterest::interestShouldNotFail, this, _1, _2, message));
  }

  /**
   * Call fixture_->validator_.validate and if it calls the successCallback then
   * fail the test with the given message.
   * @param data The Data to validate.
   * @param message The message to show if the test succeeds.
   */
  void
  validateExpectFailure(const Data& data, const std::string& message)
  {
    fixture_->validator_.validate
      (data,
       bind(&TestValidationPolicyCommandInterest::dataShouldNotSucceed, this, _1, message),
       bind(&TestValidationPolicyCommandInterest::dataShouldFail, this, _1, _2));
  }

  /**
   * Call fixture_->validator_.validate and if it calls the successCallback then
   * fail the test with the given message.
   * @param interest The Interest to validate.
   * @param message The message to show if the test succeeds.
   */
  void
  validateExpectFailure(const Interest& interest, const std::string& message)
  {
    fixture_->validator_.validate
      (interest,
       bind(&TestValidationPolicyCommandInterest::interestShouldNotSucceed, this, _1, message),
       bind(&TestValidationPolicyCommandInterest::interestShouldFail, this, _1, _2));
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

  static void
  setNameComponent(Interest& interest, int index, const Name::Component& component)
  {
    Name name = interest.getName().getPrefix(index);
    name.append(component);
    name.append(interest.getName().getSubName(name.size()));
    interest.setName(name);
  }

  ptr_lib::shared_ptr<ValidationPolicyCommandInterestFixture> fixture_;
};

TEST_F(TestValidationPolicyCommandInterest, Basic)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest1, "Should succeed (within grace period)");

  fixture_->setNowOffsetMilliseconds(5 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest2, "Should succeed (timestamp larger than previous)");
}

TEST_F(TestValidationPolicyCommandInterest, DataPassthrough)
{
  Data data1("/Security/V2/ValidatorFixture/Sub1");
  fixture_->keyChain_.sign(data1);
  validateExpectSuccess(data1,
    "Should succeed (fallback on inner validation policy for data)");
}

TEST_F(TestValidationPolicyCommandInterest, NameTooShort)
{
  Interest interest1(Name("/name/too/short"));
  validateExpectFailure(interest1, "Should fail (name is too short)");
}

TEST_F(TestValidationPolicyCommandInterest, BadSignatureInfo)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  setNameComponent
    (*interest1, CommandInterestSigner::POS_SIGNATURE_INFO, "not-SignatureInfo");
  validateExpectFailure(*interest1, "Should fail (missing signature info)");
}

TEST_F(TestValidationPolicyCommandInterest, MissingKeyLocator)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  Sha256WithRsaSignature signatureInfo;
  setNameComponent
    (*interest1, CommandInterestSigner::POS_SIGNATURE_INFO,
     TlvWireFormat::get()->encodeSignatureInfo(signatureInfo));
  validateExpectFailure(*interest1, "Should fail (missing KeyLocator)");
}

TEST_F(TestValidationPolicyCommandInterest, BadKeyLocatorType)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  KeyLocator keyLocator;
  keyLocator.setType(ndn_KeyLocatorType_KEY_LOCATOR_DIGEST);
  keyLocator.setKeyData(Blob((const uint8_t*)"\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD", 8));
  Sha256WithRsaSignature signatureInfo;
  signatureInfo.setKeyLocator(keyLocator);

  setNameComponent
    (*interest1, CommandInterestSigner::POS_SIGNATURE_INFO,
     TlvWireFormat::get()->encodeSignatureInfo(signatureInfo));
  validateExpectFailure(*interest1, "Should fail (bad KeyLocator type)");
}

TEST_F(TestValidationPolicyCommandInterest, BadCertificateName)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  KeyLocator keyLocator;
  keyLocator.setType(ndn_KeyLocatorType_KEYNAME);
  keyLocator.setKeyName(Name("/bad/cert/name"));
  Sha256WithRsaSignature signatureInfo;
  signatureInfo.setKeyLocator(keyLocator);

  setNameComponent
    (*interest1, CommandInterestSigner::POS_SIGNATURE_INFO,
     TlvWireFormat::get()->encodeSignatureInfo(signatureInfo));
  validateExpectFailure(*interest1, "Should fail (bad certificate name)");
}

TEST_F(TestValidationPolicyCommandInterest, InnerPolicyReject)
{
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->otherIdentity_);
  validateExpectFailure(*interest1, "Should fail (inner policy should reject)");
}

TEST_F(TestValidationPolicyCommandInterest, TimestampOutOfGracePositive)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0)));

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Verifying at +16 seconds.
  fixture_->setNowOffsetMilliseconds(16 * 1000.0);
  validateExpectFailure(*interest1,
    "Should fail (timestamp outside the grace period)");

  // Signed at +16 seconds.
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest2, "Should succeed");
}

TEST_F(TestValidationPolicyCommandInterest, TimestampOutOfGraceNegative)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0)));

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +1 seconds.
  fixture_->setNowOffsetMilliseconds(1 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +2 seconds.
  fixture_->setNowOffsetMilliseconds(2 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest3 = fixture_->makeCommandInterest
    (fixture_->identity_);

  // Verifying at -16 seconds.
  fixture_->setNowOffsetMilliseconds(-16 * 1000.0);
  validateExpectFailure(*interest1,
    "Should fail (timestamp outside the grace period)");

  // The CommandInterestValidator should not remember interest1's timestamp.
  validateExpectFailure(*interest2,
    "Should fail (timestamp outside the grace period)");

  // The CommandInterestValidator should not remember interest2's timestamp, and
  // should treat interest3 as initial.
  // Verifying at +2 seconds.
  fixture_->setNowOffsetMilliseconds(2 * 1000.0);
  validateExpectSuccess(*interest3, "Should succeed");
}

TEST_F(TestValidationPolicyCommandInterest, TimestampReorderEqual)
{
  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest1, "Should succeed");

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  setNameComponent
    (*interest2, CommandInterestSigner::POS_TIMESTAMP,
     interest1->getName().get(CommandInterestSigner::POS_TIMESTAMP));
  validateExpectFailure(*interest2, "Should fail (timestamp reordered)");

  // Signed at +2 seconds.
  fixture_->setNowOffsetMilliseconds(2 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest3 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest3, "Should succeed");
}

TEST_F(TestValidationPolicyCommandInterest, TimestampReorderNegative)
{
  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +200 milliseconds.
  fixture_->setNowOffsetMilliseconds(200.0);
  ptr_lib::shared_ptr<Interest> interest3 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +1100 milliseconds.
  fixture_->setNowOffsetMilliseconds(1100.0);
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +1400 milliseconds.
  fixture_->setNowOffsetMilliseconds(1400.0);
  ptr_lib::shared_ptr<Interest> interest4 = fixture_->makeCommandInterest
    (fixture_->identity_);

  // Verifying at +1100 milliseconds.
  fixture_->setNowOffsetMilliseconds(1100.0);
  validateExpectSuccess(*interest1, "Should succeed");

  // Verifying at 0 milliseconds.
  fixture_->setNowOffsetMilliseconds(0.0);
  validateExpectFailure(*interest2, "Should fail (timestamp reordered)");

  // The CommandInterestValidator should not remember interest2's timestamp.
  // Verifying at +200 milliseconds.
  fixture_->setNowOffsetMilliseconds(200.0);
  validateExpectFailure(*interest3, "Should fail (timestamp reordered)");

  // Verifying at +1400 milliseconds.
  fixture_->setNowOffsetMilliseconds(1400.0);
  validateExpectSuccess(*interest4, "Should succeed");
}

TEST_F(TestValidationPolicyCommandInterest, LimitedRecords)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0, 3)));

  ptr_lib::shared_ptr<PibIdentity> identity1 = fixture_->addSubCertificate
    ("/Security/V2/ValidatorFixture/Sub1", fixture_->identity_);
  fixture_->cache_.insert(*identity1->getDefaultKey()->getDefaultCertificate());
  ptr_lib::shared_ptr<PibIdentity> identity2 = fixture_->addSubCertificate
    ("/Security/V2/ValidatorFixture/Sub2", fixture_->identity_);
  fixture_->cache_.insert(*identity2->getDefaultKey()->getDefaultCertificate());
  ptr_lib::shared_ptr<PibIdentity> identity3 = fixture_->addSubCertificate
    ("/Security/V2/ValidatorFixture/Sub3", fixture_->identity_);
  fixture_->cache_.insert(*identity3->getDefaultKey()->getDefaultCertificate());
  ptr_lib::shared_ptr<PibIdentity> identity4 = fixture_->addSubCertificate
    ("/Security/V2/ValidatorFixture/Sub4", fixture_->identity_);
  fixture_->cache_.insert(*identity4->getDefaultKey()->getDefaultCertificate());

  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (identity2);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (identity3);
  ptr_lib::shared_ptr<Interest> interest3 = fixture_->makeCommandInterest
    (identity4);
  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest00 = fixture_->makeCommandInterest
    (identity1);
  // Signed at +1 seconds.
  fixture_->setNowOffsetMilliseconds(1 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest01 = fixture_->makeCommandInterest
    (identity1);
  // Signed at +2 seconds.
  fixture_->setNowOffsetMilliseconds(2 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest02 = fixture_->makeCommandInterest
    (identity1);

  validateExpectSuccess(*interest00, "Should succeed");

  validateExpectSuccess(*interest02, "Should succeed");

  validateExpectSuccess(*interest1, "Should succeed");

  validateExpectSuccess(*interest2, "Should succeed");

  validateExpectSuccess(*interest3, "Should succeed, forgets identity1");

  validateExpectSuccess(*interest01,
    "Should succeed despite timestamp is reordered, because the record has been evicted");
}

TEST_F(TestValidationPolicyCommandInterest, UnlimitedRecords)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0, -1)));

  vector<ptr_lib::shared_ptr<PibIdentity> > identities;
  for (int i = 0; i < 20; ++i) {
    ptr_lib::shared_ptr<PibIdentity> identity = fixture_->addSubCertificate
      ("/Security/V2/ValidatorFixture/Sub" + to_string(i), fixture_->identity_);
    fixture_->cache_.insert(*identity->getDefaultKey()->getDefaultCertificate());
    identities.push_back(identity);
  }

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (identities[0]);
  fixture_->setNowOffsetMilliseconds(1 * 1000.0);
  for (int i = 0; i < 20; ++i) {
    // Signed at +1 seconds.
    ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
      (identities[i]);

    validateExpectSuccess(*interest2, "Should succeed");
  }

  validateExpectFailure(*interest1, "Should fail (timestamp reorder)");
}

TEST_F(TestValidationPolicyCommandInterest, ZeroRecords)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0, 0)));

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +1 seconds.
  fixture_->setNowOffsetMilliseconds(1 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest2, "Should succeed");

  validateExpectSuccess(*interest1,
    "Should succeed despite the timestamp being reordered, because the record isn't kept");
}

TEST_F(TestValidationPolicyCommandInterest, LimitedRecordLifetime)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(400 * 1000.0, 1000, 300 * 1000.0)));

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +240 seconds.
  fixture_->setNowOffsetMilliseconds(240 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +360 seconds.
  fixture_->setNowOffsetMilliseconds(360 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest3 = fixture_->makeCommandInterest
    (fixture_->identity_);

  // Validate at 0 seconds.
  fixture_->setNowOffsetMilliseconds(0.0);
  validateExpectSuccess(*interest1, "Should succeed");

  validateExpectSuccess(*interest3, "Should succeed");

  // Validate at +301 seconds.
  fixture_->setNowOffsetMilliseconds(301 * 1000.0);
  validateExpectSuccess(*interest2,
    "Should succeed despite the timestamp being reordered, because the record has expired");
}

TEST_F(TestValidationPolicyCommandInterest, ZeroRecordLifetime)
{
  fixture_.reset(new ValidationPolicyCommandInterestFixture
    (ValidationPolicyCommandInterest::Options(15 * 1000.0, 1000, 0.0)));

  // Signed at 0 seconds.
  ptr_lib::shared_ptr<Interest> interest1 = fixture_->makeCommandInterest
    (fixture_->identity_);
  // Signed at +1 second.
  fixture_->setNowOffsetMilliseconds(1 * 1000.0);
  ptr_lib::shared_ptr<Interest> interest2 = fixture_->makeCommandInterest
    (fixture_->identity_);
  validateExpectSuccess(*interest2, "Should succeed");

  validateExpectSuccess(*interest1,
    "Should succeed despite the timestamp being reordered, because the record has expired");
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
