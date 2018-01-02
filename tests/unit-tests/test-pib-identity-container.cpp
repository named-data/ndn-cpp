/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/identity-container.t.cpp
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
#include <ndn-cpp/security/pib/pib-identity-container.hpp>
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class TestPibIdentityContainer : public ::testing::Test {
public:
  PibDataFixture fixture;
};

TEST_F(TestPibIdentityContainer, Basic)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  // Start with an empty container.
  PibIdentityContainer container(pibImpl);
  ASSERT_EQ(0, container.size());
  ASSERT_EQ(0, container.identities_.size());

  // Add the first identity.
  ptr_lib::shared_ptr<PibIdentity> identity11 = container.add(fixture.id1);
  ASSERT_EQ(fixture.id1, identity11->getName());
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.identities_.size());
  ASSERT_TRUE
    (container.identities_.find(fixture.id1) != container.identities_.end());

  // Add the same identity again.
  ptr_lib::shared_ptr<PibIdentity> identity12 = container.add(fixture.id1);
  ASSERT_EQ(fixture.id1, identity12->getName());
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.identities_.size());
  ASSERT_TRUE
    (container.identities_.find(fixture.id1) != container.identities_.end());

  // Add the second identity.
  ptr_lib::shared_ptr<PibIdentity> identity21 = container.add(fixture.id2);
  ASSERT_EQ(fixture.id2, identity21->getName());
  ASSERT_EQ(2, container.size());
  ASSERT_EQ(2, container.identities_.size());
  ASSERT_TRUE
    (container.identities_.find(fixture.id1) != container.identities_.end());
  ASSERT_TRUE
    (container.identities_.find(fixture.id2) != container.identities_.end());

  // Get identities.
  ASSERT_NO_THROW(container.get(fixture.id1));
  ASSERT_NO_THROW(container.get(fixture.id2));
  ASSERT_THROW(container.get(Name("/non-existing")), Pib::Error);

  // Check the identity.
  ptr_lib::shared_ptr<PibIdentity> identity1 = container.get(fixture.id1);
  ptr_lib::shared_ptr<PibIdentity> identity2 = container.get(fixture.id2);
  ASSERT_EQ(fixture.id1, identity1->getName());
  ASSERT_EQ(fixture.id2, identity2->getName());

  // Create another container from the same PibImpl. The cache should be empty.
  PibIdentityContainer container2(pibImpl);
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(0, container2.identities_.size());

  // Get keys. The cache should be filled.
  ASSERT_NO_THROW(container2.get(fixture.id1));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(1, container2.identities_.size());

  ASSERT_NO_THROW(container2.get(fixture.id2));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(2, container2.identities_.size());

  // Remove a key.
  container2.remove(fixture.id1);
  ASSERT_EQ(1, container2.size());
  ASSERT_EQ(1, container2.identities_.size());
  ASSERT_TRUE
    (container2.identities_.find(fixture.id1) == container2.identities_.end());
  ASSERT_TRUE
    (container2.identities_.find(fixture.id2) != container2.identities_.end());

  // Remove another key.
  container2.remove(fixture.id2);
  ASSERT_EQ(0, container2.size());
  ASSERT_EQ(0, container2.identities_.size());
  ASSERT_TRUE
    (container2.identities_.find(fixture.id2) == container2.identities_.end());
}

// Note: Don't test the Iterator because it's not implemented.

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

