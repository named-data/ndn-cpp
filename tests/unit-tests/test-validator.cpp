/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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
#include <ndn-cpp/security/v2/certificate-fetcher-offline.hpp>
#include <ndn-cpp/security/v2/validation-policy-simple-hierarchy.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class TestValidator : public ::testing::Test {
public:
  TestValidator()
  : fixture_(ptr_lib::make_shared<ValidationPolicySimpleHierarchy>(),
             ptr_lib::make_shared<CertificateFetcherOffline>())
  {
  }

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
  : fixture_(ptr_lib::make_shared<ValidationPolicySimpleHierarchyForInterestOnly>(),
             ptr_lib::make_shared<CertificateFetcherOffline>())
  {
  }

  HierarchicalValidatorFixture fixture_;
};

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
