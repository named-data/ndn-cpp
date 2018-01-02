/**
 * Copyright (C) 2014-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/validator-null.t.cpp
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
#include "identity-management-fixture.hpp"
#include <ndn-cpp/security/validator-null.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class TestValidatorNull : public ::testing::Test {
public:
  TestValidatorNull()
  {
  }

  void
  dataShouldSucceed(const Data& data)
  {
  }

  void
  dataShouldNotFail(const Data& data, const ValidationError& error)
  {
    FAIL() << "Validation should not have failed";
  }

  void
  interestShouldSucceed(const Interest& interest)
  {
  }

  void
  interestShouldNotFail(const Interest& interest, const ValidationError& error)
  {
    FAIL() << "Validation should not have failed";
  }

  IdentityManagementFixture fixture_;
};

TEST_F(TestValidatorNull, ValidateData)
{
  ptr_lib::shared_ptr<PibIdentity> identity =
    fixture_.addIdentity("/TestValidator/Null");
  Data data("/Some/Other/Data/Name");
  fixture_.keyChain_.sign(data, SigningInfo(identity));

  ValidatorNull validator;
  validator.validate
    (data, 
     bind(&TestValidatorNull::dataShouldSucceed, this, _1),
     bind(&TestValidatorNull::dataShouldNotFail, this, _1, _2));
}

TEST_F(TestValidatorNull, ValidateInterest)
{
  ptr_lib::shared_ptr<PibIdentity> identity =
    fixture_.addIdentity("/TestValidator/Null");
  Interest interest("/Some/Other/Interest/Name");
  fixture_.keyChain_.sign(interest, SigningInfo(identity));

  ValidatorNull validator;
  validator.validate
    (interest,
     bind(&TestValidatorNull::interestShouldSucceed, this, _1),
     bind(&TestValidatorNull::interestShouldNotFail, this, _1, _2));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
