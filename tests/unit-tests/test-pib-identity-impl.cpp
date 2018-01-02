/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/detail/identity-impl.t.cpp
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
#include "../../src/security/pib/detail/pib-identity-impl.hpp"
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class TestPibIdentityImpl : public ::testing::Test {
public:
  PibDataFixture fixture;
};

TEST_F(TestPibIdentityImpl, Basic)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());
  PibIdentityImpl identity1(fixture.id1, pibImpl, true);

  ASSERT_EQ(fixture.id1, identity1.getName());
}

TEST_F(TestPibIdentityImpl, KeyOperation)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());
  PibIdentityImpl identity1(fixture.id1, pibImpl, true);
  ASSERT_NO_THROW(PibIdentityImpl(fixture.id1, pibImpl, false));

  // The identity should not have any key.
  ASSERT_EQ(0, identity1.keys_.size());

  // Getting non-existing key should throw Pib::Error.
  ASSERT_THROW(identity1.getKey(fixture.id1Key1Name), Pib::Error);
  // Getting the default key should throw Pib::Error.
  ASSERT_THROW(identity1.getDefaultKey(), Pib::Error);
  // Setting a non-existing key as the default key should throw Pib::Error.
  ASSERT_THROW(identity1.setDefaultKey(fixture.id1Key1Name), Pib::Error);

  // Add a key.
  identity1.addKey
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name);
  ASSERT_NO_THROW(identity1.getKey(fixture.id1Key1Name));

  // A new key should become the default key when there is no default.
  ASSERT_NO_THROW(identity1.getDefaultKey());
  const ptr_lib::shared_ptr<PibKey>& defaultKey0 = identity1.getDefaultKey();
  ASSERT_EQ(fixture.id1Key1Name, defaultKey0->getName());
  ASSERT_TRUE(defaultKey0->getPublicKey().equals(fixture.id1Key1));

  // Remove a key.
  identity1.removeKey(fixture.id1Key1Name);
  ASSERT_THROW(identity1.getKey(fixture.id1Key1Name), Pib::Error);
  ASSERT_THROW(identity1.getDefaultKey(), Pib::Error);

  // Set the default key directly.
  ASSERT_NO_THROW(identity1.setDefaultKey
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name));
  ASSERT_NO_THROW(identity1.getDefaultKey());
  ASSERT_NO_THROW(identity1.getKey(fixture.id1Key1Name));

  // Check for a default key.
  const ptr_lib::shared_ptr<PibKey>& defaultKey1 = identity1.getDefaultKey();
  ASSERT_EQ(fixture.id1Key1Name, defaultKey1->getName());
  ASSERT_TRUE(defaultKey1->getPublicKey().equals(fixture.id1Key1));

  // Add another key.
  identity1.addKey
    (fixture.id1Key2.buf(), fixture.id1Key2.size(), fixture.id1Key2Name);
  ASSERT_EQ(2, identity1.keys_.size());

  // Set the default key using a name.
  ASSERT_NO_THROW(identity1.setDefaultKey(fixture.id1Key2Name));
  ASSERT_NO_THROW(identity1.getDefaultKey());
  const ptr_lib::shared_ptr<PibKey>& defaultKey2 = identity1.getDefaultKey();
  ASSERT_EQ(fixture.id1Key2Name, defaultKey2->getName());
  ASSERT_TRUE(defaultKey2->getPublicKey().equals(fixture.id1Key2));

  // Remove a key.
  identity1.removeKey(fixture.id1Key1Name);
  ASSERT_THROW(identity1.getKey(fixture.id1Key1Name), Pib::Error);
  ASSERT_EQ(1, identity1.keys_.size());

  // Seting the default key directly again should change the default.
  ASSERT_NO_THROW(identity1.setDefaultKey
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name));
  const ptr_lib::shared_ptr<PibKey>& defaultKey3 = identity1.getDefaultKey();
  ASSERT_EQ(fixture.id1Key1Name, defaultKey3->getName());
  ASSERT_TRUE(defaultKey3->getPublicKey().equals(fixture.id1Key1));
  ASSERT_EQ(2, identity1.keys_.size());

  // Remove all keys.
  identity1.removeKey(fixture.id1Key1Name);
  ASSERT_THROW(identity1.getKey(fixture.id1Key1Name), Pib::Error);
  ASSERT_EQ(1, identity1.keys_.size());
  identity1.removeKey(fixture.id1Key2Name);
  ASSERT_THROW(identity1.getKey(fixture.id1Key2Name), Pib::Error);
  ASSERT_EQ(0, identity1.keys_.size());
  ASSERT_THROW(identity1.getDefaultKey(), Pib::Error);
}

TEST_F(TestPibIdentityImpl, Overwrite)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());
  PibIdentityImpl identity1(fixture.id1, pibImpl, true);

  identity1.addKey
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name);
  ASSERT_TRUE(identity1.getKey(fixture.id1Key1Name)->getPublicKey().equals
              (fixture.id1Key1));

  // Overwriting the key should work.
  identity1.addKey
    (fixture.id1Key2.buf(), fixture.id1Key2.size(), fixture.id1Key1Name);
  ASSERT_TRUE(identity1.getKey(fixture.id1Key1Name)->getPublicKey().equals
              (fixture.id1Key2));
}

TEST_F(TestPibIdentityImpl, Errors)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  ASSERT_THROW(PibIdentityImpl(fixture.id1, pibImpl, false), Pib::Error);
  PibIdentityImpl identity1(fixture.id1, pibImpl, true);

  identity1.addKey
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name);
  ASSERT_THROW(identity1.addKey
    (fixture.id2Key1.buf(), fixture.id2Key1.size(), fixture.id2Key1Name),
    invalid_argument);
  ASSERT_THROW(identity1.removeKey(fixture.id2Key1Name), invalid_argument);
  ASSERT_THROW(identity1.getKey(fixture.id2Key1Name), invalid_argument);
  ASSERT_THROW(identity1.setDefaultKey
    (fixture.id2Key1.buf(), fixture.id2Key1.size(), fixture.id2Key1Name),
     invalid_argument);
  ASSERT_THROW(identity1.setDefaultKey(fixture.id2Key1Name), invalid_argument);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

