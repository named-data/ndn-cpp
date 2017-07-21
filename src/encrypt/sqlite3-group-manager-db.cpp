/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/group-manager-db https://github.com/named-data/ndn-group-encrypt
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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include <ndn-cpp/ndn-cpp-config.h>
#ifdef NDN_CPP_HAVE_SQLITE3

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <ndn-cpp/encrypt/sqlite3-group-manager-db.hpp>

using namespace std;

namespace ndn {

  static const string INITIALIZATION =
  "CREATE TABLE IF NOT EXISTS                         \n"
  "  schedules(                                       \n"
  "    schedule_id         INTEGER PRIMARY KEY,       \n"
  "    schedule_name       TEXT NOT NULL,             \n"
  "    schedule            BLOB NOT NULL              \n"
  "  );                                               \n"
  "CREATE UNIQUE INDEX IF NOT EXISTS                  \n"
  "   scheduleNameIndex ON schedules(schedule_name);  \n"
  "                                                   \n"
  "CREATE TABLE IF NOT EXISTS                         \n"
  "  members(                                         \n"
  "    member_id           INTEGER PRIMARY KEY,       \n"
  "    schedule_id         INTEGER NOT NULL,          \n"
  "    member_name         BLOB NOT NULL,             \n"
  "    key_name            BLOB NOT NULL,             \n"
  "    pubkey              BLOB NOT NULL,             \n"
  "    FOREIGN KEY(schedule_id)                       \n"
  "      REFERENCES schedules(schedule_id)            \n"
  "      ON DELETE CASCADE                            \n"
  "      ON UPDATE CASCADE                            \n"
  "  );                                               \n"
  "CREATE UNIQUE INDEX IF NOT EXISTS                  \n"
  "   memNameIndex ON members(member_name);           \n";

/**
 * A utility function to call the normal sqlite3_bind_text where the value and
 * length are value.c_str() and value.size().
 */
static int sqlite3_bind_text
  (sqlite3_stmt* statement, int index, const string& value, void(*destructor)(void*))
{
  return sqlite3_bind_text(statement, index, value.c_str(), value.size(), destructor);
}

/**
 * A utility function to call the normal sqlite3_bind_blob where the value and
 * length are blob.buf() and blob.size().
 */
static int sqlite3_bind_blob
  (sqlite3_stmt* statement, int index, const Blob& value, void(*destructor)(void*))
{
  return sqlite3_bind_blob(statement, index, value.buf(), value.size(), destructor);
}

Sqlite3GroupManagerDb::Sqlite3GroupManagerDb(const string& databaseFilePath)
{
  int status = sqlite3_open(databaseFilePath.c_str(), &database_);

  if (status != SQLITE_OK)
    throw GroupManagerDb::Error("GroupManager DB cannot be opened/created");

  // Enable foreign keys.
  sqlite3_exec(database_, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);

  // initialize database specific tables
  char* errorMessage = 0;
  status = sqlite3_exec(database_, INITIALIZATION.c_str(), NULL, NULL, &errorMessage);
  if (status != SQLITE_OK && errorMessage != 0) {
    sqlite3_free(errorMessage);
    throw GroupManagerDb::Error("GroupManager DB cannot be initialized");
  }
}

bool
Sqlite3GroupManagerDb::hasSchedule(const string& name)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT schedule_id FROM schedules WHERE schedule_name=?",
     -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);

  bool result = (sqlite3_step(statement) == SQLITE_ROW);
  sqlite3_finalize(statement);
  return result;
}

void
Sqlite3GroupManagerDb::listAllScheduleNames(vector<string>& nameList)
{
  nameList.clear();
  
  sqlite3_stmt* statement;
  sqlite3_prepare_v2
    (database_, "SELECT schedule_name FROM schedules", -1, &statement, 0);

  while (sqlite3_step(statement) == SQLITE_ROW)
    nameList.push_back
      (string(reinterpret_cast<const char *>(sqlite3_column_text(statement, 0)),
              sqlite3_column_bytes(statement, 0)));

  sqlite3_finalize(statement);
}

ptr_lib::shared_ptr<Schedule>
Sqlite3GroupManagerDb::getSchedule(const string& name)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT schedule FROM schedules where schedule_name=?",
     -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);

  if (status == SQLITE_ROW) {
    ptr_lib::shared_ptr<Schedule> result(new Schedule());
    try {
      result->wireDecode
        (Blob((const uint8_t*)sqlite3_column_blob(statement, 0),
              sqlite3_column_bytes(statement, 0)));
    } catch (...) {
      sqlite3_finalize(statement);
      throw GroupManagerDb::Error
        ("Sqlite3GroupManagerDb::getSchedule: The schedule cannot be decoded");
    }

    sqlite3_finalize(statement);
    return result;
  }
  else {
    sqlite3_finalize(statement);
    throw GroupManagerDb::Error("Cannot get the result from the database");
  }
}

void
Sqlite3GroupManagerDb::getScheduleMembers
  (const string& name, map<Name, Blob>& memberMap)
{
  memberMap.clear();

  sqlite3_stmt* statement;
  sqlite3_prepare_v2
    (database_,
     "SELECT key_name, pubkey\
      FROM members JOIN schedules ON members.schedule_id=schedules.schedule_id\
      WHERE schedule_name=?", -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);

  while (sqlite3_step(statement) == SQLITE_ROW) {
    Name keyName;
    try {
      keyName.wireDecode
        (Blob((const uint8_t*)sqlite3_column_blob(statement, 0),
              sqlite3_column_bytes(statement, 0)));
    } catch (...) {
      sqlite3_finalize(statement);
      throw GroupManagerDb::Error
        ("Sqlite3GroupManagerDb.getScheduleMembers: Error decoding name");
    }

    memberMap[keyName] = Blob((const uint8_t*)sqlite3_column_blob(statement, 1),
                              sqlite3_column_bytes(statement, 1));
  }

  sqlite3_finalize(statement);
}

void
Sqlite3GroupManagerDb::addSchedule(const string& name, const Schedule& schedule)
{
  if (name.size() == 0)
    throw GroupManagerDb::Error("addSchedule: The schedule name cannot be empty");

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "INSERT INTO schedules (schedule_name, schedule) values (?, ?)",
     -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);
  sqlite3_bind_blob(statement, 2, schedule.wireEncode(), SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw GroupManagerDb::Error("Cannot add the schedule to the database");
}

void
Sqlite3GroupManagerDb::deleteSchedule(const string& name)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "DELETE FROM schedules WHERE schedule_name=?", -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

void
Sqlite3GroupManagerDb::renameSchedule
  (const string& oldName, const string& newName)
{
  if (newName.size() == 0)
    throw GroupManagerDb::Error
      ("renameSchedule: The schedule newName cannot be empty");

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "UPDATE schedules SET schedule_name=? WHERE schedule_name=?",
     -1, &statement, 0);
  sqlite3_bind_text(statement, 1, newName, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, oldName, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw GroupManagerDb::Error("Cannot rename the schedule in the database");
}

void
Sqlite3GroupManagerDb::updateSchedule
  (const string& name, const Schedule& schedule)
{
  if (!hasSchedule(name)) {
    addSchedule(name, schedule);
    return;
  }

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "UPDATE schedules SET schedule=? WHERE schedule_name=?",
     -1, &statement, 0);
  sqlite3_bind_blob(statement, 1, schedule.wireEncode(), SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, name, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw GroupManagerDb::Error("Cannot update the schedule in the database");
}

bool
Sqlite3GroupManagerDb::hasMember(const Name& identity)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT member_id FROM members WHERE member_name=?",
     -1, &statement, 0);
  sqlite3_bind_blob(statement, 1, identity.wireEncode(), SQLITE_TRANSIENT);

  bool result = (sqlite3_step(statement) == SQLITE_ROW);
  sqlite3_finalize(statement);
  return result;
}

void
Sqlite3GroupManagerDb::listAllMembers(vector<Name>& nameList)
{
  nameList.clear();

  sqlite3_stmt* statement;
  sqlite3_prepare_v2
    (database_, "SELECT member_name FROM members", -1, &statement, 0);

  while (sqlite3_step(statement) == SQLITE_ROW) {
    Name name;
    try {
      name.wireDecode
        (Blob((const uint8_t*)sqlite3_column_blob(statement, 0),
              sqlite3_column_bytes(statement, 0)));
    } catch (...) {
      sqlite3_finalize(statement);
      throw GroupManagerDb::Error
        ("Sqlite3GroupManagerDb.listAllMembers: Error decoding name");
    }

    nameList.push_back(name);
  }

  sqlite3_finalize(statement);
}

string
Sqlite3GroupManagerDb::getMemberSchedule(const Name& identity)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_,
     "SELECT schedule_name\
      FROM schedules JOIN members ON schedules.schedule_id = members.schedule_id\
      WHERE member_name=?",
     -1, &statement, 0);
  sqlite3_bind_blob(statement, 1, identity.wireEncode(), SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);

  if (status == SQLITE_ROW) {
    string result(reinterpret_cast<const char *>(sqlite3_column_text(statement, 0)),
                  sqlite3_column_bytes(statement, 0));

    sqlite3_finalize(statement);
    return result;
  }
  else {
    sqlite3_finalize(statement);
    throw GroupManagerDb::Error("Cannot get the result from the database");
  }
}

void
Sqlite3GroupManagerDb::addMember
  (const string& scheduleName, const Name& keyName, const Blob& key)
{
  int scheduleId = getScheduleId(scheduleName);
  if (scheduleId == -1)
    throw GroupManagerDb::Error("The schedule does not exist");

  // Needs to be changed in the future.
  Name memberName = keyName.getPrefix(-1);

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_,
     "INSERT INTO members(schedule_id, member_name, key_name, pubkey)\
      values (?, ?, ?, ?)",
     -1, &statement, 0);
  sqlite3_bind_int(statement, 1, scheduleId);
  sqlite3_bind_blob(statement, 2, memberName.wireEncode(), SQLITE_TRANSIENT);
  sqlite3_bind_blob(statement, 3, keyName.wireEncode(), SQLITE_TRANSIENT);
  sqlite3_bind_blob(statement, 4, key, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw GroupManagerDb::Error("Cannot add the member to the database");
}

void
Sqlite3GroupManagerDb::updateMemberSchedule
  (const Name& identity, const string& scheduleName)
{
  int scheduleId = getScheduleId(scheduleName);
  if (scheduleId == -1)
    throw GroupManagerDb::Error
      ("Sqlite3GroupManagerDb.updateMemberSchedule: The schedule does not exist");

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "UPDATE members SET schedule_id=? WHERE member_name=?",
     -1, &statement, 0);
  sqlite3_bind_int(statement, 1, scheduleId);
  sqlite3_bind_blob(statement, 2, identity.wireEncode(), SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw GroupManagerDb::Error("Sqlite3GroupManagerDb.updateMemberSchedule: SQLite error");
}

void
Sqlite3GroupManagerDb::deleteMember(const Name& identity)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "DELETE FROM members WHERE member_name=?", -1, &statement, 0);
  sqlite3_bind_blob(statement, 1, identity.wireEncode(), SQLITE_TRANSIENT);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

int
Sqlite3GroupManagerDb::getScheduleId(const std::string& name)
{
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT schedule_id FROM schedules WHERE schedule_name=?",
     -1, &statement, 0);
  sqlite3_bind_text(statement, 1, name, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);

  int result = -1;
  if (status == SQLITE_ROW)
    result = sqlite3_column_int(statement, 0);

  sqlite3_finalize(statement);
  return result;
}

}

#endif // NDN_CPP_HAVE_SQLITE3
