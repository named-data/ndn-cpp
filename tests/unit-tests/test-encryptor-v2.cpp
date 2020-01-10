/**
 * Copyright (C) 2018-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/name-based-access-control/blob/new/tests/tests/encryptor.t.cpp
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
#if NDN_CPP_HAVE_TIME_H
#include <time.h>
#endif
#if NDN_CPP_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <algorithm>
#include <unistd.h>
#include <fstream>
#include <cstdio>
#include <ndn-cpp/encrypt/encryptor-v2.hpp>
#include <ndn-cpp/security/validator-null.hpp>
#include "identity-management-fixture.hpp"
#include "in-memory-storage-face.hpp"
#include "encrypt-static-data.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class EncryptorFixture : public IdentityManagementFixture {
public:
  EncryptorFixture(bool shouldPublishData, const EncryptError::OnError& onError)
  : face_(&storage_)
  {
    // Include the code here from the NAC unit-tests class
    // EncryptorStaticDataEnvironment instead of making it a base class.
    if (shouldPublishData)
      publishData();

    encryptor_ = ptr_lib::make_shared<EncryptorV2>
      (Name("/access/policy/identity/NAC/dataset"),
       Name("/some/ck/prefix"),
       SigningInfo(SigningInfo::SIGNER_TYPE_SHA256),
       onError, &validator_, &keyChain_, &face_);
  }

  void
  publishData()
  {
    for (size_t i = 0; i < EncryptStaticData::get().managerPackets.size(); ++i) {
      Data data;
      data.wireDecode(EncryptStaticData::get().managerPackets[i]);
      storage_.insert(data);
    }
  }

  InMemoryStorageRetaining storage_;
  InMemoryStorageFace face_;
  ValidatorNull validator_;
  ptr_lib::shared_ptr<EncryptorV2> encryptor_;
};

class TestEncryptorV2 : public ::testing::Test {
public:
  TestEncryptorV2()
  {
    fixture_ = ptr_lib::make_shared<EncryptorFixture>
      (true, TestEncryptorV2::onError);
  }

  static void
  onError(EncryptError::ErrorCode errorCode, const string& message)
  {
    FAIL() << "onError: " << message;
  }

  ptr_lib::shared_ptr<EncryptorFixture> fixture_;
};

TEST_F(TestEncryptorV2, EncryptAndPublishCk)
{
  fixture_->encryptor_->impl_->kekData_.reset();
  ASSERT_EQ(false, fixture_->encryptor_->impl_->isKekRetrievalInProgress_);
  fixture_->encryptor_->regenerateCk();
  // Unlike the ndn-group-encrypt unit tests, we don't check
  // isKekRetrievalInProgress_ true because we use a synchronous face which
  // finishes immediately.

  string message = "Data to encrypt";
  Blob plainText((const uint8_t*)message.c_str(), message.size());
  ptr_lib::shared_ptr<EncryptedContent> encryptedContent =
    fixture_->encryptor_->encrypt(plainText);

  Name ckPrefix = encryptedContent->getKeyLocatorName();
  ASSERT_TRUE(Name("/some/ck/prefix/CK").equals(ckPrefix.getPrefix(-1)));

  ASSERT_TRUE(encryptedContent->hasInitialVector());
  ASSERT_TRUE(!encryptedContent->getPayload().equals(plainText));

  // Check that the KEK Interest has been sent.
  ASSERT_TRUE(fixture_->face_.sentInterests_[0]->getName().getPrefix(6).equals
    (Name("/access/policy/identity/NAC/dataset/KEK")));

  ptr_lib::shared_ptr<Data> kekData = fixture_->face_.sentData_[0];
  ASSERT_TRUE(kekData->getName().getPrefix(6).equals
    (Name("/access/policy/identity/NAC/dataset/KEK")));
  ASSERT_EQ(7, kekData->getName().size());

  fixture_->face_.sentData_.clear();
  fixture_->face_.sentInterests_.clear();

  ptr_lib::shared_ptr<Interest> interest =
    ptr_lib::make_shared<Interest>(ckPrefix);
  interest->setCanBePrefix(true).setMustBeFresh(true);
  fixture_->face_.receive(interest);

  Name ckName = fixture_->face_.sentData_[0]->getName();
  ASSERT_TRUE(ckName.getPrefix(4).equals(Name("/some/ck/prefix/CK")));
  ASSERT_TRUE(ckName.get(5).equals(Name("/ENCRYPTED-BY")[0]));

  Name extractedKek = ckName.getSubName(6);
  ASSERT_TRUE(extractedKek.equals(kekData->getName()));

  ASSERT_EQ(false, fixture_->encryptor_->impl_->isKekRetrievalInProgress_);
}

TEST_F(TestEncryptorV2, KekRetrievalFailure)
{
  // Replace the default fixture.
  class Callbacks : public ptr_lib::enable_shared_from_this<Callbacks> {
  public:
    Callbacks()
    : nErrors_(0)
    {}

    void
    onError(EncryptError::ErrorCode errorCode, const string& message)
    {
      ++nErrors_;
    }

    int nErrors_;
  };
  ptr_lib::shared_ptr<Callbacks> callbacks = ptr_lib::make_shared<Callbacks>();
  fixture_ = ptr_lib::make_shared<EncryptorFixture>
    (false, bind(&Callbacks::onError, callbacks, _1, _2));

  string message = "Data to encrypt";
  Blob plainText((const uint8_t*)message.c_str(), message.size());
  ptr_lib::shared_ptr<EncryptedContent> encryptedContent =
    fixture_->encryptor_->encrypt(plainText);

  // Check that KEK interests has been sent.
  ASSERT_TRUE(fixture_->face_.sentInterests_[0]->getName().getPrefix(6).equals
    (Name("/access/policy/identity/NAC/dataset/KEK")));

  // ... and failed to retrieve.
  ASSERT_EQ(0, fixture_->face_.sentData_.size());

  ASSERT_EQ(1, callbacks->nErrors_);
  ASSERT_EQ(0, fixture_->face_.sentData_.size());

  // Check recovery.
  fixture_->publishData();

  fixture_->face_.delayedCallTable_.setNowOffsetMilliseconds_(73000);
  fixture_->face_.processEvents();

  ptr_lib::shared_ptr<Data> kekData = fixture_->face_.sentData_[0];
  ASSERT_TRUE(kekData->getName().getPrefix(6).equals
    (Name("/access/policy/identity/NAC/dataset/KEK")));
  ASSERT_EQ(7, kekData->getName().size());
}

TEST_F(TestEncryptorV2, EnumerateDataFromInMemoryStorage)
{
  usleep(200000);
  fixture_->encryptor_->regenerateCk();
  usleep(200000);
  fixture_->encryptor_->regenerateCk();

  ASSERT_EQ(3, fixture_->encryptor_->size());
  int nCk = 0;
  for (map<Name, ptr_lib::shared_ptr<Data> >::iterator i =
        fixture_->encryptor_->impl_->storage_.cache_.begin();
       i != fixture_->encryptor_->impl_->storage_.cache_.end(); ++i) {
    if (i->second->getName().getPrefix(4).equals(Name("/some/ck/prefix/CK")))
      ++nCk;
  }
  ASSERT_EQ(3, nCk);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

