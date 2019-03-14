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

#include "full-psync2017.hpp"
#include "psync-missing-data-info.hpp"

namespace ndn {

class PSyncUserPrefixes;

/**
 * FullPSync2017WithUsers uses FullPSync2017 to implement the full sync logic of
 * PSync to synchronize with other nodes, where all nodes want to sync the
 * sequence number of all users based on their user prefix. The application
 * should call publishName whenever it wants to let consumers know that new data
 * with a new sequence number is available for the user prefix. Multiple user
 * prefixes can be added by using addUserNode. Currently, fetching and
 * publishing the data (named by the user prefix plus the sequence number) needs
 * to be handled by the application. See FullPSync2017 for details on the
 * Full PSync protocol. The Full PSync
 * protocol is described in Section G "Full-Data Synchronization" of:
 * https://named-data.net/wp-content/uploads/2017/05/scalable_name-based_data_synchronization.pdf
 * (Note: In the PSync library, this class is called FullProducer. But because
 * the class actually handles both producing and consuming, we omit "producer"
 * in the name to avoid confusion.)
 */
class FullPSync2017WithUsers {
public:
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<std::vector<ptr_lib::shared_ptr<PSyncMissingDataInfo>>>& updates)> OnUpdate;

  /**
   * Create a FullPSync2017WithUsers.
   * @param expectedNEntries The expected number of entries in the IBLT.
   * @param face The application's Face.
   * @param syncPrefix The prefix Name of the sync group, which is copied.
   * @param userPrefix The prefix Name of the first user in the group, which is
   * copied. However, if this Name is empty, it is not added and you must call
   * addUserNode.
   * @param onUpdate When there is new data, this calls onUdate(updates) where
   * updates is a list of PSyncMissingDataInfo.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param keyChain The KeyChain for signing Data packets.
   * @param syncInterestLifetime (optional) The Interest lifetime for the sync
   * Interests, in milliseconds. If omitted, use
   * FullPSync2017::DEFAULT_SYNC_INTEREST_LIFETIME.
   * @param syncReplyFreshnessPeriod (optional) The freshness period of the sync
   * Data packet, in milliseconds. If omitted, use
   * FullPSync2017::DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD.
   * @param signingInfo (optional) The SigningInfo for signing Data packets,
   * which is copied. If omitted, use the default SigningInfo().
   */
  FullPSync2017WithUsers
    (size_t expectedNEntries, Face& face, const Name& syncPrefix,
     const Name& userPrefix, const OnUpdate& onUpdate, KeyChain& keyChain,
     Milliseconds syncInterestLifetime = FullPSync2017::DEFAULT_SYNC_INTEREST_LIFETIME,
     Milliseconds syncReplyFreshnessPeriod = FullPSync2017::DEFAULT_SYNC_REPLY_FRESHNESS_PERIOD,
     const SigningInfo& signingInfo = SigningInfo())
  : impl_(new Impl(onUpdate))
  {
    impl_->initialize
      (expectedNEntries, face, syncPrefix, userPrefix, keyChain,
       syncInterestLifetime, syncReplyFreshnessPeriod, signingInfo);
  }

  /**
   * Return the current sequence number of the given user prefix.
   * @param prefix The user prefix for the sequence number.
   * @return The sequence number for the user prefix, or -1 if not found.
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
   * Publish the sequence number for the prefix Name to inform the others.
   * (addUserNode needs to be called before this to add the prefix, if it was
   * not already added via the constructor.)
   * @param prefix the prefix Name to be updated.
   * @param sequenceNo (optional) The sequence number of the user prefix to be
   * set in the IBLT. However, if sequenceNo is omitted or -1, then the existing
   * sequence number is incremented by 1.
   */
  void
  publishName(const Name& prefix, int sequenceNo = -1)
  {
    impl_->publishName(prefix, sequenceNo);
  }

private:
  /**
   * FullPSync2017WithUsers::Impl does the work of FullPSync2017WithUsers. It is a
   * separate class so that FullPSync2017WithUsers can create an instance in a
   * shared_ptr to use in callbacks.
   */
  class Impl : public ptr_lib::enable_shared_from_this<Impl> {
  public:
    /**
     * Create a new Impl, which should belong to a shared_ptr. Then you must
     * call initialize(). See the FullPSync2017WithUsers constructor for parameter
     * documentation.
     */
    Impl(const OnUpdate& onUpdate);

    /**
     * Complete the work of the constructor. This is needed because we can't
     * call shared_from_this() in the constructor.
     */
    void
    initialize
      (size_t expectedNEntries, Face& face, const Name& syncPrefix,
       const Name& userPrefix, KeyChain& keyChain, 
       Milliseconds syncInterestLifetime, Milliseconds syncReplyFreshnessPeriod,
       const SigningInfo& signingInfo);

    int
    getSequenceNo(const Name& prefix) const;

    bool
    addUserNode(const Name& prefix);

    void
    removeUserNode(const Name& prefix);

    void
    publishName(const Name& prefix, int sequenceNo);

  private:
    /**
     * This is called when new names are received to check if the name can be
     * added to the IBLT.
     * @param name The Name to check.
     * @return True if the received name can be added.
     */
    bool
    canAddReceivedName(const Name& name);

    /**
     * This is called when new names are received. Update prefixes_, create the
     * list of PSyncMissingDataInfo and call the onUpdate_ callback.
     * @param names The new received names.
     */
    void
    onNamesUpdate(const ptr_lib::shared_ptr<std::vector<Name>>& names);

    /**
     * Get the prefix from the name and check if hash(prefix + 1) is in the
     * negative set. (Sometimes the Interest from the other side gets to us
     * before the Data.)
     * @return True if hash(prefix + 1) is NOT in the negative set (meaning that
     * it is not a future hash), or false if it IS in the negative set.
     */
    bool
    isNotFutureHash(const Name& name, const std::set<uint32_t>& negative);

    /**
     * Update prefixes_ and iblt_ with the given prefix and sequence number.
     * Whoever calls this needs to make sure that prefix is in prefixes_.
     * We remove an already-existing prefix/sequence number from iblt_ (unless
     * sequenceNo is zero because we don't insert a zero sequence number into the
     * IBLT.) Then we update prefixes_. If this returns true, the caller should
     * update  nameToHash_, hashToName_ and iblt_ .
     * @param prefix The prefix of the sequence number to update.
     * @param sequenceNumber The new sequence number.
     * @return True if the prefixes_ were updated, false if not.
     */
    bool
    updateSequenceNo(const Name& prefix, int sequenceNo);

    OnUpdate onUpdate_;
    ptr_lib::shared_ptr<FullPSync2017> fullPSync_;
    ptr_lib::shared_ptr<PSyncUserPrefixes> prefixes_;
  };

  ptr_lib::shared_ptr<Impl> impl_;
};

}

#endif
