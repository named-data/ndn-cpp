/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_PUBLISHERPUBLICKEYDIGEST_HPP
#define NDN_PUBLISHERPUBLICKEYDIGEST_HPP

#include <vector>
#include "common.hpp"
#include "util/blob.hpp"

struct ndn_PublisherPublicKeyDigest;

namespace ndn {
  
/**
 * A PublisherPublicKeyDigest holds the publisher public key digest value, if any.
 * We make a separate class since this is used by multiple other classes.
 */
class PublisherPublicKeyDigest {
public:    
  PublisherPublicKeyDigest() 
  : changeCount_(0)
  {
  }
  
  /**
   * Set the publisherPublicKeyDigestStruct to point to the entries in this PublisherPublicKeyDigest, without copying any memory.
   * WARNING: The resulting pointers in publisherPublicKeyDigestStruct are invalid after a further use of this object which could reallocate memory.
   * @param publisherPublicKeyDigestStruct a C ndn_PublisherPublicKeyDigest struct to receive the pointer
   */
  void 
  get(struct ndn_PublisherPublicKeyDigest& publisherPublicKeyDigestStruct) const;
  
  /**
   * Clear this PublisherPublicKeyDigest, and copy from the ndn_PublisherPublicKeyDigest struct.
   * @param excludeStruct a C ndn_Exclude struct
   */
  void 
  set(const struct ndn_PublisherPublicKeyDigest& publisherPublicKeyDigestStruct);

  const Blob& 
  getPublisherPublicKeyDigest() const { return publisherPublicKeyDigest_; }

  void 
  setPublisherPublicKeyDigest(const Blob& publisherPublicKeyDigest) 
  { 
    publisherPublicKeyDigest_ = publisherPublicKeyDigest; 
    ++changeCount_;
  }
  
  /**
   * Clear the publisherPublicKeyDigest.
   */
  void 
  clear()
  {
    publisherPublicKeyDigest_.reset();
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t 
  getChangeCount() const { return changeCount_; }

private:
  Blob publisherPublicKeyDigest_;
  uint64_t changeCount_;
};
  
}

#endif
