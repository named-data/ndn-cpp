/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/consumer-db.t.cpp
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
#include <fstream>
#include <stdexcept>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/sqlite3-consumer-db.hpp>

using namespace std;
using namespace ndn;

static bool
fileExists(const string& filePath)
{
  ifstream stream(filePath.c_str());
  bool result = (bool)stream;
  stream.close();
  return result;
}

static string
getPolicyConfigDirectory()
{
  string policyConfigDirectory = "policy_config";
  // Check if expected files are in this directory.
  if (!fileExists(policyConfigDirectory + "/regex_ruleset.conf")) {
    // Maybe we are running "make check" from the ndn-cpp root.  There may be
    //   a way to tell "make check" to run from tests/unit-tests, but for
    //   now just set policyConfigDirectory explicitly.
    policyConfigDirectory = "tests/unit-tests/policy_config";

    if(!fileExists(policyConfigDirectory + "/regex_ruleset.conf"))
      throw runtime_error("Cannot find the directory for policy-config");
  }

  return policyConfigDirectory;
}

class TestConsumerDb : public ::testing::Test {
public:
  TestConsumerDb()
  {
    databaseFilePath = getPolicyConfigDirectory() + "/test.db";
    remove(databaseFilePath.c_str());
  }

  virtual void
  TearDown()
  {
    remove(databaseFilePath.c_str());
  }

  string databaseFilePath;
};

static void
generateRsaKeys(Blob& encryptionKeyBlob, Blob& decryptionKeyBlob)
{
  RsaKeyParams params;
  DecryptKey decryptKey = RsaAlgorithm::generateKey(params);
  decryptionKeyBlob = decryptKey.getKeyBits();
  EncryptKey encryptKey = RsaAlgorithm::deriveEncryptKey(decryptionKeyBlob);
  encryptionKeyBlob = encryptKey.getKeyBits();
}

static void
generateAesKeys(Blob& encryptionKeyBlob, Blob& decryptionKeyBlob)
{
  AesKeyParams params;
  DecryptKey memberDecryptKey = AesAlgorithm::generateKey(params);
  decryptionKeyBlob = memberDecryptKey.getKeyBits();
  EncryptKey memberEncryptKey = AesAlgorithm::deriveEncryptKey(decryptionKeyBlob);
  encryptionKeyBlob = memberEncryptKey.getKeyBits();
}

TEST_F(TestConsumerDb, OperateAesDecryptionKey)
{
  // Test construction.
  Sqlite3ConsumerDb database(databaseFilePath);

  // Generate key blobs.
  Blob encryptionKeyBlob;
  Blob decryptionKeyBlob;
  generateAesKeys(encryptionKeyBlob, decryptionKeyBlob);

  Name keyName
    ("/alice/health/samples/activity/steps/C-KEY/20150928080000/20150928090000!");
  keyName.append(Name("FOR/alice/health/read/activity!"));
  database.addKey(keyName, decryptionKeyBlob);
  Blob resultBlob = database.getKey(keyName);

  ASSERT_TRUE(decryptionKeyBlob.equals(resultBlob));

  database.deleteKey(keyName);
  resultBlob = database.getKey(keyName);

  ASSERT_EQ(0, resultBlob.size());
}

TEST_F(TestConsumerDb, OperateRsaDecryptionKey)
{
  // Test construction.
  Sqlite3ConsumerDb database(databaseFilePath);

  // Generate key blobs.
  Blob encryptionKeyBlob;
  Blob decryptionKeyBlob;
  generateRsaKeys(encryptionKeyBlob, decryptionKeyBlob);

  Name keyName
    ("/alice/health/samples/activity/steps/D-KEY/20150928080000/20150928090000!");
  keyName.append(Name("FOR/test/member/KEY/123!"));
  database.addKey(keyName, decryptionKeyBlob);
  Blob resultBlob = database.getKey(keyName);

  ASSERT_TRUE(decryptionKeyBlob.equals(resultBlob));

  database.deleteKey(keyName);
  resultBlob = database.getKey(keyName);

  ASSERT_EQ(0, resultBlob.size());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

