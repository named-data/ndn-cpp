/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/consumer-db https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_SQLITE3_CONSUMER_DB_HPP
#define NDN_SQLITE3_CONSUMER_DB_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include "../ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_SQLITE3

#include "../common.hpp"
#include "consumer-db.hpp"

struct sqlite3;

namespace ndn {

/**
 * Sqlite3ConsumerDb extends ConsumerDb to implement the storage of decryption
 * keys for the consumer using SQLite3.
 * @note This class is an experimental feature. The API may change.
 */
class Sqlite3ConsumerDb : public ConsumerDb {
public:
  /**
   * Create an Sqlite3ConsumerDb to use the given SQLite3 file.
   * @param databaseFilePath The path of the SQLite file.
   */
  Sqlite3ConsumerDb(const std::string& databaseFilePath);

  /**
   * Get the key with keyName from the database.
   * @param keyName The key name.
   * @return A Blob with the encoded key, or an isNull Blob if cannot find the
   * key with keyName.
   * @throws ConsumerDb::Error for a database error.
   */
  virtual Blob
  getKey(const Name& keyName);

  /**
   * Add the key with keyName and keyBlob to the database.
   * @param keyName The key name.
   * @param keyBlob The encoded key.
   * @throws ConsumerDb::Error if a key with the same keyName already exists in
   * the database, or other database error.
   */
  virtual void
  addKey(const Name& keyName, const Blob& keyBlob);

  /**
   * Delete the key with keyName from the database. If there is no key with
   * keyName, do nothing.
   * @param keyName The key name.
   * @throws ConsumerDb::Error for a database error.
   */
  virtual void
  deleteKey(const Name& keyName);

private:
  struct sqlite3 *database_;
};

}

#endif // NDN_CPP_HAVE_SQLITE3

#endif
