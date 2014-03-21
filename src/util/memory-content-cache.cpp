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

void 
MemoryContentCache::operator()
  (const ptr_lib::shared_ptr<const Name>& prefix, 
   const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport,
   uint64_t registeredPrefixId)
{
  pruneContentCache();
  
  for (size_t i = 0; i < contentCache_.size(); ++i) {
    const Content* content = contentCache_[i].get();
    // TODO: Look for longest match?
    // TODO: Check childSelector.
    if (interest->matchesName(content->getName())) {
      transport.send(*content->getDataEncoding());
      break;
    }
  }
}


void
MemoryContentCache::pruneContentCache()
{
  // Go backwards through the list so we can erase entries.
  MillisecondsSince1970 nowMilliseconds = ndn_getNowMilliseconds();
  for (int i = (int)contentCache_.size() - 1; i >= 0; --i) {
    if (contentCache_[i]->isStale(nowMilliseconds))
      contentCache_.erase(contentCache_.begin() + i);
  }
}

MemoryContentCache::Content::Content(const Data& data)
// wireEncode returns the cached encoding if available.
: name_(data.getName()), dataEncoding_(data.wireEncode())
{
  // Set up staleTimeMilliseconds_.
  if (data.getMetaInfo().getFreshnessPeriod() >= 0.0)
    staleTimeMilliseconds_ = ndn_getNowMilliseconds() + 
      data.getMetaInfo().getFreshnessPeriod();
  else
    // No timeout.
    staleTimeMilliseconds_ = -1.0;
}

}
