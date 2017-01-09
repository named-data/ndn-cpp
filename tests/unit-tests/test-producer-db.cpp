/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/producer-db.t.cpp
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
#include <ndn-cpp/encrypt/schedule.hpp>
#include <ndn-cpp/encrypt/sqlite3-producer-db.hpp>

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

static MillisecondsSince1970
fromIsoString(const string& dateString)
{
  return Schedule::fromIsoString(dateString);
}

class TestProducerDb : public ::testing::Test {
public:
  TestProducerDb()
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

TEST_F(TestProducerDb, DatabaseFunctions)
{
  // Test construction.
  Sqlite3ProducerDb database(databaseFilePath);

  // Create member.
  AesKeyParams params(128);
  Blob keyBlob1 = AesAlgorithm::generateKey(params).getKeyBits();
  Blob keyBlob2 = AesAlgorithm::generateKey(params).getKeyBits();

  MillisecondsSince1970 point1 = fromIsoString("20150101T100000");
  MillisecondsSince1970 point2 = fromIsoString("20150102T100000");
  MillisecondsSince1970 point3 = fromIsoString("20150103T100000");
  MillisecondsSince1970 point4 = fromIsoString("20150104T100000");

  // Add keys into the database.
  database.addContentKey(point1, keyBlob1);
  database.addContentKey(point2, keyBlob1);
  database.addContentKey(point3, keyBlob2);

  // Throw an exception when adding a key to an existing time slot.
  ASSERT_THROW(database.addContentKey(point1, keyBlob1), ProducerDb::Error)
    << "addContentKey did not throw an exception";

  // Check has functions.
  ASSERT_EQ(true, database.hasContentKey(point1));
  ASSERT_EQ(true, database.hasContentKey(point2));
  ASSERT_EQ(true, database.hasContentKey(point3));
  ASSERT_EQ(false, database.hasContentKey(point4));

  // Get content keys.
  Blob keyResult = database.getContentKey(point1);
  ASSERT_TRUE(keyResult.equals(keyBlob1));

  keyResult = database.getContentKey(point3);
  ASSERT_TRUE(keyResult.equals(keyBlob2));

  // Throw exception when there is no such time slot in the database.
  ASSERT_THROW(database.getContentKey(point4), ProducerDb::Error)
    << "getContentKey did not throw an exception";

  // Delete content keys.
  ASSERT_EQ(true, database.hasContentKey(point1));
  database.deleteContentKey(point1);
  ASSERT_EQ(false, database.hasContentKey(point1));

  // Delete at a non-existing time slot.
  database.deleteContentKey(point4);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

