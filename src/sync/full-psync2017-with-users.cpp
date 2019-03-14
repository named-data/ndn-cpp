/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/full-producer.cpp
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
#include "./detail/psync-segment-publisher.hpp"
#include "./detail/invertible-bloom-lookup-table.hpp"
#include "./detail/psync-state.hpp"
#include "./detail/psync-segment-publisher.hpp"
#include "./detail/psync-user-prefixes.hpp"
#include <ndn-cpp/sync/full-psync2017-with-users.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.FullPSync2017WithUsers");

namespace ndn {

FullPSync2017WithUsers::Impl::Impl(const OnUpdate& onUpdate)
: onUpdate_(onUpdate), prefixes_(new PSyncUserPrefixes())
{
}

void
FullPSync2017WithUsers::Impl::initialize
  (size_t expectedNEntries, Face& face, const Name& syncPrefix,
   const Name& userPrefix, KeyChain& keyChain, Milliseconds syncInterestLifetime,
   Milliseconds syncReplyFreshnessPeriod, const SigningInfo& signingInfo)
{
  fullPSync_ = ptr_lib::make_shared<FullPSync2017>
    (expectedNEntries, face, syncPrefix,
     bind(&FullPSync2017WithUsers::Impl::onNamesUpdate, shared_from_this(), _1),
     keyChain, syncInterestLifetime, syncReplyFreshnessPeriod, signingInfo,
     bind(&FullPSync2017WithUsers::Impl::isNotFutureHash, shared_from_this(), _1, _2),
     bind(&FullPSync2017WithUsers::Impl::canAddReceivedName, shared_from_this(), _1));

  if (userPrefix.size() > 0)
    addUserNode(userPrefix);
}

int
FullPSync2017WithUsers::Impl::getSequenceNo(const Name& prefix) const
{
  return prefixes_->getSequenceNo(prefix);
}

bool
FullPSync2017WithUsers::Impl::addUserNode(const Name& prefix)
{
  return prefixes_->addUserNode(prefix);
}

void
FullPSync2017WithUsers::Impl::removeUserNode(const Name& prefix)
{
  if (prefixes_->isUserNode(prefix)) {
    int sequenceNo = prefixes_->prefixes_[prefix];
    prefixes_->removeUserNode(prefix);
    fullPSync_->removeName(Name(prefix).appendNumber(sequenceNo));
  }
}

void
FullPSync2017WithUsers::Impl::publishName(const Name& prefix, int sequenceNo)
{
  if (!prefixes_->isUserNode(prefix)) {
    _LOG_ERROR("Prefix not added: " << prefix);
    return;
  }

  int newSequenceNo = sequenceNo >= 0 ? sequenceNo : prefixes_->prefixes_[prefix] + 1;

  _LOG_TRACE("Publish: " << prefix << "/" << newSequenceNo);
  if (updateSequenceNo(prefix, newSequenceNo))
    // Insert the new sequence number.
    fullPSync_->publishName(Name(prefix).appendNumber(newSequenceNo));
}

bool
FullPSync2017WithUsers::Impl::canAddReceivedName(const Name& name)
{
  Name prefix = name.getPrefix(-1);
  int sequenceNo = name.get(-1).toNumber();

  bool havePrefix = prefixes_->isUserNode(prefix);
  if (!havePrefix || prefixes_->prefixes_[prefix] < sequenceNo) {
    if (havePrefix) {
      int oldSequenceNo = prefixes_->prefixes_[prefix];
      if (oldSequenceNo != 0)
        // Remove the old sequence number from the IBLT before the caller adds
        // the new one.
        fullPSync_->removeName(Name(prefix).appendNumber(oldSequenceNo));
    }

    return true;
  }
  else
    return false;
}

void
FullPSync2017WithUsers::Impl::onNamesUpdate
  (const ptr_lib::shared_ptr<std::vector<Name>>& names)
{
  ptr_lib::shared_ptr<vector<ptr_lib::shared_ptr<PSyncMissingDataInfo>>> updates
    (ptr_lib::make_shared<vector<ptr_lib::shared_ptr<PSyncMissingDataInfo>>>());

  for (vector<Name>::const_iterator name = names->begin();
       name != names->end(); ++name) {
    Name prefix = name->getPrefix(-1);
    int sequenceNo = name->get(-1).toNumber();

    updates->push_back(ptr_lib::make_shared<PSyncMissingDataInfo>
      (prefix, prefixes_->prefixes_[prefix] + 1, sequenceNo));

    // canAddReceivedName already made sure that the new sequenceNo is greater
    // than the old one, and removed the old one from the IBLT.
    prefixes_->prefixes_[prefix] = sequenceNo;
  }

  try {
    onUpdate_(updates);
  } catch (const std::exception& ex) {
    _LOG_ERROR("Error in onUpdate: " << ex.what());
  } catch (...) {
    _LOG_ERROR("Error in onUpdate.");
  }
}

bool
FullPSync2017WithUsers::Impl::isNotFutureHash
  (const Name& name, const set<uint32_t>& negative)
{
  Name prefix = name.getPrefix(-1);

  string uri = Name(prefix).appendNumber(prefixes_->prefixes_[prefix] + 1).toUri();
  uint32_t nextHash = CryptoLite::murmurHash3
    (InvertibleBloomLookupTable::N_HASHCHECK, uri.data(), uri.size());

  for (set<uint32_t>::iterator negativeHash = negative.begin();
       negativeHash != negative.end();
       ++negativeHash) {
    if (*negativeHash == nextHash)
      return false;
  }

  return true;
}

bool
FullPSync2017WithUsers::Impl::updateSequenceNo(const Name& prefix, int sequenceNo)
{
  int oldSequenceNo;
  if (!prefixes_->updateSequenceNo(prefix, sequenceNo, oldSequenceNo))
    return false;

  // Delete the old sequence number from the IBLT. If oldSequenceNo is zero, we
  // don't need to delete it, because we don't insert a prefix with sequence
  // number zero in the IBLT.
  if (oldSequenceNo != 0)
    fullPSync_->removeName(Name(prefix).appendNumber(oldSequenceNo));

  return true;
}

}
