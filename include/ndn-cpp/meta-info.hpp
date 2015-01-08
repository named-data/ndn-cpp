/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#ifndef NDN_META_INFO_HPP
#define NDN_META_INFO_HPP

#include <math.h>
#include "name.hpp"
#include "c/data-types.h"

struct ndn_MetaInfo;

namespace ndn {

/**
 * A MetaInfo holds the meta info which is signed inside the data packet.
 */
class MetaInfo {
public:
  MetaInfo()
  : changeCount_(0)
  {
    timestampMilliseconds_ = -1;
    type_ = ndn_ContentType_BLOB;
    freshnessPeriod_ = -1;
  }

  /**
   * Set the metaInfoStruct to point to the values in this meta info object, without copying any memory.
   * WARNING: The resulting pointers in metaInfoStruct are invalid after a further use of this object which could reallocate memory.
   * @param metaInfoStruct a C ndn_MetaInfo struct where the name components array is already allocated.
   */
  void
  get(struct ndn_MetaInfo& metaInfoStruct) const;

  /**
   * Clear this meta info, and set the values by copying from the ndn_MetaInfo struct.
   * @param metaInfoStruct a C ndn_MetaInfo struct
   */
  void
  set(const struct ndn_MetaInfo& metaInfoStruct);

  /**
   * @deprecated Use the application-specific content to store a timestamp.
   */
  MillisecondsSince1970
  DEPRECATED_IN_NDN_CPP getTimestampMilliseconds() const
  {
    return timestampMilliseconds_;
  }

  ndn_ContentType
  getType() const { return type_; }

  Milliseconds
  getFreshnessPeriod() const { return freshnessPeriod_; }

  /**
   * @deprecated Use getFreshnessPeriod.
   */
  int
  DEPRECATED_IN_NDN_CPP getFreshnessSeconds() const
  {
    return freshnessPeriod_ < 0 ? -1 : (int)round(freshnessPeriod_ / 1000.0);
  }

  /**
   * Get the final block ID.
   * @return The final block ID as a Name::Component.  If the name component
   * getValue().size() is 0, then the final block ID is not specified.
   */
  const Name::Component&
  getFinalBlockId() const { return finalBlockId_; }

  /**
   * @deprecated Use getFinalBlockId.
   */
  const Name::Component&
  DEPRECATED_IN_NDN_CPP getFinalBlockID() const { return getFinalBlockId(); }

  /**
   * @deprecated Use the application-specific content to store a timestamp.
   */
  void
  DEPRECATED_IN_NDN_CPP setTimestampMilliseconds
    (MillisecondsSince1970 timestampMilliseconds)
  {
    timestampMilliseconds_ = timestampMilliseconds;
    ++changeCount_;
  }

  void
  setType(ndn_ContentType type)
  {
    type_ = type;
    ++changeCount_;
  }

  void
  setFreshnessPeriod(Milliseconds freshnessPeriod)
  {
    freshnessPeriod_ = freshnessPeriod;
    ++changeCount_;
  }

  /**
   * @deprecated Use setFreshnessPeriod.
   */
  void
  DEPRECATED_IN_NDN_CPP setFreshnessSeconds(int freshnessSeconds)
  {
    setFreshnessPeriod(freshnessSeconds < 0 ? -1.0 : (double)freshnessSeconds * 1000.0);
  }

  /**
   * Set the final block ID.
   * @param finalBlockId The final block ID as a Name::Component.  If the name
   * component getValue().size() is 0, then the final block ID is not specified.
   */
  void
  setFinalBlockId(const Name::Component& finalBlockId)
  {
    finalBlockId_ = finalBlockId;
    ++changeCount_;
  }

  /**
   * @deprecated Use setFinalBlockId.
   */
  void
  DEPRECATED_IN_NDN_CPP setFinalBlockID(const Name::Component& finalBlockId)
  {
    finalBlockId_ = finalBlockId;
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const { return changeCount_; }

private:
  MillisecondsSince1970 timestampMilliseconds_; /**< milliseconds since 1/1/1970. -1 for none */
  ndn_ContentType type_;         /**< default is ndn_ContentType_BLOB. -1 for none */
  Milliseconds freshnessPeriod_; /**< -1 for none */
  Name::Component finalBlockId_; /** size 0 for none */
  uint64_t changeCount_;
};

}

#endif
