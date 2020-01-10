/**
 * Copyright (C) 2018-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/name-based-access-control/blob/new/tests/tests/decryptor.t.cpp
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
#include <ndn-cpp/encrypt/decryptor-v2.hpp>
#include "identity-management-fixture.hpp"
#include "in-memory-storage-face.hpp"
#include "encrypt-static-data.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

class DecryptorFixture : public IdentityManagementFixture {
public:
  DecryptorFixture(const Name& identityName)
  : face_(&storage_)
  {
    // Include the code here from the NAC unit-tests class
    // DecryptorStaticDataEnvironment instead of making it a base class.
    for (size_t i = 0; i < EncryptStaticData::get().managerPackets.size(); ++i) {
      Data data;
      data.wireDecode(EncryptStaticData::get().managerPackets[i]);
      storage_.insert(data);
    }

    for (size_t i = 0; i < EncryptStaticData::get().encryptorPackets.size(); ++i) {
      Data data;
      data.wireDecode(EncryptStaticData::get().encryptorPackets[i]);
      storage_.insert(data);
    }

    // Import the "/first/user" identity.
    string password = "password";
    keyChain_.importSafeBag
      (SafeBag(EncryptStaticData::get().userIdentity),
       (const uint8_t*)password.c_str(), password.size());

    addIdentity(Name("/not/authorized"));

    decryptor_ = ptr_lib::make_shared<DecryptorV2>
      (keyChain_.getPib().getIdentity(identityName)->getDefaultKey().get(),
       &validator_, &keyChain_, &face_);
  }

  InMemoryStorageRetaining storage_;
  InMemoryStorageFace face_;
  ValidatorNull validator_;
  ptr_lib::shared_ptr<DecryptorV2> decryptor_;
};

class TestDecryptorV2 : public ::testing::Test {
};

TEST_F(TestDecryptorV2, DecryptValid)
{
  DecryptorFixture fixture(Name("/first/user"));

  ptr_lib::shared_ptr<EncryptedContent> encryptedContent =
    ptr_lib::make_shared<EncryptedContent>();
  encryptedContent->wireDecodeV2(EncryptStaticData::get().encryptedBlobs[0]);

  class Callbacks {
  public:
    Callbacks()
    : nSuccesses_(0), nFailures_(0)
    {}

    void
    onSuccess(const Blob& plainData)
    {
      ++nSuccesses_;
      ASSERT_EQ(15, plainData.size());
      string text = "Data to encrypt";
      ASSERT_TRUE(plainData.equals(Blob((const uint8_t*)text.c_str(), text.size())));
    }

    void
    onError(EncryptError::ErrorCode errorCode, const string& message)
    {
      ++nFailures_;
    }

    int nSuccesses_;
    int nFailures_;
  };

  Callbacks callbacks;
  // Database and crypto operations are synchronous, so this returns immediately.
  fixture.decryptor_->decrypt
    (encryptedContent,
     bind(&Callbacks::onSuccess, &callbacks, _1),
     bind(&Callbacks::onError, &callbacks, _1, _2));

  ASSERT_EQ(1, callbacks.nSuccesses_);
  ASSERT_EQ(0, callbacks.nFailures_);
}

TEST_F(TestDecryptorV2, DecryptInvalid)
{
  DecryptorFixture fixture(Name("/not/authorized"));

  ptr_lib::shared_ptr<EncryptedContent> encryptedContent =
    ptr_lib::make_shared<EncryptedContent>();
  encryptedContent->wireDecodeV2(EncryptStaticData::get().encryptedBlobs[0]);

  class Callbacks {
  public:
    Callbacks()
    : nSuccesses_(0), nFailures_(0)
    {}

    void
    onSuccess(const Blob& plainData)
    {
      ++nSuccesses_;
    }

    void
    onError(EncryptError::ErrorCode errorCode, const string& message)
    {
      ++nFailures_;
    }

    int nSuccesses_;
    int nFailures_;
  };

  Callbacks callbacks;
  // Database and crypto operations are synchronous, so this returns immediately.
  fixture.decryptor_->decrypt
    (encryptedContent,
     bind(&Callbacks::onSuccess, &callbacks, _1),
     bind(&Callbacks::onError, &callbacks, _1, _2));

  ASSERT_EQ(0, callbacks.nSuccesses_);
  ASSERT_EQ(1, callbacks.nFailures_);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

