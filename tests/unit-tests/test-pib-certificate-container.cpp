/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/certificate-container.t.cpp
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
#include <ndn-cpp/security/pib/pib-certificate-container.hpp>
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include "pib-data-fixture.hpp"

using namespace std;
using namespace ndn;

class TestPibCertificateContainer : public ::testing::Test {
public:
  PibDataFixture fixture;
};

TEST_F(TestPibCertificateContainer, Basic)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  // Start with an empty container.
  PibCertificateContainer container(fixture.id1Key1Name, pibImpl);
  ASSERT_EQ(0, container.size());
  ASSERT_EQ(0, container.certificates_.size());

  // Add a certificate.
  container.add(*fixture.id1Key1Cert1);
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.certificates_.size());
  ASSERT_TRUE(container.certificates_.find
    (fixture.id1Key1Cert1->getName()) != container.certificates_.end());

  // Add the same certificate again.
  container.add(*fixture.id1Key1Cert1);
  ASSERT_EQ(1, container.size());
  ASSERT_EQ(1, container.certificates_.size());
  ASSERT_TRUE(container.certificates_.find
    (fixture.id1Key1Cert1->getName()) != container.certificates_.end());

  // Add another certificate.
  container.add(*fixture.id1Key1Cert2);
  ASSERT_EQ(2, container.size());
  ASSERT_EQ(2, container.certificates_.size());
  ASSERT_TRUE(container.certificates_.find
    (fixture.id1Key1Cert1->getName()) != container.certificates_.end());
  ASSERT_TRUE(container.certificates_.find
    (fixture.id1Key1Cert2->getName()) != container.certificates_.end());

  // Get the certificates.
  ASSERT_NO_THROW(container.get(fixture.id1Key1Cert1->getName()));
  ASSERT_NO_THROW(container.get(fixture.id1Key1Cert2->getName()));
  Name id1Key1Cert3Name(fixture.id1Key1Name);
  id1Key1Cert3Name.append("issuer").appendVersion(3);
  ASSERT_THROW(container.get(id1Key1Cert3Name), Pib::Error);

  // Check the certificates.
  ptr_lib::shared_ptr<CertificateV2> cert1 = container.get
    (fixture.id1Key1Cert1->getName());
  ptr_lib::shared_ptr<CertificateV2> cert2 = container.get
    (fixture.id1Key1Cert2->getName());
  // Use the wire encoding to check equivalence.
  ASSERT_TRUE(cert1->wireEncode().equals(fixture.id1Key1Cert1->wireEncode()));
  ASSERT_TRUE(cert2->wireEncode().equals(fixture.id1Key1Cert2->wireEncode()));

  // Create another container with the same PibImpl. The cache should be empty.
  PibCertificateContainer container2(fixture.id1Key1Name, pibImpl);
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(0, container2.certificates_.size());

  // Get a certificate. The cache should be filled.
  ASSERT_NO_THROW(container2.get(fixture.id1Key1Cert1->getName()));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(1, container2.certificates_.size());

  ASSERT_NO_THROW(container2.get(fixture.id1Key1Cert2->getName()));
  ASSERT_EQ(2, container2.size());
  ASSERT_EQ(2, container2.certificates_.size());

  // Remove a certificate.
  container2.remove(fixture.id1Key1Cert1->getName());
  ASSERT_EQ(1, container2.size());
  ASSERT_EQ(1, container2.certificates_.size());
  ASSERT_TRUE(container2.certificates_.find(fixture.id1Key1Cert1->getName())
              == container2.certificates_.end());
  ASSERT_TRUE(container2.certificates_.find(fixture.id1Key1Cert2->getName())
              != container2.certificates_.end());

  // Remove another certificate.
  container2.remove(fixture.id1Key1Cert2->getName());
  ASSERT_EQ(0, container2.size());
  ASSERT_EQ(0, container2.certificates_.size());
  ASSERT_TRUE(container2.certificates_.find(fixture.id1Key1Cert2->getName())
              == container2.certificates_.end());
}

TEST_F(TestPibCertificateContainer, Errors)
{
  ptr_lib::shared_ptr<PibMemory> pibImpl(new PibMemory());

  PibCertificateContainer container(fixture.id1Key1Name, pibImpl);

  ASSERT_THROW(container.add(*fixture.id1Key2Cert1), std::invalid_argument);
  ASSERT_THROW(container.remove(fixture.id1Key2Cert1->getName()), std::invalid_argument);
  ASSERT_THROW(container.get(fixture.id1Key2Cert1->getName()), std::invalid_argument);
}

// Note: Don't test the Iterator because it's not implemented.

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

