/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/util/sqlite3-statement.cpp
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

#include <stdexcept>
#include "sqlite3-statement.hpp"

using namespace std;

namespace ndn {

Sqlite3Statement::Sqlite3Statement(sqlite3* database, const string& statement)
{
  int result = sqlite3_prepare_v2(database, statement.c_str(), -1, &statement_, 0);
  if (result != SQLITE_OK)
    throw domain_error("Error preparing SQL statement: " + statement);
}

Sqlite3Statement::~Sqlite3Statement()
{
  sqlite3_finalize(statement_);
}

}

#endif // NDN_CPP_HAVE_SQLITE3
