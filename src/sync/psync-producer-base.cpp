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
#include <ndn-cpp/sync/psync-producer-base.hpp>

using namespace std;

INIT_LOGGER("ndn.PSyncProducerBase");

namespace ndn {

PSyncProducerBase::PSyncProducerBase
  (size_t expectedNEntries, const Name& syncPrefix,
   Milliseconds syncReplyFreshnessPeriod)
: iblt_(new InvertibleBloomLookupTable(expectedNEntries)),
  expectedNEntries_(expectedNEntries),
  threshold_(expectedNEntries / 2),
  syncPrefix_(syncPrefix),
  syncReplyFreshnessPeriod_(syncReplyFreshnessPeriod)
{
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
