/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "../c/util/time.h"
#include <ndn-cpp/util/memory-content-cache.hpp>

using namespace std;

namespace ndn {
  
MemoryContentCache::MemoryContentCache
  (Face* face, Milliseconds cleanupIntervalMilliseconds)
: face_(face), cleanupIntervalMilliseconds_(cleanupIntervalMilliseconds),
  nextCleanupTime_(ndn_getNowMilliseconds() + cleanupIntervalMilliseconds)
{
}
  
void
MemoryContentCache::add(const Data& data)
{
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
}

void 
MemoryContentCache::operator()
  (const ptr_lib::shared_ptr<const Name>& prefix, 
   const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport,
   uint64_t registeredPrefixId)
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
        transport.send(*content->getDataEncoding());
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
    transport.send(*selectedEncoding);
  else {
    // Call the onDataNotFound callback (if defined).
    map<string, OnInterest>::iterator onDataNotFound = 
      onDataNotFoundForPrefix_.find(prefix->toUri());
    if (onDataNotFound != onDataNotFoundForPrefix_.end() &&
        onDataNotFound->second)
      onDataNotFound->second(prefix, interest, transport, registeredPrefixId);
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

}
