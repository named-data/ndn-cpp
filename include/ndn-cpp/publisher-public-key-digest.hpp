/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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
