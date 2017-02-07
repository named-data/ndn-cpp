/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include <algorithm>
#include <ndn-cpp/util/logging.hpp>
#include "pending-interest-table.hpp"

INIT_LOGGER("ndn.PendingInterestTable");

using namespace std;

namespace ndn {

void
PendingInterestTable::Entry::callTimeout()
{
  if (onTimeout_) {
    try {
      onTimeout_(interest_);
    } catch (const std::exception& ex) {
      _LOG_ERROR("PendingInterestTable::Entry::callTimeout: Error in onTimeout: " <<
                 ex.what());
    } catch (...) {
      _LOG_ERROR("PendingInterestTable::Entry::callTimeout: Error in onTimeout.");
    }
  }
}

ptr_lib::shared_ptr<PendingInterestTable::Entry>
PendingInterestTable::add
  (uint64_t pendingInterestId,
   const ptr_lib::shared_ptr<const Interest>& interestCopy, const OnData& onData,
   const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack)
{
  vector<uint64_t>::iterator removeRequestIterator =
    ::find(removeRequests_.begin(), removeRequests_.end(), pendingInterestId);
  if (removeRequestIterator != removeRequests_.end()) {
    // removePendingInterest was called with the pendingInterestId returned by
    //   expressInterest before we got here, so don't add a PIT entry.
    removeRequests_.erase(removeRequestIterator);
    return ptr_lib::shared_ptr<Entry>();
  }

  ptr_lib::shared_ptr<Entry> entry(new Entry
    (pendingInterestId, interestCopy, onData, onTimeout, onNetworkNack));
  table_.push_back(entry);
  return entry;
}

void
PendingInterestTable::extractEntriesForExpressedInterest
  (const Data& data, vector<ptr_lib::shared_ptr<Entry> > &entries)
{
  // Go backwards through the list so we can erase entries.
  for (int i = (int)table_.size() - 1; i >= 0; --i) {
    ptr_lib::shared_ptr<Entry>& pendingInterest = table_[i];
    if (pendingInterest->getInterest()->matchesData(data)) {
      entries.push_back(pendingInterest);
      // We let the callback from callLater call _processInterestTimeout, but
      // for efficiency, mark this as removed so that it returns right away.
      pendingInterest->setIsRemoved();
      table_.erase(table_.begin() + i);
    }
  }
}

void
PendingInterestTable::extractEntriesForNackInterest
  (const Interest& interest, vector<ptr_lib::shared_ptr<Entry> > &entries)
{
  SignedBlob encoding = interest.wireEncode();

  // Go backwards through the list so we can erase entries.
  for (int i = (int)table_.size() - 1; i >= 0; --i) {
    ptr_lib::shared_ptr<Entry>& pendingInterest = table_[i];
    if (!pendingInterest->getOnNetworkNack())
      continue;

    // wireEncode returns the encoding cached when the interest was sent (if
    // it was the default wire encoding).
    if (pendingInterest->getInterest()->wireEncode().equals(encoding)) {
      entries.push_back(pendingInterest);
      // We let the callback from callLater call _processInterestTimeout, but
      // for efficiency, mark this as removed so that it returns right away.
      pendingInterest->setIsRemoved();
      table_.erase(table_.begin() + i);
    }
  }
}

void
PendingInterestTable::removePendingInterest(uint64_t pendingInterestId)
{
  int count = 0;
  // Go backwards through the list so we can erase entries.
  // Remove all entries even though pendingInterestId should be unique.
  for (int i = (int)table_.size() - 1; i >= 0; --i) {
    if (table_[i]->getPendingInterestId() == pendingInterestId) {
      ++count;
      // For efficiency, mark this as removed so that processInterestTimeout
      // doesn't look for it.
      table_[i]->setIsRemoved();
      table_.erase(table_.begin() + i);
    }
  }

  if (count == 0)
    _LOG_DEBUG("removePendingInterest: Didn't find pendingInterestId " << pendingInterestId);

  if (count == 0) {
    // The pendingInterestId was not found. Perhaps this has been called before
    //   the callback in expressInterest can add to the PIT. Add this
    //   removal request which will be checked before adding to the PIT.
    if (::find(removeRequests_.begin(), removeRequests_.end(), pendingInterestId)
        == removeRequests_.end())
      // Not already requested, so add the request.
      removeRequests_.push_back(pendingInterestId);
  }
}

bool
PendingInterestTable::removeEntry
  (const ptr_lib::shared_ptr<Entry>& pendingInterest)
{
  if (pendingInterest->getIsRemoved())
    // extractEntriesForExpressedInterest or removePendingInterest has
    // removed pendingInterest from the table, so we don't need to look for it.
    // Do nothing.
    return false;

  // Search by pointer equality.
  for (size_t i = 0; i < table_.size(); ++i) {
    if (table_[i].get() == pendingInterest.get()) {
      table_[i]->setIsRemoved();
      table_.erase(table_.begin() + i);
      return true;
    }
  }

  return false;
}

}
