/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include "../util/logging.hpp"
#include "registered-prefix-table.hpp"

INIT_LOGGER("ndn.RegisteredPrefixTable");

using namespace std;

namespace ndn {

void
RegisteredPrefixTable::removeRegisteredPrefix(uint64_t registeredPrefixId)
{
  int count = 0;
  // Go backwards through the list so we can erase entries.
  // Remove all entries even though registeredPrefixId should be unique.
  for (int i = (int)table_.size() - 1; i >= 0; --i) {
    Entry& entry = *table_[i];

    if (entry.getRegisteredPrefixId() == registeredPrefixId) {
      ++count;

      if (entry.getRelatedInterestFilterId() > 0)
        // Remove the related interest filter.
        interestFilterTable_.unsetInterestFilter(entry.getRelatedInterestFilterId());

      table_.erase(table_.begin() + i);
    }
  }

  if (count == 0)
    _LOG_DEBUG("removeRegisteredPrefix: Didn't find registeredPrefixId " << registeredPrefixId);
}

}