/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/name-based-access-control/blob/new/tests/tests/access-manager.t.cpp
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
#include <cstdio>
#include <ndn-cpp/security/validator-null.hpp>
#include <ndn-cpp/encrypt/encryptor-v2.hpp>
#include <ndn-cpp/encrypt/access-manager-v2.hpp>
#include "identity-management-fixture.hpp"
#include "in-memory-storage-face.hpp"
#include "encrypt-static-data.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class AccessManagerFixture : public IdentityManagementFixture {
public:
  AccessManagerFixture()
  : face_(&storage_)
  {
    accessIdentity_ = addIdentity(Name("/access/policy/identity"));
    // This is a hack to get access to the KEK key-id.
    nacIdentity_ = addIdentity
      (Name("/access/policy/identity/NAC/dataset"), RsaKeyParams());
    userIdentities_.push_back(addIdentity(Name("/first/user"), RsaKeyParams()));
    userIdentities_.push_back(addIdentity(Name("/second/user"), RsaKeyParams()));

    manager_ = ptr_lib::make_shared<AccessManagerV2>
      (accessIdentity_, Name("/dataset"), &keyChain_, &face_);

    for (size_t i = 0; i < userIdentities_.size(); ++i)
      manager_->addMember
        (*userIdentities_[i]->getDefaultKey()->getDefaultCertificate());
  }

  InMemoryStorageRetaining storage_;
  InMemoryStorageFace face_;
  ptr_lib::shared_ptr<PibIdentity> accessIdentity_;
  ptr_lib::shared_ptr<PibIdentity> nacIdentity_;
  vector<ptr_lib::shared_ptr<PibIdentity> > userIdentities_;
  ptr_lib::shared_ptr<AccessManagerV2> manager_;
};

class TestAccessManagerV2 : public ::testing::Test {
public:
  AccessManagerFixture fixture_;
};

TEST_F(TestAccessManagerV2, PublishedKek)
{
  ptr_lib::shared_ptr<Interest> interest = ptr_lib::make_shared<Interest>
    (Name("/access/policy/identity/NAC/dataset/KEK"));
  interest->setCanBePrefix(true).setMustBeFresh(true);
  fixture_.face_.receive(interest);

  ASSERT_TRUE(fixture_.face_.sentData_[0]->getName().getPrefix(-1).equals
    (Name("/access/policy/identity/NAC/dataset/KEK")));
  ASSERT_TRUE(fixture_.face_.sentData_[0]->getName().get(-1).equals
    (fixture_.nacIdentity_->getDefaultKey()->getName().get(-1)));
}

TEST_F(TestAccessManagerV2, PublishedKdks)
{
  for (size_t i = 0; i < fixture_.userIdentities_.size(); ++i) {
    Name kdkName("/access/policy/identity/NAC/dataset/KDK");
    kdkName
      .append(fixture_.nacIdentity_->getDefaultKey()->getName().get(-1))
      .append("ENCRYPTED-BY")
      .append(fixture_.userIdentities_[i]->getDefaultKey()->getName());

    ptr_lib::shared_ptr<Interest> interest = ptr_lib::make_shared<Interest>
      (kdkName);
    interest->setCanBePrefix(true).setMustBeFresh(true);
    fixture_.face_.receive(interest);

    ASSERT_TRUE(fixture_.face_.sentData_[0]->getName().equals(kdkName)) <<
      "Sent Data does not have the KDK name " + kdkName.toUri();
    fixture_.face_.sentData_.clear();
  }
}

TEST_F(TestAccessManagerV2, EnumerateDataFromInMemoryStorage)
{
  ASSERT_EQ(3, fixture_.manager_->size());

  int nKek = 0;
  int nKdk = 0;
  for (map<Name, ptr_lib::shared_ptr<Data> >::iterator i =
        fixture_.manager_->impl_->storage_.cache_.begin();
       i != fixture_.manager_->impl_->storage_.cache_.end(); ++i) {
    if (i->second->getName().get(5).equals(EncryptorV2::getNAME_COMPONENT_KEK()))
      ++nKek;
    if (i->second->getName().get(5).equals(EncryptorV2::getNAME_COMPONENT_KDK()))
      ++nKdk;
  }

  ASSERT_EQ(1, nKek);
  ASSERT_EQ(2, nKdk);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

