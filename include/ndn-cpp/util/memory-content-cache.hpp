/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_MEMORY_CONTENT_CACHE_HPP
#define NDN_MEMORY_CONTENT_CACHE_HPP

#include <vector>
#include "../face.hpp"

namespace ndn {

class MemoryContentCache {
public:
  MemoryContentCache(Face* face)
  : face_(face)
  {
  }
  
  void
  registerPrefix
    (const Name& prefix, const OnRegisterFailed& onRegisterFailed, 
     const ForwardingFlags& flags = ForwardingFlags(), 
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    face_->registerPrefix
      (prefix, func_lib::ref(*this), onRegisterFailed, flags, wireFormat);
  }
  
  void
  add(const Data& data)
  {
    contentCache_.push_back(ptr_lib::make_shared<const Content>(data));
  }
  
  // onInterest.
  void 
  operator()
    (const ptr_lib::shared_ptr<const Name>& prefix, 
     const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport,
     uint64_t registeredPrefixId);
  
private:
  class Content {
  public:
    Content(const Data& data);
    
    const Name&
    getName() const { return name_; }
    
    const Blob&
    getDataEncoding() const { return dataEncoding_; }

    /**
     * Check if this content is stale.
     * @param nowMilliseconds The current time in milliseconds from 
     * ndn_getNowMilliseconds.
     * @return true if this interest is stale, otherwise false.
     */
    bool 
    isStale(MillisecondsSince1970 nowMilliseconds) const
    {
      return staleTimeMilliseconds_ >= 0.0 && 
             nowMilliseconds >= staleTimeMilliseconds_;
    }
    
  private:
    Name name_;
    Blob dataEncoding_;
    MillisecondsSince1970 staleTimeMilliseconds_; /**< The time when the content 
      becomse stale in milliseconds according to ndn_getNowMilliseconds, or -1 
      to never become stale. */
  };

  void
  pruneContentCache();
  
  Face* face_;
  std::vector<ptr_lib::shared_ptr<const Content> > contentCache_;
};

}

#endif
