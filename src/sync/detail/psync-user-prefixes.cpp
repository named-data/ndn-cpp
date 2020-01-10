/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/user-prefixes.cpp
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

#include <ndn-cpp/util/logging.hpp>
#include "psync-user-prefixes.hpp"

using namespace std;

INIT_LOGGER("ndn.PSyncUserPrefixes");

namespace ndn {

bool
PSyncUserPrefixes::addUserNode(const Name& prefix)
{
  if (!isUserNode(prefix)) {
    prefixes_[prefix] = 0;
    return true;
  }
  else
    return false;
}

void
PSyncUserPrefixes::removeUserNode(const Name& prefix)
{
  map<Name, int>::iterator entry = prefixes_.find(prefix);
  if (entry != prefixes_.end())
    prefixes_.erase(entry);
}

bool
PSyncUserPrefixes::updateSequenceNo
  (const Name& prefix, int sequenceNo, int& oldSequenceNo)
{
  oldSequenceNo = 0;
  _LOG_DEBUG("updateSequenceNo: " << prefix << " " << sequenceNo);

  map<Name, int>::iterator entry = prefixes_.find(prefix);
  if (entry != prefixes_.end())
    oldSequenceNo = entry->second;
  else {
    _LOG_TRACE("The prefix was not found in prefixes_");
    return false;
  }

  if (oldSequenceNo >= sequenceNo) {
    _LOG_TRACE("The update has a lower/equal sequence number for the prefix. Doing nothing!");
    return false;
  }

  // Insert the new sequence number.
  entry->second = sequenceNo;
  return true;
}

}
