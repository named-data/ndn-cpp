/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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
#include "../c/util/time.h"
#include "logging.hpp"
#include <ndn-cpp/util/memory-content-cache.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.MemoryContentCache");

namespace ndn {

MemoryContentCache::MemoryContentCache
  (Face* face, Milliseconds cleanupIntervalMilliseconds)
: face_(face), cleanupIntervalMilliseconds_(cleanupIntervalMilliseconds),
  nextCleanupTime_(ndn_getNowMilliseconds() + cleanupIntervalMilliseconds),
  // We don't expect this callback to be stored outside of this object, so
  // don't worry about using shared_from_this().
  storePendingInterestCallback_(bind
    (&MemoryContentCache::storePendingInterestCallback, this, _1, _2, _3, _4, _5))
{
}

void
MemoryContentCache::unregisterAll()
{
  for (size_t i = 0; i < interestFilterIdList_.size(); ++i)
    face_->unsetInterestFilter(interestFilterIdList_[i]);
  interestFilterIdList_.clear();

  for (size_t i = 0; i < registeredPrefixIdList_.size(); ++i)
    face_->removeRegisteredPrefix(registeredPrefixIdList_[i]);
  registeredPrefixIdList_.clear();

  // Also clear each onDataNotFoundForPrefix given to registerPrefix.
  onDataNotFoundForPrefix_.clear();
}

void
MemoryContentCache::add(const Data& data)
{
  doCleanup();

  if (data.getMetaInfo().getFreshnessPeriod() >= 0.0) {
    // The content will go stale, so use staleTimeCache_.
    ptr_lib::shared_ptr<const StaleTimeContent> content(new StaleTimeContent(data));
    // Insert into staleTimeCache_, sorted on content->staleTimeMilliseconds_.
    staleTimeCache_.insert
      (std::lower_bound(staleTimeCache_.begin(), staleTimeCache_.end(), content, contentCompare_),
       content);
  }
  else
    // The data does not go stale, so use noStaleTimeCache_.
    noStaleTimeCache_.push_back
      (ptr_lib::make_shared<const Content>(data));

  // Remove timed-out interests and check if the data packet matches any pending
  // interest.
  // Go backwards through the list so we can erase entries.
  MillisecondsSince1970 nowMilliseconds = ndn_getNowMilliseconds();
  for (int i = (int)pendingInterestTable_.size() - 1; i >= 0; --i) {
    if (pendingInterestTable_[i]->isTimedOut(nowMilliseconds)) {
      pendingInterestTable_.erase(pendingInterestTable_.begin() + i);
      continue;
    }

    if (pendingInterestTable_[i]->getInterest()->matchesName(data.getName())) {
      try {
        // Send to the same transport from the original call to onInterest.
        // wireEncode returns the cached encoding if available.
        pendingInterestTable_[i]->getFace().send(*data.wireEncode());
      } catch (std::exception& e) {
        _LOG_DEBUG("Error in send: " << e.what());
        return;
      }

      // The pending interest is satisfied, so remove it.
      pendingInterestTable_.erase(pendingInterestTable_.begin() + i);
    }
  }
}

void
MemoryContentCache::storePendingInterest
  (const ptr_lib::shared_ptr<const Interest>& interest, Face& face)
{
  pendingInterestTable_.push_back(ptr_lib::shared_ptr<PendingInterest>
    (new PendingInterest(interest, face)));
}

void
MemoryContentCache::getPendingInterestsForName
  (const Name& name,
   vector<ptr_lib::shared_ptr<const PendingInterest> >& pendingInterests)
{
  pendingInterests.clear();

  // Remove timed-out interests as we add results.
  // Go backwards through the list so we can erase entries.
  MillisecondsSince1970 nowMilliseconds = ndn_getNowMilliseconds();
  for (int i = (int)pendingInterestTable_.size() - 1; i >= 0; --i) {
    if (pendingInterestTable_[i]->isTimedOut(nowMilliseconds)) {
      pendingInterestTable_.erase(pendingInterestTable_.begin() + i);
      continue;
    }

    if (pendingInterestTable_[i]->getInterest()->matchesName(name))
      pendingInterests.push_back(pendingInterestTable_[i]);
  }
}

void
MemoryContentCache::operator()
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  doCleanup();

  const Name::Component* selectedComponent = 0;
  Blob selectedEncoding;
  // We need to iterate over both arrays.
  size_t totalSize = staleTimeCache_.size() + noStaleTimeCache_.size();
  for (size_t i = 0; i < totalSize; ++i) {
    const Content* content;
    if (i < staleTimeCache_.size())
      content = staleTimeCache_[i].get();
    else
      // We have iterated over the first array. Get from the second.
      content = noStaleTimeCache_[i - staleTimeCache_.size()].get();

    if (interest->matchesName(content->getName())) {
      if (interest->getChildSelector() < 0) {
        // No child selector, so send the first match that we have found.
        face.send(*content->getDataEncoding());
        return;
      }
      else {
        // Update selectedEncoding based on the child selector.
        const Name::Component* component;
        if (content->getName().size() > interest->getName().size())
          component = &content->getName().get(interest->getName().size());
        else
          component = &emptyComponent_;

        bool gotBetterMatch = false;
        if (!selectedEncoding)
          // Save the first match.
          gotBetterMatch = true;
        else {
          if (interest->getChildSelector() == 0) {
            // Leftmost child.
            if (*component < *selectedComponent)
              gotBetterMatch = true;
          }
          else {
            // Rightmost child.
            if (*component > *selectedComponent)
              gotBetterMatch = true;
          }
        }

        if (gotBetterMatch) {
          selectedComponent = component;
          selectedEncoding = content->getDataEncoding();
        }
      }
    }
  }

  if (selectedEncoding)
    // We found the leftmost or rightmost child.
    face.send(*selectedEncoding);
  else {
    // Call the onDataNotFound callback (if defined).
    map<string, OnInterestCallback>::iterator onDataNotFound =
      onDataNotFoundForPrefix_.find(prefix->toUri());
    if (onDataNotFound != onDataNotFoundForPrefix_.end() &&
        onDataNotFound->second) {
      try {
        onDataNotFound->second(prefix, interest, face, interestFilterId, filter);
      } catch (const std::exception& ex) {
        _LOG_ERROR("MemoryContentCache::operator(): Error in onDataNotFound: " << ex.what());
      } catch (...) {
        _LOG_ERROR("MemoryContentCache::operator(): Error in onDataNotFound.");
      }
    }
  }
}

void
MemoryContentCache::doCleanup()
{
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  if (now >= nextCleanupTime_) {
    // staleTimeCache_ is sorted on staleTimeMilliseconds_, so we only need to
    // erase the stale entries at the front, then quit.
    while (staleTimeCache_.size() > 0 && staleTimeCache_.front()->isStale(now))
      staleTimeCache_.erase(staleTimeCache_.begin());

    nextCleanupTime_ = now + cleanupIntervalMilliseconds_;
  }
}

MemoryContentCache::StaleTimeContent::StaleTimeContent(const Data& data)
// wireEncode returns the cached encoding if available.
: Content(data)
{
  // Set up staleTimeMilliseconds_.
  staleTimeMilliseconds_ = ndn_getNowMilliseconds() +
    data.getMetaInfo().getFreshnessPeriod();
}

MemoryContentCache::PendingInterest::PendingInterest
  (const ptr_lib::shared_ptr<const Interest>& interest, Face& face)
  : interest_(interest), face_(face), timeoutPeriodStart_(ndn_getNowMilliseconds())
{
  // Set up timeoutTime_.
  if (interest_->getInterestLifetimeMilliseconds() >= 0.0)
    timeoutTimeMilliseconds_ = timeoutPeriodStart_ +
      interest_->getInterestLifetimeMilliseconds();
  else
    // No timeout.
    timeoutTimeMilliseconds_ = -1.0;
}

}
