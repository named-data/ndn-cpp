/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/util/sqlite3-statement.hpp
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

#ifndef NDN_SQLITE3_STATEMENT_HPP
#define NDN_SQLITE3_STATEMENT_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include <ndn-cpp/ndn-cpp-config.h>
#ifdef NDN_CPP_HAVE_SQLITE3

#include <string>
#include <sqlite3.h>
#include <ndn-cpp/util/blob.hpp>

namespace ndn {

/*
 * Sqlite3Statement is a utility class to wrap an SQLite3 prepared statement,
 * provide access methods, and finalize the statement in the destructor.
 */
class Sqlite3Statement
{
public:
  /**
   * Create an Sqlite3Statement to prepare an SQLite3 statement.
   * @param database The handle to open the SQLite3 database.
   * @param statement The SQL statement to prepare.
   * @throws std::domain_error SQL statement is bad
   */
  Sqlite3Statement(sqlite3* database, const std::string& statement);

  /**
   * Finalize the statement.
   */
  ~Sqlite3Statement();

  /**
   * Wrap sqlite3_bind_text to bind the char array to the statement. Use
   * SQLITE_TRANSIENT.
   * @param index The binding position.
   * @param value The pointer to the char array string to bind.
   * @param size The size of the char array string.
   * @return SQLite result value.
   */
  int
  bind(int index, const char* value, size_t size)
  {
    return sqlite3_bind_text(statement_, index, value, size, SQLITE_TRANSIENT);
  }

  /**
   * Wrap sqlite3_bind_text to bind the string to the statement. Use
   * SQLITE_TRANSIENT.
   * @param index The binding position.
   * @param value The string to bind.
   * @return SQLite result value.
   */
  int
  bind(int index, const std::string& value)
  {
    return bind(index, value.c_str(), value.size());
  }

  /**
   * Wrap sqlite3_bind_blob to bind the byte buffer to the statement.
   * @param index The binding position.
   * @param buf The pointer to the byte buffer to bind.
   * @param size The size of the byte buffer.
   * @param bufIsStatic (optional) If true, use SQLITE_STATIC to not copy the
   * static byte buffer, which must remain valid until stored. If false or
   * omitted, use SQLITE_TRANSIENT to copy the byte buffer.
   * @return SQLite result value.
   */
  int
  bind(int index, const uint8_t* buf, size_t size, bool bufIsStatic = false)
  {
    return sqlite3_bind_blob
      (statement_, index, buf, size,
       bufIsStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
  }

  /**
   * Wrap sqlite3_bind_blob to bind the blob to the statement.
   * @param index The binding position.
   * @param blob The Blob with the byte buffer to bind.
   * @param bufIsStatic (optional) If true, use SQLITE_STATIC to not copy the
   * blob's byte buffer, which must remain valid until stored. If false or
   * omitted, use SQLITE_TRANSIENT to copy the byte buffer.
   * @return SQLite result value.
   */
  int
  bind(int index, const Blob& blob, bool bufIsStatic = false)
  {
    return bind(index, blob.buf(), blob.size(), bufIsStatic);
  }

  /**
   * Wrap sqlite3_bind_int to bind the integer to the statement.
   * @param index The binding position.
   * @param integer The integer to bind.
   * @return SQLite result value.
   */
  int
  bind(int index, int integer)
  {
    return sqlite3_bind_int(statement_, index, integer);
  }

  /**
   * Wrap sqlite3_column_text.
   * @param column The column.
   * @return The text converted to a string.
   */
  std::string
  getString(int column)
  {
    return std::string
      (reinterpret_cast<const char*>(sqlite3_column_text(statement_, column)),
       sqlite3_column_bytes(statement_, column));
  }

  /**
   * Wrap sqlite3_column_int.
   * @param column The column.
   * @return The integer.
   */
  int
  getInt(int column) { return sqlite3_column_int(statement_, column); }

  /**
   * Wrap sqlite3_column_blob.
   * @param column The column.
   * @return The buffer pointer as a const uint8_t*.
   */
  const uint8_t*
  getBuf(int column)
  {
    return static_cast<const uint8_t*>(sqlite3_column_blob(statement_, column));
  }

  /**
   * Wrap sqlite3_column_bytes.
   * @param column The column.
   * @return The number of bytes.
   */
  int
  getSize(int column) { return sqlite3_column_bytes(statement_, column); }

  /**
   * Wrap sqlite3_column_blob and sqlite3_column_bytes to return a new Blob.
   * @param column The column.
   * @return The new Blob.
   */
  Blob
  getBlob(int column) { return Blob(getBuf(column), getSize(column)); }

  /**
   * Wrap sqlite3_step.
   * @return SQLite result value.
   */
  int
  step() { return sqlite3_step(statement_); }

private:
  // Disable the copy constructor and assignment operator.
  Sqlite3Statement(const Sqlite3Statement& other);
  Sqlite3Statement& operator=(const Sqlite3Statement& other);

  sqlite3_stmt* statement_;
};

}

#endif // NDN_CPP_HAVE_SQLITE3

#endif
