/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/group-manager-db.t.cpp
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
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/sqlite3-group-manager-db.hpp>

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

static uint8_t SCHEDULE[] = {
  0x8f, 0xc4,// Schedule
  0x8d, 0x90,// WhiteIntervalList
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x04,
    0x89, 0x01,
      0x07,
    0x8a, 0x01,
      0x00,
    0x8b, 0x01,
      0x00,
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x38, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x05,
    0x89, 0x01,
      0x0a,
    0x8a, 0x01,
      0x02,
    0x8b, 0x01,
      0x01,
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x38, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x06,
    0x89, 0x01,
      0x08,
    0x8a, 0x01,
      0x01,
    0x8b, 0x01,
      0x01,
  0x8e, 0x30, // BlackIntervalList
  0x8c, 0x2e, // RepetitiveInterval
     0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x37, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x37, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x07,
    0x89, 0x01,
      0x08,
    0x8a, 0x01,
      0x00,
    0x8b, 0x01,
      0x00
};

class TestGroupManagerDb : public ::testing::Test {
public:
  TestGroupManagerDb()
  {
    databaseFilePath = getPolicyConfigDirectory() + "/test.db";
    remove(databaseFilePath.c_str());

    database.reset(new Sqlite3GroupManagerDb(databaseFilePath));
  }

  virtual void
  TearDown()
  {
    remove(databaseFilePath.c_str());
  }

  string databaseFilePath;
  ptr_lib::shared_ptr<GroupManagerDb> database;
};

TEST_F(TestGroupManagerDb, DatabaseFunctions)
{
  Blob scheduleBlob(SCHEDULE, sizeof(SCHEDULE));

  // Create a schedule.
  Schedule schedule;
  schedule.wireDecode(scheduleBlob);

  // Create a member.
  RsaKeyParams params;
  DecryptKey decryptKey = RsaAlgorithm::generateKey(params);
  EncryptKey encryptKey = RsaAlgorithm::deriveEncryptKey(decryptKey.getKeyBits());
  Blob keyBlob = encryptKey.getKeyBits();

  Name name1("/ndn/BoyA/ksk-123");
  Name name2("/ndn/BoyB/ksk-1233");
  Name name3("/ndn/GirlC/ksk-123");
  Name name4("/ndn/GirlD/ksk-123");
  Name name5("/ndn/Hello/ksk-123");

  database->addSchedule("work-time", schedule);
  database->addSchedule("rest-time", schedule);
  database->addSchedule("play-time", schedule);
  database->addSchedule("boelter-time", schedule);

  // Throw an exception when adding a schedule with an existing name.
  ASSERT_THROW(database->addSchedule("boelter-time", schedule), GroupManagerDb::Error)
    << "Expected an error adding a duplicate schedule";

  // Add members into the database.
  database->addMember("work-time", name1, keyBlob);
  database->addMember("rest-time", name2, keyBlob);
  database->addMember("play-time", name3, keyBlob);
  database->addMember("play-time", name4, keyBlob);

  // Throw an exception when adding a member with a non-existing schedule name.
  ASSERT_THROW(database->addMember("false-time", name5, keyBlob), GroupManagerDb::Error)
    << "Expected an error adding a member with non-existing schedule";

  database->addMember("boelter-time", name5, keyBlob);

  // Throw an exception when adding a member having an existing identity.
  ASSERT_THROW(database->addMember("work-time", name5, keyBlob), GroupManagerDb::Error)
    << "Expected an error adding a member with an existing identity";

  // Test has functions.
  ASSERT_EQ(true, database->hasSchedule("work-time"));
  ASSERT_EQ(true, database->hasSchedule("rest-time"));
  ASSERT_EQ(true, database->hasSchedule("play-time"));
  ASSERT_EQ(false, database->hasSchedule("sleep-time"));
  ASSERT_EQ(false, database->hasSchedule(""));

  ASSERT_EQ(true, database->hasMember(Name("/ndn/BoyA")));
  ASSERT_EQ(true, database->hasMember(Name("/ndn/BoyB")));
  ASSERT_EQ(false, database->hasMember(Name("/ndn/BoyC")));

  // Get a schedule.
  ptr_lib::shared_ptr<Schedule> scheduleResult = database->getSchedule("work-time");
  ASSERT_TRUE(scheduleResult->wireEncode().equals(scheduleBlob));

  scheduleResult = database->getSchedule("play-time");
  ASSERT_TRUE(scheduleResult->wireEncode().equals(scheduleBlob));

  // Throw an exception when when there is no such schedule in the database.
  ASSERT_THROW(database->getSchedule("work-time-11"), GroupManagerDb::Error)
    << "Expected an error getting a non-existing schedule";

  // List all schedule names.
  vector<string> names;
  database->listAllScheduleNames(names);
  ASSERT_TRUE(find(names.begin(), names.end(), "work-time") != names.end());
  ASSERT_TRUE(find(names.begin(), names.end(), "play-time") != names.end());
  ASSERT_TRUE(find(names.begin(), names.end(), "rest-time") != names.end());
  ASSERT_TRUE(find(names.begin(), names.end(), "sleep-time") == names.end());

  // List members of a schedule.
  map<Name, Blob> memberMap;
  database->getScheduleMembers("play-time", memberMap);
  ASSERT_TRUE(memberMap.size() != 0);

  // When there's no such schedule, the return map's size should be 0.
  database->getScheduleMembers("sleep-time", memberMap);
  ASSERT_EQ(0, memberMap.size());

  // List all members.
  vector<Name> members;
  database->listAllMembers(members);
  ASSERT_TRUE(find(members.begin(), members.end(), Name("/ndn/GirlC")) != members.end());
  ASSERT_TRUE(find(members.begin(), members.end(), Name("/ndn/GirlD")) != members.end());
  ASSERT_TRUE(find(members.begin(), members.end(), Name("/ndn/BoyA")) != members.end());
  ASSERT_TRUE(find(members.begin(), members.end(), Name("/ndn/BoyB")) != members.end());

  // Rename a schedule.
  ASSERT_EQ(true, database->hasSchedule("boelter-time"));
  database->renameSchedule("boelter-time", "rieber-time");
  ASSERT_EQ(false, database->hasSchedule("boelter-time"));
  ASSERT_EQ(true, database->hasSchedule("rieber-time"));
  ASSERT_EQ("rieber-time", database->getMemberSchedule(Name("/ndn/Hello")));

  // Update a schedule.
  Schedule newSchedule;
  newSchedule.wireDecode(scheduleBlob);
  ptr_lib::shared_ptr<RepetitiveInterval> repetitiveInterval(new RepetitiveInterval
    (fromIsoString("20150825T000000"), fromIsoString("20150921T000000"), 2, 10,
     5, RepetitiveInterval::RepeatUnit::DAY));
  newSchedule.addWhiteInterval(repetitiveInterval);
  database->updateSchedule("rieber-time", newSchedule);
  scheduleResult = database->getSchedule("rieber-time");
  ASSERT_TRUE(!scheduleResult->wireEncode().equals(scheduleBlob));
  ASSERT_TRUE(scheduleResult->wireEncode().equals(newSchedule.wireEncode()));

  // Add a new schedule when updating a non-existing schedule.
  ASSERT_EQ(false, database->hasSchedule("ralphs-time"));
  database->updateSchedule("ralphs-time", newSchedule);
  ASSERT_EQ(true, database->hasSchedule("ralphs-time"));

  // Update the schedule of a member.
  database->updateMemberSchedule(Name("/ndn/Hello"), "play-time");
  ASSERT_EQ("play-time", database->getMemberSchedule(Name("/ndn/Hello")));

  // Delete a member.
  ASSERT_EQ(true, database->hasMember(Name("/ndn/Hello")));
  database->deleteMember(Name("/ndn/Hello"));
  ASSERT_EQ(false, database->hasMember(Name("/ndn/Hello")));

  // Delete a non-existing member.
  database->deleteMember(Name("/ndn/notExisting"));

  // Delete a schedule. All the members using this schedule should be deleted.
  database->deleteSchedule("play-time");
  ASSERT_EQ(false, database->hasSchedule("play-time"));
  ASSERT_EQ(false, database->hasMember(Name("/ndn/GirlC")));
  ASSERT_EQ(false, database->hasMember(Name("/ndn/GirlD")));

  // Delete a non-existing schedule.
  database->deleteSchedule("not-existing-time");
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

