/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/full-producer-arbitrary.hpp
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

#ifndef NDN_FULL_PSYNC2017_HPP
#define NDN_FULL_PSYNC2017_HPP

#include "../face.hpp"
#include "../util/segment-fetcher.hpp"
#include "../security/key-chain.hpp"
#include "psync-producer-base.hpp"

namespace ndn {

class PSyncSegmentPublisher;

/**
 * FullPSync2017 implements the full sync logic of PSync to synchronize with
 * other nodes, where all nodes want to sync all the names. The application
 * should call publishName whenever it wants to let consumers know that a new name
 * is available. Currently, fetching and publishing the data given by the
 * announced name needs to be handled by the application. The Full PSync
 * protocol is described in Section G "Full-Data Synchronization" of:
 * https://named-data.net/wp-content/uploads/2017/05/scalable_name-based_data_synchronization.pdf
 * (Note: In the PSync library, this class is called FullProducerArbitrary. But
 * because the class actually handles both producing and consuming, we omit
 * "producer" in the name to avoid confusion.)
 */
class FullPSync2017 {
public:
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<std::vector<Name>>& updates)> OnNamesUpdate;
  typedef func_lib::function<bool
    (const Name& name, const std::set<uint32_t>& negative)> CanAddToSyncData;
  typedef func_lib::function<bool(const Name& name)> CanAddReceivedName;

  /**
   * Create a FullPSync2017.
   * @param expectedNEntries The expected number of entries in the IBLT.
   * @param face The application's Face.
   * @param syncPrefix The prefix Name of the sync group, which is copied.
   * @param onNamesUpdate When there are new names, this calls
   * onNamesUpdate(names) where names is a list of Names. However, see the
   * canAddReceivedName callback which can control which names are added.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param keyChain The KeyChain for signing Data packets.
   * @param syncInterestLifetime (optional) The Interest lifetime for the sync
   * Interests, in milliseconds. If omitted, use
   * DEFAULT_SYNC_INTEREST_LIFETIME.
   * @param syncReplyFreshnessPeriod (optional) The freshness period of the sync
   * Data packet, in milliseconds. If omitted, use
   * DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD.
   * @param signingInfo (optional) The SigningInfo for signing Data packets,
   * which is copied. If omitted, use the default SigningInfo().
   * @param canAddToSyncData (optional) When a new IBLT is received in a sync
   * Interest, this calls canAddToSyncData(name, negative) where Name is the
   * candidate Name to add to the response Data packet of Names, and negative is
   * the set of names that the other's user's Name set, but not in our own Name
   * set. If the callback returns false, then this does not report the Name to
   * the other user. However, if canAddToSyncData is omitted or null, then each
   * name is reported.
   * @param canAddReceivedName (optional) When new names are received, this calls
   * canAddReceivedName(name) for each name. If the callback returns false,
   * then this does not add to the IBLT or report to the application with
   * onNamesUpdate. However, if canAddReceivedName is omitted or null, then each
   * name is added.
   */
  FullPSync2017
    (size_t expectedNEntries, Face& face, const Name& syncPrefix,
     const OnNamesUpdate& onNamesUpdate, KeyChain& keyChain,
     Milliseconds syncInterestLifetime = DEFAULT_SYNC_INTEREST_LIFETIME,
     Milliseconds syncReplyFreshnessPeriod = DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD,
     const SigningInfo& signingInfo = SigningInfo(),
     const CanAddToSyncData& canAddToSyncData = CanAddToSyncData(),
     const CanAddReceivedName& canAddReceivedName = CanAddReceivedName())
  : impl_(new Impl
          (expectedNEntries, face, syncPrefix, onNamesUpdate, keyChain,
           syncInterestLifetime, syncReplyFreshnessPeriod, signingInfo,
           canAddToSyncData, canAddReceivedName))
  {
    impl_->initialize();
  }

  /**
   * Publish the Name to inform the others. However, if the Name has already
   * been published, do nothing.
   * @param name The Name to publish.
   */
  void
  publishName(const Name& name)
  {
    impl_->publishName(name);
  }

  /**
   * Remove the Name from the IBLT so that it won't be announced to other users.
   * @param name The Name to remove.
   */
  void
  removeName(const Name& name)
  {
    impl_->removeName(name);
  }

  static const int DEFAULT_SYNC_INTEREST_LIFETIME = 1000;
  static const int DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD = 1000;

private:
  /**
   * FullPSync2017::Impl does the work of FullPSync2017. It is a separate class
   * so that FullPSync2017 can create an instance in a shared_ptr to use in
   * callbacks.
   */
  class Impl : public PSyncProducerBase {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize(). See the FullPSync2017 constructor for parameter
     * documentation.
     */
    Impl
      (size_t expectedNEntries, Face& face, const Name& syncPrefix,
       const OnNamesUpdate& onNamesUpdate, KeyChain& keyChain,
       Milliseconds syncInterestLifetime, Milliseconds syncReplyFreshnessPeriod,
       const SigningInfo& signingInfo, const CanAddToSyncData& canAddToSyncData,
       const CanAddReceivedName& canAddReceivedName);

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize();

    void
    publishName(const Name& name);

    void
    removeName(const Name& name) { removeFromIblt(name); }

  private:
    class PendingEntryInfoFull {
    public:
      PendingEntryInfoFull
        (const ptr_lib::shared_ptr<InvertibleBloomLookupTable> iblt)
      : iblt_(iblt), isRemoved_(false)
      {}

      ptr_lib::shared_ptr<InvertibleBloomLookupTable> iblt_;
      bool isRemoved_;
    };

    /**
     * Send the sync interest for full synchronization. This forms the interest
     * name: /<sync-prefix>/<own-IBLT>. This cancels any pending sync interest
     * we sent earlier on the face.
     */
    void
    sendSyncInterest();

    /**
     * Process a sync interest received from another party.
     * This gets the difference between our IBLT and the IBLT in the other sync
     * interest. If we cannot get the difference successfully, then send an
     * application Nack. If we have some things in our IBLT that the other side
     * does not have, then reply with the content. Or, if the number of
     * different items is greater than threshold or equals zero, then send a
     * Nack. Otherwise add the sync interest into the pendingEntries_ map with
     * the interest name as the key and a PendingEntryInfoFull as the value.
     * @param prefixName The prefix Name for the sync group which we registered.
     * @param interest The the received Interest.
     */
    void
    onSyncInterest
      (const ptr_lib::shared_ptr<const Name>& prefixName,
       const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
       uint64_t interestFilterId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter);

    static void
    onError(SegmentFetcher::ErrorCode errorCode, const std::string& message);

    /**
     * Send the sync Data. Check if the data will satisfy our own pending 
     * Interest. If it does, then remove it and then renew the sync interest.
     * Otherwise, just send the Data.
     * @param name The basis to use for the Data name.
     * @param content The content of the Data.
     */
    void
    sendSyncData(const Name& name, Blob content);

    /**
     * Process the sync data after the content is assembled by the
     * SegmentFetcher. Call deletePendingInterests to delete any pending sync
     * Interest with the Interest name, which would have been satisfied by the
     * forwarder once it got the data. For each name in the data content, check
     * that we don't already have the name, and call _canAddReceivedName (which
     * may process the name as a prefix/sequenceNo). Call onUpdate_ to notify 
     * the application about the updates. Call sendSyncInterest because the last
     * one was satisfied by the incoming data.
     * @param encodedContent The encoded sync data content that was assembled by
     * the SegmentFetcher.
     * @param interest The Interest for which we got the data.
     */
    void
    onSyncData(Blob encodedContent, ptr_lib::shared_ptr<Interest>& interest);

    /**
     * Satisfy pending sync Interests. For a pending sync interests, if the
     * IBLT of the sync Interest has any difference from our own IBLT, then
     * send a Data back. If we can't decode the difference from the stored IBLT,
     * then delete it.
     */
    void
    satisfyPendingInterests();

    /**
     * Delete pending sync Interests that match the given name.
     */
    void
    deletePendingInterests(const Name& interestName);

    /**
     * Remove the entry from pendingEntries_ which has the name. However, if
     * entry->isRemoved_ is true, do nothing. Therefore, if an entry is
     * directly removed from pendingEntries_, it should set isRemoved_.
     * @param name The key in the pendingEntries_ map for the entry to remove.
     * @param entry A (possibly earlier and removed) entry from when it was
     * inserted into the pendingEntries_ map.
     * @param nonce This is only used for the log message.
     */
    void
    delayedRemovePendingEntry
      (const Name& name, const ptr_lib::shared_ptr<PendingEntryInfoFull>& entry,
       const Blob& nonce);

    Face& face_;
    KeyChain& keyChain_;
    SigningInfo signingInfo_;
    ptr_lib::shared_ptr<PSyncSegmentPublisher> segmentPublisher_;
    std::map<Name, ptr_lib::shared_ptr<PendingEntryInfoFull> > pendingEntries_;
    Milliseconds syncInterestLifetime_;
    OnNamesUpdate onNamesUpdate_;
    CanAddToSyncData canAddToSyncData_;
    CanAddReceivedName canAddReceivedName_;
    Name outstandingInterestName_;
    uint64_t registeredPrefix_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
