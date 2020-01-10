/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/full-producer-arbitrary.cpp
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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_LIBZ 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_LIBZ

#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "./detail/psync-segment-publisher.hpp"
#include "./detail/invertible-bloom-lookup-table.hpp"
#include "./detail/psync-state.hpp"
#include "./detail/psync-segment-publisher.hpp"
#include "./detail/invertible-bloom-lookup-table.hpp"
#include <ndn-cpp/sync/full-psync2017.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.FullPSync2017");

namespace ndn {

FullPSync2017::Impl::Impl
  (size_t expectedNEntries, Face& face, const Name& syncPrefix,
   const OnNamesUpdate& onNamesUpdate, KeyChain& keyChain,
   Milliseconds syncInterestLifetime, Milliseconds syncReplyFreshnessPeriod,
   const SigningInfo& signingInfo, const CanAddToSyncData& canAddToSyncData,
   const CanAddReceivedName& canAddReceivedName)
: PSyncProducerBase(expectedNEntries, syncPrefix, syncReplyFreshnessPeriod),
  face_(face), keyChain_(keyChain), syncInterestLifetime_(syncInterestLifetime),
  signingInfo_(signingInfo), onNamesUpdate_(onNamesUpdate),
  canAddToSyncData_(canAddToSyncData), canAddReceivedName_(canAddReceivedName),
  segmentPublisher_(new PSyncSegmentPublisher(face_, keyChain_))
{
}

void
FullPSync2017::Impl::initialize()
{
  registeredPrefix_ = face_.registerPrefix
    (syncPrefix_,
     bind(&FullPSync2017::Impl::onSyncInterest,
     static_pointer_cast<FullPSync2017::Impl>(shared_from_this()),
     _1, _2, _3, _4, _5),
     &PSyncProducerBase::onRegisterFailed);

  // TODO: Should we do this after the registerPrefix onSuccess callback?
  sendSyncInterest();
}

void
FullPSync2017::Impl::publishName(const Name& name)
{
  if (nameToHash_.find(name) != nameToHash_.end()) {
    _LOG_DEBUG("Already published, ignoring: " << name);
    return;
  }

  _LOG_INFO("Publish: " << name);
  insertIntoIblt(name);
  satisfyPendingInterests();
}

void
FullPSync2017::Impl::sendSyncInterest()
{
#if 0 // Debug: Implement stopping an ongoing fetch.
  // If we send two sync interest one after the other
  // since there is no new data in the network yet,
  // when data is available it may satisfy both of them
  if (fetcher_) {
    fetcher_->stop();
  }
#endif

  // Sync Interest format for full sync: /<sync-prefix>/<ourLatestIBF>
  Name syncInterestName(syncPrefix_);

  // Append our latest IBLT.
  syncInterestName.append(iblt_->encode());

  outstandingInterestName_ = syncInterestName;

  // random1 is from 0.0 to 1.0.
  float random1;
  CryptoLite::generateRandomFloat(random1);
  // Get a jitter of +/- syncInterestLifetime_ * 0.2 .
  Milliseconds jitter = (random1 - 0.5) * (syncInterestLifetime_ * 0.2);

  face_.callLater
    (syncInterestLifetime_ / 2 + jitter, 
     bind(&FullPSync2017::Impl::sendSyncInterest,
          static_pointer_cast<FullPSync2017::Impl>(shared_from_this())));

  ptr_lib::shared_ptr<Interest> syncInterest
    (ptr_lib::make_shared<Interest>(syncInterestName));
  syncInterest->setInterestLifetimeMilliseconds(syncInterestLifetime_);
  syncInterest->setNonce(Blob((const uint8_t*)"0000", 4));
  syncInterest->refreshNonce();

  SegmentFetcher::fetch
    (face_, *syncInterest, 0,
     bind(&FullPSync2017::Impl::onSyncData,
          static_pointer_cast<FullPSync2017::Impl>(shared_from_this()), _1, syncInterest),
     &FullPSync2017::Impl::onError);

  _LOG_DEBUG("sendFullSyncInterest, nonce: " << syncInterest->getNonce().toHex() <<
             ", hash: " << syncInterestName.hash());
}

void
FullPSync2017::Impl::onError
  (SegmentFetcher::ErrorCode errorCode, const std::string& message)
{
  _LOG_TRACE("Cannot fetch sync data, error: " << errorCode <<
             " message: " << message);
}

void
FullPSync2017::Impl::onSyncInterest
  (const ptr_lib::shared_ptr<const Name>& prefixName,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  if (segmentPublisher_->replyFromStore(interest->getName()))
    return;

  Name nameWithoutSyncPrefix = interest->getName().getSubName(prefixName->size());
  Name interestName;

  if (nameWithoutSyncPrefix.size() == 1)
    // Get /<prefix>/IBLT from /<prefix>/IBLT
    interestName = interest->getName();
  else if (nameWithoutSyncPrefix.size() == 3)
    // Get /<prefix>/IBLT from /<prefix>/IBLT/<version>/<segment-no>
    interestName = interest->getName().getPrefix(-2);
  else
    return;

  Name::Component ibltName = interestName.get(-1);

  _LOG_DEBUG("Full Sync Interest received, nonce: " << interest->getNonce().toHex() <<
             ", hash:" << interestName.hash());

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> iblt
    (new InvertibleBloomLookupTable(expectedNEntries_));
  try {
    iblt->initialize(ibltName.getValue());
  } catch (const std::exception& ex) {
    _LOG_ERROR(ex.what());
    return;
  }

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> difference =
   iblt_->difference(*iblt);

  set<uint32_t> positive;
  set<uint32_t> negative;

  if (!difference->listEntries(positive, negative)) {
    _LOG_TRACE("Cannot decode differences, positive: " << positive.size() <<
            " negative: " << negative.size() << " threshold: " <<
            threshold_);

    // Send all data if greater than the threshold, or if there are neither
    // positive nor negative differences. Otherwise, continue below and send
    // the positive as usual.
    if (positive.size() + negative.size() >= threshold_ ||
        (positive.size() == 0 && negative.size() == 0)) {
      PSyncState state1;
      for (map<Name, uint32_t>::iterator entry = nameToHash_.begin();
           entry != nameToHash_.end(); ++entry)
        state1.addContent(entry->first);

      if (state1.getContent().size() > 0)
        segmentPublisher_->publish
          (interest->getName(), interest->getName(), state1.wireEncode(),
           syncReplyFreshnessPeriod_, signingInfo_);

      return;
    }
  }

  PSyncState state;
  for (set<uint32_t>::iterator hash = positive.begin(); hash != positive.end();
       ++hash) {
    Name name = hashToName_[*hash];

    if (nameToHash_.find(name) != nameToHash_.end()) {
      if (!canAddToSyncData_ || canAddToSyncData_(name, negative))
        state.addContent(name);
    }
  }

  if (state.getContent().size() > 0) {
    _LOG_DEBUG("Sending sync content: " << state.toString());
    sendSyncData(interestName, state.wireEncode());
    return;
  }

  ptr_lib::shared_ptr<PendingEntryInfoFull> entry(new PendingEntryInfoFull(iblt));
  pendingEntries_[interestName] = entry;
  face_.callLater
    (interest->getInterestLifetimeMilliseconds(),
     bind(&FullPSync2017::Impl::delayedRemovePendingEntry,
          static_pointer_cast<FullPSync2017::Impl>(shared_from_this()),
          interest->getName(), entry, interest->getNonce()));
}

void
FullPSync2017::Impl::sendSyncData(const Name& name, Blob content)
{
  _LOG_DEBUG("Checking if the Data will satisfy our own pending interest");

  Name nameWithIblt;
  nameWithIblt.append(iblt_->encode());

  // Append the hash of our IBLT so that the Data name should be different for
  // each node.
  Name dataName(Name(name).appendNumber(nameWithIblt.hash()));

  // Check if our own Interest got satisfied.
  if (outstandingInterestName_.equals(name)) {
    _LOG_DEBUG("Satisfies our own pending Interest");
    // remove outstanding interest
#if 0 // Debug: Implement stopping an ongoing fetch.
    if (fetcher_) {
      _LOG_DEBUG("Removing our pending interest from the Face (stopping fetcher)");
      fetcher_->stop();
      outstandingInterestName_ = Name();
    }
#else
    outstandingInterestName_ = Name();
#endif

    _LOG_DEBUG("Sending sync Data");

    // Send Data after removing the pending sync interest on the Face.
    segmentPublisher_->publish
      (name, dataName, content, syncReplyFreshnessPeriod_, signingInfo_);

    _LOG_TRACE("sendSyncData: Renewing sync interest");
    sendSyncInterest();
  }
  else {
    _LOG_DEBUG("Sending Sync Data for not our own Interest");
    segmentPublisher_->publish
      (name, dataName, content, syncReplyFreshnessPeriod_, signingInfo_);
  }
}

void
FullPSync2017::Impl::satisfyPendingInterests()
{
  _LOG_DEBUG("Satisfying full sync Interest: " << pendingEntries_.size());

  for (map<Name, ptr_lib::shared_ptr<PendingEntryInfoFull> >::iterator it =
         pendingEntries_.begin();
       it != pendingEntries_.end();) {
    const InvertibleBloomLookupTable& entryIblt = *it->second->iblt_;
    ptr_lib::shared_ptr<InvertibleBloomLookupTable> difference =
            iblt_->difference(entryIblt);
    set<uint32_t> positive;
    set<uint32_t> negative;

    if (!difference->listEntries(positive, negative)) {
      _LOG_TRACE("Decode failed for pending interest");
      if (positive.size() + negative.size() >= threshold_ ||
          (positive.size() == 0 && negative.size() == 0)) {
        _LOG_TRACE
          ("positive + negative > threshold or no difference can be found. Erase pending interest.");
        // Prevent delayedRemovePendingEntry from removing a new entry with the same Name.
        it->second->isRemoved_ = true;
        pendingEntries_.erase(it++);
        continue;
      }
    }

    PSyncState state;
    for (set<uint32_t>::iterator hash = positive.begin(); hash != positive.end();
         ++hash) {
      Name name = hashToName_[*hash];

      if (nameToHash_.find(name) != nameToHash_.end())
        state.addContent(name);
    }

    if (state.getContent().size() > 0) {
      _LOG_DEBUG("Satisfying sync content: " << state.toString());
      sendSyncData(it->first, state.wireEncode());
      // Prevent delayedRemovePendingEntry from removing a new entry with the same Name.
      it->second->isRemoved_ = true;
      pendingEntries_.erase(it++);
    }
    else
      ++it;
  }
}

void
FullPSync2017::Impl::deletePendingInterests(const Name& interestName)
{
  map<Name, ptr_lib::shared_ptr<PendingEntryInfoFull> >::iterator entry =
    pendingEntries_.find(interestName);
  if (entry == pendingEntries_.end())
    return;

  _LOG_TRACE("Delete pending interest: " << interestName);
  // Prevent delayedRemovePendingEntry from removing a new entry with the same Name.
  entry->second->isRemoved_ = true;
  pendingEntries_.erase(entry);
}

void
FullPSync2017::Impl::onSyncData
  (Blob encodedContent, ptr_lib::shared_ptr<Interest>& interest)
{
  deletePendingInterests(interest->getName());

  PSyncState state(encodedContent);
  ptr_lib::shared_ptr<vector<Name>> names
    (ptr_lib::make_shared<vector<Name>>());

  _LOG_DEBUG("Sync Data Received: " << state.toString());

  const std::vector<Name>& content = state.getContent();
  for (vector<Name>::const_iterator contentName = content.begin();
       contentName != content.end(); ++contentName) {
    if (nameToHash_.find(*contentName) == nameToHash_.end()) {
      _LOG_DEBUG("Checking whether to add " << *contentName);
      if (!canAddReceivedName_ || canAddReceivedName_(*contentName)) {
        _LOG_DEBUG("Adding name " << *contentName);
        names->push_back(*contentName);
        insertIntoIblt(*contentName);
      }
      // We should not call satisfyPendingSyncInterests here because we just
      // got data and deleted pending interests by calling deletePendingInterests.
      // But we might have interests which don't match this interest that might
      // not have been deleted from the pending sync interests.
    }
  }

  // We just got the data, so send a new sync Interest.
  if (names->size() > 0) {
    try {
      onNamesUpdate_(names);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Error in onUpdate: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Error in onUpdate.");
    }

    _LOG_TRACE("onSyncData: Renewing sync interest");
    sendSyncInterest();
  } else {
    _LOG_TRACE("No new update, interest nonce: " << interest->getNonce().toHex() <<
            " , hash: " << interest->getName().hash());
  }
}

void
FullPSync2017::Impl::delayedRemovePendingEntry
  (const Name& name, const ptr_lib::shared_ptr<PendingEntryInfoFull>& entry,
   const Blob& nonce)
{
  if (entry->isRemoved_)
    // A previous operation already removed this entry, so don't try again to
    // remove the entry with the Name in case it is a new entry.
    return;

  _LOG_TRACE("Remove Pending Interest " << nonce);
  entry->isRemoved_ = true;
  pendingEntries_.erase(name);
}

}

#endif // NDN_CPP_HAVE_LIBZ
