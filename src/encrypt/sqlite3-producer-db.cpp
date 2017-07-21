/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/producer-db https://github.com/named-data/ndn-group-encrypt
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
#include <ndn-cpp/encrypt/sqlite3-producer-db.hpp>

using namespace std;

namespace ndn {

static const string INITIALIZATION =
  "CREATE TABLE IF NOT EXISTS                         \n"
  "  contentkeys(                                     \n"
  "    rowId            INTEGER PRIMARY KEY,          \n"
  "    timeslot         INTEGER,                      \n"
  "    key              BLOB NOT NULL                 \n"
  "  );                                               \n"
  "CREATE UNIQUE INDEX IF NOT EXISTS                  \n"
  "   timeslotIndex ON contentkeys(timeslot);         \n";

/**
 * A utility function to call the normal sqlite3_bind_blob where the value and
 * length are blob.buf() and blob.size().
 */
static int sqlite3_bind_blob
  (sqlite3_stmt* statement, int index, const Blob& value, void(*destructor)(void*))
{
  return sqlite3_bind_blob(statement, index, value.buf(), value.size(), destructor);
}

Sqlite3ProducerDb::Sqlite3ProducerDb(const string& databaseFilePath)
{
  int status = sqlite3_open(databaseFilePath.c_str(), &database_);

  if (status != SQLITE_OK)
    throw ProducerDb::Error("Producer DB cannot be opened/created");

  // Enable foreign keys.
  sqlite3_exec(database_, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);

  // initialize database specific tables
  char* errorMessage = 0;
  status = sqlite3_exec(database_, INITIALIZATION.c_str(), NULL, NULL, &errorMessage);
  if (status != SQLITE_OK && errorMessage != 0) {
    sqlite3_free(errorMessage);
    throw ProducerDb::Error("Producer DB cannot be initialized");
  }
}

bool
Sqlite3ProducerDb::hasContentKey(MillisecondsSince1970 timeSlot)
{
  int fixedTimeSlot = getFixedTimeSlot(timeSlot);

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT key FROM contentkeys where timeslot=?",
     -1, &statement, 0);
  sqlite3_bind_int(statement, 1, fixedTimeSlot);

  bool result = (sqlite3_step(statement) == SQLITE_ROW);
  sqlite3_finalize(statement);
  return result;
}

Blob
Sqlite3ProducerDb::getContentKey(MillisecondsSince1970 timeSlot)
{
  int fixedTimeSlot = getFixedTimeSlot(timeSlot);

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "SELECT key FROM contentkeys where timeslot=?", -1, &statement, 0);
  sqlite3_bind_int(statement, 1, fixedTimeSlot);

  int res = sqlite3_step(statement);

  if (res == SQLITE_ROW) {
    Blob result = Blob
      (static_cast<const uint8_t*>(sqlite3_column_blob(statement, 0)),
       sqlite3_column_bytes(statement, 0));
    sqlite3_finalize(statement);
    return result;
  }
  else
    throw ProducerDb::Error
      ("Sqlite3ProducerDb.getContentKey: Cannot get the key from the database");
}

void
Sqlite3ProducerDb::addContentKey(MillisecondsSince1970 timeSlot, const Blob& key)
{
  int fixedTimeSlot = getFixedTimeSlot(timeSlot);

  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "INSERT INTO contentkeys (timeslot, key) values (?, ?)",
     -1, &statement, 0);
  sqlite3_bind_int(statement, 1, fixedTimeSlot);
  sqlite3_bind_blob(statement, 2, key, SQLITE_TRANSIENT);

  int status = sqlite3_step(statement);
  sqlite3_finalize(statement);
  if (status != SQLITE_DONE)
    throw ProducerDb::Error("Cannot add the key to the database");
}

void
Sqlite3ProducerDb::deleteContentKey(MillisecondsSince1970 timeSlot)
{
  int fixedTimeSlot = getFixedTimeSlot(timeSlot);
  sqlite3_stmt *statement;
  sqlite3_prepare_v2
    (database_, "DELETE FROM contentkeys WHERE timeslot=?", -1, &statement, 0);
  sqlite3_bind_int(statement, 1, fixedTimeSlot);
  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

}

#endif // NDN_CPP_HAVE_SQLITE3
