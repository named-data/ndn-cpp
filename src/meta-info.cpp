/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <ndn-cpp/meta-info.hpp>
#include "c/data.h"

using namespace std;

namespace ndn {

void 
MetaInfo::get(struct ndn_MetaInfo& metaInfoStruct) const 
{
  metaInfoStruct.timestampMilliseconds = timestampMilliseconds_;
  metaInfoStruct.type = type_;
  metaInfoStruct.freshnessPeriod = freshnessPeriod_;
  finalBlockID_.get(metaInfoStruct.finalBlockID);
}

void 
MetaInfo::set(const struct ndn_MetaInfo& metaInfoStruct)
{
  setTimestampMilliseconds(metaInfoStruct.timestampMilliseconds);
  setType(metaInfoStruct.type);
  setFreshnessPeriod(metaInfoStruct.freshnessPeriod);
  setFinalBlockID(Name::Component(Blob(metaInfoStruct.finalBlockID.value)));
}

}
