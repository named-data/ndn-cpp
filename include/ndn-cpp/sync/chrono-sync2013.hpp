/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from ChronoChat-js by Qiuhan Ding and Wentao Shang.
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

#ifndef NDN_CHRONO_SYNC2013_HPP
#define NDN_CHRONO_SYNC2013_HPP

#include <vector>
#include "../face.hpp"
#include "../security/key-chain.hpp"
#include "../util/memory-content-cache.hpp"

namespace google { namespace protobuf { template <typename Element> class RepeatedPtrField; } }
namespace Sync { class SyncStateMsg; }
namespace Sync { class SyncState; }

namespace ndn {

class DigestTree;

/**
 * ChronoSync2013 implements the NDN ChronoSync protocol as described in the
 * 2013 paper "Let's ChronoSync: Decentralized Dataset State Synchronization in
 * Named Data Networking". http://named-data.net/publications/chronosync .
 * @note The support for ChronoSync is experimental and the API is not finalized.
 * See the API docs for more detail at
 * http://named-data.net/doc/ndn-ccl-api/chrono-sync2013.html .
 */
class ChronoSync2013 {
public:
  class SyncState;
  typedef func_lib::function<void
    (const std::vector<ChronoSync2013::SyncState>& syncStates, bool isRecovery)>
      OnReceivedSyncState;

  typedef func_lib::function<void()> OnInitialized;

  /**
   * Create a new ChronoSync2013 to communicate using the given face. Initialize
   * the digest log with a digest of "00" and and empty content. Register the
   * applicationBroadcastPrefix to receive interests for sync state messages and
   * express an interest for the initial root digest "00".
   * @note Your application must call processEvents. Since processEvents
   * modifies the internal ChronoSync data structures, your application should
   * make sure that it calls processEvents in the same thread as this
   * constructor (which also modifies the data structures).
   * @param onReceivedSyncState When ChronoSync receives a sync state message,
   * this calls onReceivedSyncState(syncStates, isRecovery) where syncStates is the
   * list of SyncState messages and isRecovery is true if this is the initial
   * list of SyncState messages or from a recovery interest. (For example, if
   * isRecovery is true, a chat application would not want to re-display all
   * the associated chat messages.) The callback should send interests to fetch
   * the application data for the sequence numbers in the sync state.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onInitialized This calls onInitialized() when the first sync data
   * is received (or the interest times out because there are no other
   * publishers yet).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param applicationDataPrefix The prefix used by this application instance
   * for application data. For example, "/my/local/prefix/ndnchat4/0K4wChff2v".
   * This is used when sending a sync message for a new sequence number.
   * In the sync message, this uses applicationDataPrefix.toUri().
   * @param applicationBroadcastPrefix The broadcast name prefix including the
   * application name. For example, "/ndn/broadcast/ChronoChat-0.3/ndnchat1".
   * This makes a copy of the name.
   * @param sessionNo The session number used with the applicationDataPrefix in
   * sync state messages.
   * @param face The Face for calling registerPrefix and expressInterest. The
   * Face object must remain valid for the life of this ChronoSync2013 object.
   * @param keyChain To sign a data packet containing a sync state message, this
   * calls keyChain.sign(data, certificateName).
   * @param certificateName The certificate name of the key to use for signing a
   * data packet containing a sync state message.
   * @param syncLifetime The interest lifetime in milliseconds for sending
   * sync interests.
   * @param onRegisterFailed If failed to register the prefix to receive
   * interests for the applicationBroadcastPrefix, this calls
   * onRegisterFailed(applicationBroadcastPrefix).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param previousSequenceNumber (optional) The previously published sequence
   * number for the same applicationDataPrefix and sessionNo. This can be used
   * by the application to restore the state from a previous use. If omitted,
   * this uses -1 so that the next published sequence number is 0.
   */
  ChronoSync2013
    (const OnReceivedSyncState& onReceivedSyncState,
     const OnInitialized& onInitialized, const Name& applicationDataPrefix,
     const Name& applicationBroadcastPrefix, int sessionNo,
     Face& face, KeyChain& keyChain, const Name& certificateName,
     Milliseconds syncLifetime, const OnRegisterFailed& onRegisterFailed,
     int previousSequenceNumber = -1)
  : impl_(new Impl
      (onReceivedSyncState, onInitialized, applicationDataPrefix,
       applicationBroadcastPrefix, sessionNo, face, keyChain, certificateName,
       syncLifetime, previousSequenceNumber))
  {
    impl_->initialize(onRegisterFailed);
  }

  /**
   * A SyncState holds the values of a sync state message which is passed to the
   * onReceivedSyncState callback which was given to the ChronoSyn2013
   * constructor. Note: this has the same info as the Protobuf class
   * Sync::SyncState, but we make a separate class so that we don't need the
   * Protobuf definition in the ChronoSync API.
   */
  class SyncState {
  public:
    SyncState
      (const std::string& dataPrefixUri, int sessionNo, int sequenceNo,
       const Blob& applicationInfo)
    : dataPrefixUri_(dataPrefixUri), sessionNo_(sessionNo),
      sequenceNo_(sequenceNo), applicationInfo_(applicationInfo)
    {
    }

    /**
     * Get the application data prefix.
     * @return The application data prefix as a Name URI string.
     */
    const std::string&
    getDataPrefix() const { return dataPrefixUri_; }

    /**
     * Get the session number associated with the application data prefix.
     * @return The session number.
     */
    int
    getSessionNo() const { return sessionNo_; }

    /**
     * Get the sequence number for this sync state message.
     * @return The sequence number.
     */
    int
    getSequenceNo() const { return sequenceNo_; }

    /**
     * Get the application info which was included when the sender published
     * the next sequence number.
     * @return The applicationInfo Blob. If the sender did not provide any,
     * return an isNull Blob.
     */
    const Blob&
    getApplicationInfo() const { return applicationInfo_; }

  private:
    std::string dataPrefixUri_;
    int sessionNo_;
    int sequenceNo_;
    Blob applicationInfo_;
  };

  /**
   * A PrefixAndSessionNo holds a user's data prefix and session number (used to
   * return a list from getProducerPrefixes).
   */
  class PrefixAndSessionNo {
  public:
    PrefixAndSessionNo(const std::string& dataPrefixUri, int sessionNo)
    : dataPrefixUri_(dataPrefixUri), sessionNo_(sessionNo)
    {
    }

    /**
     * Get the application data prefix for this sync state message.
     * @return The application data prefix as a Name URI string.
     */
    const std::string&
    getDataPrefix() const { return dataPrefixUri_; }

    /**
     * Get the session number associated with the application data prefix for
     * this sync state message.
     * @return The session number.
     */
    int
    getSessionNo() const { return sessionNo_; }

  private:
    std::string dataPrefixUri_;
    int sessionNo_;
  };

  /**
   * Get a copy of the current list of producer data prefixes, and the
   * associated session number. You can use these in getProducerSequenceNo().
   * This includes the prefix for this user.
   * @param prefixes This clears the vector and adds a copy of each producer
   * prefix and session number.
   */
  void
  getProducerPrefixes(std::vector<PrefixAndSessionNo>& prefixes) const
  {
    impl_->getProducerPrefixes(prefixes);
  }

  /**
   * Get the current sequence number in the digest tree for the given
   * producer dataPrefix and sessionNo.
   * @param dataPrefix The producer data prefix as a Name URI string.
   * @param sessionNo The producer session number.
   * @return The current producer sequence number, or -1 if the producer
   * namePrefix and sessionNo are not in the digest tree.
   */
  int
  getProducerSequenceNo(const std::string& dataPrefix, int sessionNo) const
  {
    return impl_->getProducerSequenceNo(dataPrefix, sessionNo);
  }

  /**
   * Increment the sequence number, create a sync message with the new
   * sequence number and publish a data packet where the name is
   * the applicationBroadcastPrefix + the root digest of the current digest
   * tree. Then add the sync message to the digest tree and digest log which
   * creates a new root digest. Finally, express an interest for the next sync
   * update with the name applicationBroadcastPrefix + the new root digest.
   * After this, your application should publish the content for the new
   * sequence number. You can get the new sequence number with getSequenceNo().
   * @note Your application must call processEvents. Since processEvents
   * modifies the internal ChronoSync data structures, your application should
   * make sure that it calls processEvents in the same thread as
   * publishNextSequenceNo() (which also modifies the data structures).
   * @param applicationInfo (optional) This appends applicationInfo to the
   * content of the sync messages. This same info is provided to the receiving
   * application in the SyncState state object provided to the
   * onReceivedSyncState callback.
   */
  void
  publishNextSequenceNo(const Blob& applicationInfo = Blob())
  {
    return impl_->publishNextSequenceNo(applicationInfo);
  }

  /**
   * Get the sequence number of the latest data published by this application
   * instance.
   * @return The sequence number.
   */
  int
  getSequenceNo() const
  {
    return impl_->getSequenceNo();
  }

  /**
   * Unregister callbacks so that this does not respond to interests anymore.
   * If you will delete this ChronoSync2013 object while your application is
   * still running, you should call shutdown() first.  After calling this, you
   * should not call publishNextSequenceNo() again since the behavior will be
   * undefined.
   * @note Because this modifies internal ChronoSync data structures, your
   * application should make sure that it calls processEvents in the same
   * thread as shutdown() (which also modifies the data structures).
   */
  void
  shutdown()
  {
    impl_->shutdown();
  }

private:
  class DigestLogEntry {
  public:
    DigestLogEntry
      (const std::string& digest,
       const google::protobuf::RepeatedPtrField<Sync::SyncState>& data);

    const std::string&
    getDigest() const { return digest_; }

    const google::protobuf::RepeatedPtrField<Sync::SyncState>&
    getData() const { return *data_; }

  private:
    std::string digest_;
    ptr_lib::shared_ptr<google::protobuf::RepeatedPtrField<Sync::SyncState> > data_;
  };

  /**
   * ChronoSync2013::Impl does the work of ChronoSync2013. It is a separate
   * class so that ChronoSync2013 can create an instance in a shared_ptr to
   * use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize().  See the ChronoSync2013 constructor for parameter
     * documentation.
     */
    Impl
      (const OnReceivedSyncState& onReceivedSyncState,
       const OnInitialized& onInitialized, const Name& applicationDataPrefix,
       const Name& applicationBroadcastPrefix, int sessionNo,
       Face& face, KeyChain& keyChain, const Name& certificateName,
       Milliseconds syncLifetime, int previousSequenceNumber);

    /**
     * Initialize the digest log with a digest of "00" and and empty content.
     * Register the applicationBroadcastPrefix to receive interests for sync
     * state messages and express an interest for the initial root digest "00".
     * You must call this after creating this Impl and making it belong to
     * a shared_ptr. This is a separate method from the constructor because
     * we need to call shared_from_this(), but in the constructor this object
     * does not yet belong to a shared_ptr.
     */
    void
    initialize(const OnRegisterFailed& onRegisterFailed);

    /**
     * See ChronoSync2013::getProducerPrefixes.
     */
    void
    getProducerPrefixes(std::vector<PrefixAndSessionNo>& prefixes) const;

    /**
     * See ChronoSync2013::getProducerSequenceNo.
     */
    int
    getProducerSequenceNo(const std::string& dataPrefix, int sessionNo) const;

    /**
     * See ChronoSync2013::publishNextSequenceNo.
     */
    void
    publishNextSequenceNo(const Blob& applicationInfo);

    /**
     * See ChronoSync2013::getSequenceNo.
     */
    int
    getSequenceNo() const { return sequenceNo_; }

    /**
     * See ChronoSync2013::shutdown.
     */
    void
    shutdown()
    {
      enabled_ = false;
      contentCache_.unregisterAll();
    }

  private:
    /**
     * Make a data packet with the syncMessage and with name
     * applicationBroadcastPrefix_ + digest. Sign and send.
     * @param digest The root digest as a hex string for the data packet name.
     * @param syncMessage The SyncStateMsg which updates the digest tree state
     * with the given digest.
     */
    void
    broadcastSyncState
      (const std::string& digest, const Sync::SyncStateMsg& syncMessage);

    /**
     * Update the digest tree with the messages in content. If the digest tree
     * root is not in the digest log, also add a log entry with the content.
     * @param content The sync state messages.
     * @return True if added a digest log entry (because the updated digest
     * tree root was not in the log), false if didn't add a log entry.
     */
    bool
    update(const google::protobuf::RepeatedPtrField<Sync::SyncState >& content);

    // Search the digest log by digest.
    int
    logFind(const std::string& digest) const;

    /**
     * Process the sync interest from the applicationBroadcastPrefix. If we can't
     * satisfy the interest, add it to the pending interest table in the
     * contentCache_ so that a future call to add may satisfy it.
     */
    void
    onInterest
      (const ptr_lib::shared_ptr<const Name>& prefix,
       const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
       uint64_t registerPrefixId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter);

    // Process Sync Data.
    void
    onData
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& data);

    // Initial sync interest timeout, which means there are no other publishers yet.
    void
    initialTimeOut(const ptr_lib::shared_ptr<const Interest>& interest);

    void
    processRecoveryInterest
      (const Interest& interest, const std::string& syncDigest, Face& face);

    /**
     * Common interest processing, using digest log to find the difference after
     * syncDigest. Return true if sent a data packet to satisfy the interest,
     * otherwise false.
     */
    bool
    processSyncInterest(int index, const std::string& syncDigest, Face& face);

    // Send Recovery Interest.
    void
    sendRecovery(const std::string& syncDigest);

    /**
     * This is called by onInterest after a timeout to check if a recovery is needed.
     * This method has an "interest" argument because we use it as the onTimeout
     * for Face.expressInterest. Face must be a pointer, not a reference, because
     * it is used in a function object and we don't want to copy it.
     */
    void
    judgeRecovery
      (const ptr_lib::shared_ptr<const Interest> &interest,
       const std::string& syncDigest, Face* face);

    // Sync interest time out, if the interest is the static one send again.
    void
    syncTimeout(const ptr_lib::shared_ptr<const Interest>& interest);

    // Process initial data which usually includes all other publisher's info, and send back the new comer's own info.
    void
    initialOndata(const google::protobuf::RepeatedPtrField<Sync::SyncState >& content);

    /**
     * This is a do-nothing onData for using expressInterest for timeouts.
     * This should never be called.
     */
    static void
    dummyOnData
      (const ptr_lib::shared_ptr<const Interest>& interest,
       const ptr_lib::shared_ptr<Data>& data);

    Face& face_;
    KeyChain& keyChain_;
    Name certificateName_;
    Milliseconds syncLifetime_;
    OnReceivedSyncState onReceivedSyncState_;
    OnInitialized onInitialized_;
    std::vector<ptr_lib::shared_ptr<DigestLogEntry> > digestLog_;
    ptr_lib::shared_ptr<DigestTree> digestTree_;
    std::string applicationDataPrefixUri_;
    const Name applicationBroadcastPrefix_;
    int sessionNo_;
    int initialPreviousSequenceNo_;
    int sequenceNo_;
    MemoryContentCache contentCache_;
    bool enabled_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
