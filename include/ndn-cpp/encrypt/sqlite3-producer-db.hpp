/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
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

#ifndef NDN_SQLITE3_PRODUCER_DB_HPP
#define NDN_SQLITE3_PRODUCER_DB_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include "../ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_SQLITE3

#include "../common.hpp"
#include "producer-db.hpp"

struct sqlite3;

namespace ndn {

/**
 * Sqlite3ProducerDb extends ProducerDb to implement storage of keys for the
 * producer using SQLite3. It contains one table that maps time slots (to the
 * nearest hour) to the content key created for that time slot.
 * @note This class is an experimental feature. The API may change.
 */
class Sqlite3ProducerDb : public ProducerDb {
public:
  /**
   * Create an Sqlite3ProducerDb to use the given SQLite3 file.
   * @param databaseFilePath The path of the SQLite file.
   */
  Sqlite3ProducerDb(const std::string& databaseFilePath);

  /**
   * Check if a content key exists for the hour covering timeSlot.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @return True if there is a content key for timeSlot.
   * @throws ProducerDb::Error for a database error.
   */
  virtual bool
  hasContentKey(MillisecondsSince1970 timeSlot);

  /**
   * Get the content key for the hour covering timeSlot.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @return A Blob with the encoded key.
   * @throws ProducerDb::Error if there is no key covering timeSlot or other
   * database error.
   */
  virtual Blob
  getContentKey(MillisecondsSince1970 timeSlot);

  /**
   * Add key as the content key for the hour covering timeSlot.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @param key The encoded key.
   * @throws ProducerDb::Error if a key for the same hour already exists in the
   * database, or other database error.
   */
  virtual void
  addContentKey(MillisecondsSince1970 timeSlot, const Blob& key);

  /**
   * Delete the content key for the hour covering timeSlot. If there is no key
   * for the time slot, do nothing.
   * @param timeSlot The time slot as milliseconds since Jan 1, 1970 UTC.
   * @throws ProducerDb::Error for a database error.
   */
  virtual void
  deleteContentKey(MillisecondsSince1970 timeSlot);

private:
  struct sqlite3 *database_;
};

}

#endif // NDN_CPP_HAVE_SQLITE3

#endif
