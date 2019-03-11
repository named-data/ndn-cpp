/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/producer-base.cpp
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

#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "./detail/invertible-bloom-lookup-table.hpp"
#include "./detail/psync-segment-publisher.hpp"
#include <ndn-cpp/sync/psync-producer-base.hpp>

using namespace std;

INIT_LOGGER("ndn.PSyncProducerBase");

namespace ndn {

  int
PSyncProducerBase::getSequenceNo(const Name& prefix) const
{
  map<Name, int>::const_iterator entry = prefixes_.find(prefix);
  if (entry == prefixes_.end())
    return -1;

  return entry->second;
}

bool
PSyncProducerBase::addUserNode(const Name& prefix)
{
  if (prefixes_.find(prefix) == prefixes_.end()) {
    prefixes_[prefix] = 0;
    return true;
  }
  else
    return false;
}

void
PSyncProducerBase::removeUserNode(const Name& prefix)
{
  map<Name, int>::iterator entry = prefixes_.find(prefix);
  if (entry != prefixes_.end()) {
    int sequenceNo = entry->second;
    prefixes_.erase(entry);

    Name prefixWithSequenceNo = Name(prefix).appendNumber(sequenceNo);
    map<Name, uint32_t>::iterator hashEntry =
      nameToHash_.find(prefixWithSequenceNo);
    if (hashEntry != nameToHash_.end()) {
      uint32_t hash = hashEntry->second;
      nameToHash_.erase(hashEntry);
      hashToName_.erase(hash);
      iblt_->erase(hash);
    }
  }
}

PSyncProducerBase::PSyncProducerBase
  (size_t expectedNEntries, Face& face,
   const Name& syncPrefix, const Name& userPrefix, KeyChain& keyChain, 
   Milliseconds syncReplyFreshnessPeriod, Milliseconds helloReplyFreshnessPeriod,
   const SigningInfo& signingInfo)
: iblt_(new InvertibleBloomLookupTable(expectedNEntries)),
  expectedNEntries_(expectedNEntries),
  threshold_(expectedNEntries / 2),
  face_(face),
  syncPrefix_(syncPrefix),
  keyChain_(keyChain),
  syncReplyFreshnessPeriod_(syncReplyFreshnessPeriod),
  helloReplyFreshnessPeriod_(helloReplyFreshnessPeriod),
  signingInfo_(signingInfo),
  segmentPublisher_(new PSyncSegmentPublisher(face_, keyChain_))
{
  if (userPrefix.size() > 0)
    addUserNode(userPrefix);
}

void
PSyncProducerBase::updateSequenceNo(const Name& prefix, int sequenceNo)
{
  _LOG_DEBUG("updateSequenceNo: " << prefix << " " << sequenceNo);

  int oldSequenceNo;
  map<Name, int>::iterator entry = prefixes_.find(prefix);
  if (entry != prefixes_.end())
    oldSequenceNo = entry->second;
  else {
    _LOG_TRACE("The prefix was not found in prefixes_");
    return;
  }

  if (oldSequenceNo >= sequenceNo) {
    _LOG_TRACE("The update has a lower/equal seq no for then prefix. Doing nothing!");
    return;
  }

  // Delete the old sequence number from the IBLT. If oldSequenceNo is zero, we
  // don't need to delete it, because we don't insert a prefix with sequence
  // number zero in the IBLT.
  if (oldSequenceNo != 0) {
    Name prefixWithOldSequenceNo = Name(prefix).appendNumber(oldSequenceNo);
    map<Name, uint32_t>::iterator hashEntry =
      nameToHash_.find(prefixWithOldSequenceNo);
    if (hashEntry != nameToHash_.end()) {
      uint32_t hash = hashEntry->second;
      nameToHash_.erase(hashEntry);

      hashToName_.erase(hash);
      iblt_->erase(hash);
    }
  }

  // Insert the new sequence number.
  entry->second = sequenceNo;
  Name prefixWithSequenceNo = Name(prefix).appendNumber(sequenceNo);
  string uri = prefixWithSequenceNo.toUri();
  uint32_t newHash = CryptoLite::murmurHash3
    (InvertibleBloomLookupTable::N_HASHCHECK, uri.data(), uri.size());
  nameToHash_[prefixWithSequenceNo] = newHash;
  hashToName_[newHash] = prefixWithSequenceNo;
  iblt_->insert(newHash);
}

void
PSyncProducerBase::sendApplicationNack(const Name& name)
{
  _LOG_DEBUG("Sending application Nack");

  Data data(name);
  data.getName().append(iblt_->encode());
  data.getName().appendSegment(0);

  data.getMetaInfo().setFreshnessPeriod(syncReplyFreshnessPeriod_);
  data.getMetaInfo().setType(ndn_ContentType_NACK);
  data.getMetaInfo().setFinalBlockId(data.getName()[-1]);
  keyChain_.sign(data, signingInfo_);
  face_.putData(data);
}

void
PSyncProducerBase::onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
{
  _LOG_ERROR("PSyncProduerBase: Failed to register prefix: " << prefix);
}

}
