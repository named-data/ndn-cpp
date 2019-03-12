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
#include "./detail/psync-user-prefixes.hpp"
#include <ndn-cpp/sync/psync-producer-base.hpp>

using namespace std;

INIT_LOGGER("ndn.PSyncProducerBase");

namespace ndn {

int
PSyncProducerBase::getSequenceNo(const Name& prefix) const
{
  return prefixes_->getSequenceNo(prefix);
}

bool
PSyncProducerBase::addUserNode(const Name& prefix)
{
  return prefixes_->addUserNode(prefix);
}

void
PSyncProducerBase::removeUserNode(const Name& prefix)
{
  if (prefixes_->isUserNode(prefix)) {
    int sequenceNo = prefixes_->prefixes_[prefix];
    prefixes_->removeUserNode(prefix);
    removeFromIblt(Name(prefix).appendNumber(sequenceNo));
  }
}

PSyncProducerBase::PSyncProducerBase
  (size_t expectedNEntries, const Name& syncPrefix,
   Milliseconds syncReplyFreshnessPeriod)
: iblt_(new InvertibleBloomLookupTable(expectedNEntries)),
  expectedNEntries_(expectedNEntries),
  threshold_(expectedNEntries / 2),
  syncPrefix_(syncPrefix),
  syncReplyFreshnessPeriod_(syncReplyFreshnessPeriod),
  prefixes_(new PSyncUserPrefixes())
{
}

void
PSyncProducerBase::updateSequenceNo(const Name& prefix, int sequenceNo)
{
  int oldSequenceNo;
  if (!prefixes_->updateSequenceNo(prefix, sequenceNo, oldSequenceNo))
    return;  
  
  // Delete the old sequence number from the IBLT. If oldSequenceNo is zero, we
  // don't need to delete it, because we don't insert a prefix with sequence
  // number zero in the IBLT.
  if (oldSequenceNo != 0)
    removeFromIblt(Name(prefix).appendNumber(oldSequenceNo));

  // Insert the new sequence number.
  Name prefixWithSequenceNo = Name(prefix).appendNumber(sequenceNo);
  insertIntoIblt(prefixWithSequenceNo);
}

void
PSyncProducerBase::insertIntoIblt(const Name& name)
{
  string uri = name.toUri();
  uint32_t newHash = CryptoLite::murmurHash3
    (InvertibleBloomLookupTable::N_HASHCHECK, uri.data(), uri.size());
  nameToHash_[name] = newHash;
  hashToName_[newHash] = name;
  iblt_->insert(newHash);
}

void
PSyncProducerBase::removeFromIblt(const Name& name)
{
  map<Name, uint32_t>::iterator hashEntry = nameToHash_.find(name);
  if (hashEntry != nameToHash_.end()) {
    uint32_t hash = hashEntry->second;
    nameToHash_.erase(hashEntry);
    hashToName_.erase(hash);
    iblt_->erase(hash);
  }
}

void
PSyncProducerBase::onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
{
  _LOG_ERROR("PSyncProduerBase: Failed to register prefix: " << prefix);
}

}
