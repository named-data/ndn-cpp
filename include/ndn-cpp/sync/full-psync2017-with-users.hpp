/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/full-producer.hpp
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

#ifndef NDN_FULL_PSYNC2017_WITH_USERS_HPP
#define NDN_FULL_PSYNC2017_WITH_USERS_HPP

#include "../util/segment-fetcher.hpp"
#include "psync-missing-data-info.hpp"
#include "psync-producer-base.hpp"

namespace ndn {

/**
 * FullPSync2017WithUsers uses FullPSync2017 to implement the full sync logic of
 * PSync to synchronize with other nodes, where all nodes want to sync all the
 * names prefixes which are based on a user prefix plus a sequence number.
 * The application should call publishName whenever it wants to let consumers
 * know that new data is available for the user prefix. Multiple user prefixes
 * can be added by using addUserNode. Currently, fetching and publishing the
 * data needs to be handled by the application. See FullPSync2017 for details on
 * the protocol.
 * (Note: In the PSync library, this class is called FullProducer. But because
 * the class actually handles both producing and consuming, we omit "producer"
 * in the name to avoid confusion.)
 */
class FullPSync2017WithUsers {
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<std::vector<ptr_lib::shared_ptr<PSyncMissingDataInfo>>>& updates)> OnUpdate;

public:
  /**
   * Create a FullPSync2017WithUsers.
   * @param expectedNEntries The expected number of entries in the IBLT.
   * @param face The application's Face.
   * @param syncPrefix The prefix Name of the sync group, which is copied.
   * @param userPrefix The prefix Name of the first user in the group, which is
   * copied.
   * @param onUpdate When there is new data, this calls onUdate(updates) where
   * updates is a list of PSyncMissingDataInfo.
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
   */
  FullPSync2017WithUsers
    (size_t expectedNEntries, Face& face, const Name& syncPrefix,
     const Name& userPrefix, const OnUpdate& onUpdate, KeyChain& keyChain,
     Milliseconds syncInterestLifetime = DEFAULT_SYNC_INTEREST_LIFETIME,
     Milliseconds syncReplyFreshnessPeriod = DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD,
     const SigningInfo& signingInfo = SigningInfo())
  : impl_(new Impl
          (expectedNEntries, face, syncPrefix, userPrefix, onUpdate, keyChain,
           syncInterestLifetime, syncReplyFreshnessPeriod, signingInfo))
  {
    impl_->initialize();
  }

  /**
   * Return the current sequence number of the given prefix.
   * @param prefix The prefix for the sequence number.
   * @return The sequence number for the prefix, or -1 if not found.
   */
  int
  getSequenceNo(const Name& prefix) const
  {
    return impl_->getSequenceNo(prefix);
  }

  /**
   * Add a user node for synchronization based on the prefix Name, and
   * initialize the sequence number to zero. However, if the prefix Name already
   * exists, then do nothing and return false. This does not add sequence number
   * zero to the IBLT because, if a large number of user nodes are added, then
   * decoding the difference between our own IBLT and the other IBLT will not be
   * possible.
   * @param prefix The prefix Name of the user node to be added.
   * @return True if the user node with the prefix Name was added, false if the
   * prefix Name already exists.
   */
  bool
  addUserNode(const Name& prefix) { return impl_->addUserNode(prefix); }

  /**
   * Remove the user node from the synchronization. This erases the prefix from
   * the IBLT and other tables.
   * @param prefix The prefix Name of the user node to be removed. If there is
   * no user node with this prefix, do nothing.
   */
  void
  removeUserNode(const Name& prefix) { impl_->removeUserNode(prefix); }

  /**
   * Publish the prefix Name to inform the others. addUserNode needs to be
   * called before this to add the prefix, if it was not already added via the
   * constructor.
   * @param prefix the prefix Name to be updated.
   * @param sequenceNo (optional) The sequence number of the prefix to be set in
   * the IBLT. However, if sequenceNo is omitted or -1, then the existing
   * sequence number is incremented by 1.
   */
  void
  publishName(const Name& prefix, int sequenceNo = -1)
  {
    impl_->publishName(prefix, sequenceNo);
  }

  static const int DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD = 1000;
  static const int DEFAULT_HELLO_REPLY_FRESHNESS_PERIOD = 1000;
  static const int DEFAULT_SYNC_INTEREST_LIFETIME = 1000;

private:
  /**
   * FullPSync2017WithUsers::Impl does the work of FullPSync2017WithUsers. It is a
   * separate class so that FullPSync2017WithUsers can create an instance in a
   * shared_ptr to use in callbacks.
   */
  class Impl : public PSyncProducerBase {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize(). See the FullPSync2017WithUsers constructor for parameter
     * documentation.
     */
    Impl
      (size_t expectedNEntries, Face& face, const Name& syncPrefix,
       const Name& userPrefix, const OnUpdate& onUpdate, KeyChain& keyChain,
       Milliseconds syncInterestLifetime, Milliseconds syncReplyFreshnessPeriod,
       const SigningInfo& signingInfo);

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize();

    void
    publishName(const Name& prefix, int sequenceNo);

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
     * forwarder once it got the data. For each prefix/sequenceNo in the data
     * content,  check that we don't already have the prefix/sequenceNo, and
     * call updateSeq(prefix, sequenceNo). Call onUpdate_ to notify the
     * application about the updates. Call sendSyncInterest because the last one
     * was satisfied by the incoming data.
     * @param encodedContent The encoded sync data content that was assembled by
     * the SegmentFetcher.
     * @param interest The Interest for which we got the data.
     */
    void
    onSyncData(Blob encodedContent, ptr_lib::shared_ptr<Interest>& interest);

    /**
     * Satisfy pending sync Interests. For a pending sync interests SI, if the
     * IBLT of the sync Interest has any difference from our own IBLT, then
     * send Data back. If we can't decode the difference from the stored IBLT,
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
     * Check if hash(prefix + 1) is in the negative set. (Sometimes the Interest
     * from the other side gets to us before the Data.)
     */
    bool
    isFutureHash(const Name& prefix, const std::set<uint32_t>& negative);

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

    std::map<Name, ptr_lib::shared_ptr<PendingEntryInfoFull> > pendingEntries_;
    Milliseconds syncInterestLifetime_;
    OnUpdate onUpdate_;
    Name outstandingInterestName_;
    uint64_t registeredPrefix_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
