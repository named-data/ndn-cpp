/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/key-container.t.cpp
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
#include <ndn-cpp/security/pib/pib-key-container.hpp>
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class TestPibKeyContainer : public ::testing::Test {
public:
  PibDataFixture fixture;
};

TEST_F(TestPibKeyContainer, Basic)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  // Start with an empty container.
  PibKeyContainer container(fixture.id1, pibImpl);
  ASSERT_EQ(0, container.size());
  ASSERT_EQ(0, container.keys_.size());

  // Add the first key.
  ptr_lib::shared_ptr<PibKey> key11 = container.add
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name);
  ASSERT_EQ(fixture.id1Key1Name, key11->getName());
  ASSERT_TRUE(key11->getPublicKey().equals(fixture.id1Key1));
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.keys_.size());
  ASSERT_TRUE(container.keys_.find(fixture.id1Key1Name) != container.keys_.end());

  // Add the same key again.
  ptr_lib::shared_ptr<PibKey> key12 = container.add
    (fixture.id1Key1.buf(), fixture.id1Key1.size(), fixture.id1Key1Name);
  ASSERT_EQ(fixture.id1Key1Name, key12->getName());
  ASSERT_TRUE(key12->getPublicKey().equals(fixture.id1Key1));
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.keys_.size());
  ASSERT_TRUE(container.keys_.find(fixture.id1Key1Name) != container.keys_.end());

  // Add the second key.
  ptr_lib::shared_ptr<PibKey> key21 = container.add
    (fixture.id1Key2.buf(), fixture.id1Key2.size(), fixture.id1Key2Name);
  ASSERT_EQ(fixture.id1Key2Name, key21->getName());
  ASSERT_TRUE(key21->getPublicKey().equals(fixture.id1Key2));
  ASSERT_EQ(2, container.size());
  ASSERT_EQ(2, container.keys_.size());
  ASSERT_TRUE(container.keys_.find(fixture.id1Key1Name) != container.keys_.end());
  ASSERT_TRUE(container.keys_.find(fixture.id1Key2Name) != container.keys_.end());

  // Get keys.
  ASSERT_NO_THROW(container.get(fixture.id1Key1Name));
  ASSERT_NO_THROW(container.get(fixture.id1Key2Name));
  Name id1Key3Name = PibKey::constructKeyName
    (fixture.id1, Name::Component("non-existing-id"));
  ASSERT_THROW(container.get(id1Key3Name), Pib::Error);

  // Get and check keys.
  ptr_lib::shared_ptr<PibKey> key1 = container.get(fixture.id1Key1Name);
  ptr_lib::shared_ptr<PibKey> key2 = container.get(fixture.id1Key2Name);
  ASSERT_EQ(fixture.id1Key1Name, key1->getName());
  ASSERT_TRUE(key1->getPublicKey().equals(fixture.id1Key1));
  ASSERT_EQ(fixture.id1Key2Name, key2->getName());
  ASSERT_TRUE(key2->getPublicKey().equals(fixture.id1Key2));

  // Create another container using the same PibImpl. The cache should be empty.
  PibKeyContainer container2(fixture.id1, pibImpl);
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(0, container2.keys_.size());

  // Get a key. The cache should be filled.
  ASSERT_NO_THROW(container2.get(fixture.id1Key1Name));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(1, container2.keys_.size());

  ASSERT_NO_THROW(container2.get(fixture.id1Key2Name));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(2, container2.keys_.size());

  // Remove a key.
  container2.remove(fixture.id1Key1Name);
  ASSERT_EQ(1, container2.size());
  ASSERT_EQ(1, container2.keys_.size());
  ASSERT_TRUE(container2.keys_.find(fixture.id1Key1Name) == container2.keys_.end());
  ASSERT_TRUE(container2.keys_.find(fixture.id1Key2Name) != container2.keys_.end());

  // Remove another key.
  container2.remove(fixture.id1Key2Name);
  ASSERT_EQ(0, container2.size());
  ASSERT_EQ(0, container2.keys_.size());
  ASSERT_TRUE(container2.keys_.find(fixture.id1Key2Name) == container2.keys_.end());
}

TEST_F(TestPibKeyContainer, Errors)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  PibKeyContainer container(fixture.id1, pibImpl);

  ASSERT_THROW(container.add
    (fixture.id2Key1.buf(), fixture.id2Key1.size(), fixture.id2Key1Name),
     std::invalid_argument);
  ASSERT_THROW(container.remove(fixture.id2Key1Name), std::invalid_argument);
  ASSERT_THROW(container.get(fixture.id2Key1Name), std::invalid_argument);
}

// Note: Don't test the Iterator because it's not implemented.

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

