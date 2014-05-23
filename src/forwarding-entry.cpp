/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/forwarding-entry.hpp>
#include "c/forwarding-entry.h"

using namespace std;

namespace ndn {
  
void 
ForwardingEntry::set(const struct ndn_ForwardingEntry& forwardingEntryStruct) 
{
  if (forwardingEntryStruct.action.value && forwardingEntryStruct.action.length > 0)
    action_ = string(forwardingEntryStruct.action.value, forwardingEntryStruct.action.value + forwardingEntryStruct.action.length);
  else
    action_ = "";
            
  prefix_.set(forwardingEntryStruct.prefix);
  publisherPublicKeyDigest_.set(forwardingEntryStruct.publisherPublicKeyDigest);  
  faceId_ = forwardingEntryStruct.faceId;
  forwardingFlags_ = forwardingEntryStruct.forwardingFlags;
  freshnessPeriod_ = forwardingEntryStruct.freshnessPeriod;
}

void 
ForwardingEntry::get(struct ndn_ForwardingEntry& forwardingEntryStruct) const 
{
  prefix_.get(forwardingEntryStruct.prefix);
  publisherPublicKeyDigest_.get(forwardingEntryStruct.publisherPublicKeyDigest);
  forwardingEntryStruct.faceId = faceId_;
  forwardingEntryStruct.forwardingFlags = forwardingFlags_;
  forwardingEntryStruct.freshnessPeriod = freshnessPeriod_;

  forwardingEntryStruct.action.length = action_.size();
  if (action_.size() > 0)
    forwardingEntryStruct.action.value = (uint8_t *)&action_[0];
  else
    forwardingEntryStruct.action.value = 0;
}

}
