/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#ifndef NDN_PENDING_INTEREST_TABLE_HPP
#define NDN_PENDING_INTEREST_TABLE_HPP

#include <ndn-cpp/face.hpp>

namespace ndn {

/**
 * A PendingInterestTable is an internal class to hold a list of pending
 * interests with their callbacks.
 */
class PendingInterestTable {
public:
  /**
   * Entry holds the callbacks and other fields for an entry in the pending
   * interest table.
   */
  class Entry {
  public:
    /**
     * Create a new Entry with the given fields. Note: You should not call this
     * directly but call PendingInterestTable::add.
     */
    Entry
      (uint64_t pendingInterestId,
       const ptr_lib::shared_ptr<const Interest>& interest, const OnData& onData,
       const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack)
    : pendingInterestId_(pendingInterestId), interest_(interest), onData_(onData),
      onTimeout_(onTimeout), onNetworkNack_(onNetworkNack), isRemoved_(false)
    {
    }

    /**
     * Get the pendingInterestId given to the constructor.
     * @return The pendingInterestId.
     */
    uint64_t
    getPendingInterestId() { return pendingInterestId_; }

    /**
     * Get the interest given to the constructor (from Face.expressInterest).
     * @return The interest. NOTE: You must not change the interest object - if
     * you need to change it then make a copy.
     */
    const ptr_lib::shared_ptr<const Interest>&
    getInterest() { return interest_; }

    /**
     * Get the OnData callback given to the constructor.
     * @return The OnData callback.
     */
    const OnData&
    getOnData() { return onData_; }

    /**
     * Get the OnNetworkNack callback given to the constructor.
     * @return The OnNetworkNack callback.
     */
    const OnNetworkNack&
    getOnNetworkNack() { return onNetworkNack_; }

    /**
     * Set the isRemoved flag which is returned by getIsRemoved().
     */
    void
    setIsRemoved() { isRemoved_ = true; }

    /**
     * Check if setIsRemoved() was called.
     * @return True if setIsRemoved() was called.
     */
    bool
    getIsRemoved() { return isRemoved_; }

    /**
     * Call onTimeout_ (if defined).  This ignores exceptions from the call to
     * onTimeout_.
     */
    void
    callTimeout();

  private:
    ptr_lib::shared_ptr<const Interest> interest_;
    uint64_t pendingInterestId_;  /**< A unique identifier for this entry so it can be deleted */
    const OnData onData_;
    const OnTimeout onTimeout_;
    const OnNetworkNack onNetworkNack_;
    bool isRemoved_;
  };

  /**
   * Add a new entry to the pending interest table. However, if
   * removePendingInterest was already called with the pendingInterestId, don't
   * add an entry and return null.
   * @param pendingInterestId The getNextEntryId() for the pending interest ID
   * which Face got so it could return it to the caller.
   * @param interestCopy The Interest which was sent, which has already been
   * copied by expressInterest.
   * @param onData This calls onData when a matching data packet is received.
   * @param onTimeout Call onTimeout if the interest times out. If onTimeout is
   * an empty OnTimeout(), this does not use it.
   * @param onNetworkNack Call onNetworkNack.onNetworkNack when a network Nack
   * packet is received.
   * @return The new PendingInterestTable::Entry, or null if
   * removePendingInterest was already called with the pendingInterestId.
   */
  ptr_lib::shared_ptr<Entry>
  add(uint64_t pendingInterestId,
      const ptr_lib::shared_ptr<const Interest>& interestCopy,
      const OnData& onData, const OnTimeout& onTimeout,
      const OnNetworkNack& onNetworkNack);

  /**
   * Find all entries from the pending interest table where data conforms to
   * the entry's interest selectors, remove the entries from the table, set each
   * entry's isRemoved flag, and add to the entries list.
   * @param data The incoming Data packet to find the interest for.
   * @param entries Add matching PendingInterestTable::Entry from the pending
   * interest table.  The caller should pass in a reference to an empty vector.
   */
  void
  extractEntriesForExpressedInterest
    (const Data& data, std::vector<ptr_lib::shared_ptr<Entry> > &entries);

  /**
   * Find all entries from the pending interest table where the OnNetworkNack
   * callback is not an empty OnNetworkNack() and the entry's interest is the
   * same as the given interest, remove the entries from the table, set each
   * entry's isRemoved flag, and add to the entries list. (We don't remove the
   * entry if the OnNetworkNack callback is an empty OnNetworkNack() so that
   * OnTimeout will be called later.) The interests are the same if their
   * default wire encoding is the same (which has everything including the name,
   * nonce, link object and selectors).
   * @param interest The Interest to search for (typically from a Nack packet).
   * @param entries Add matching PendingInterestTable::Entry from the pending
   * interest table.  The caller should pass in a reference to an empty vector.
   */
  void
  extractEntriesForNackInterest
    (const Interest& interest, std::vector<ptr_lib::shared_ptr<Entry> > &entries);

  /**
   * Remove the pending interest entry with the pendingInterestId from the
   * pending interest table and set its isRemoved flag. This does not affect
   * another pending interest with a different pendingInterestId, even if it has
   * the same interest name. If there is no entry with the pendingInterestId, do
   * nothing.
   * @param pendingInterestId The ID returned from expressInterest.
   */
  void
  removePendingInterest(uint64_t pendingInterestId);

  /**
   * Remove the specific pendingInterest entry from the table and set its
   * isRemoved flag. However, if the pendingInterest isRemoved flag is already
   * true or the entry is not in the pending interest table then do nothing.
   * @return
   */
  bool
  removeEntry(const ptr_lib::shared_ptr<Entry>& pendingInterest);

private:
  std::vector<ptr_lib::shared_ptr<Entry> > table_;
  std::vector<uint64_t> removeRequests_;
};

}

#endif
