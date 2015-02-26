/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#ifndef NDN_META_INFO_LITE_HPP
#define NDN_META_INFO_LITE_HPP

#include "name-lite.hpp"
#include "../c/data-types.h"

namespace ndn {

class MetaInfoLite : private ndn_MetaInfo {
public:
  MetaInfoLite();

  ndn_MillisecondsSince1970
  getTimestampMilliseconds() const { return timestampMilliseconds; }

  ndn_ContentType
  getType() const { return type; }

  ndn_Milliseconds
  getFreshnessPeriod() const { return freshnessPeriod; }

  const NameLite::Component
  getFinalBlockId() const { return NameLite::Component::upCast(finalBlockId); }

  void
  setTimestampMilliseconds(ndn_MillisecondsSince1970 timestampMilliseconds)
  {
    this->timestampMilliseconds = timestampMilliseconds;
  }

  void
  setType(ndn_ContentType type) { this->type = type; }

  void
  setFreshnessPeriod(ndn_Milliseconds freshnessPeriod)
  {
    this->freshnessPeriod = freshnessPeriod;
  }

  void
  setFinalBlockId(const NameLite::Component& finalBlockId)
  {
    NameLite::Component::upCast(this->finalBlockId) = finalBlockId;
  }

  /**
   * Upcast the reference to the ndn_MetaInfo struct to a MetaInfoLite.
   * @param metaInfo A reference to the ndn_MetaInfo struct.
   * @return The same reference as MetaInfoLite.
   */
  static MetaInfoLite&
  upCast(ndn_MetaInfo& metaInfo) { return *(MetaInfoLite*)&metaInfo; }

  static const MetaInfoLite&
  upCast(const ndn_MetaInfo& metaInfo) { return *(MetaInfoLite*)&metaInfo; }
};

}

#endif
