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

#ifndef NDN_INTEREST_FILTER_TABLE_HPP
#define NDN_INTEREST_FILTER_TABLE_HPP

#include <ndn-cpp/face.hpp>

namespace ndn {

/**
 * An InterestFilterTable is an internal class to hold a list of entries with
 * an interest Filter and its OnInterestCallback.
 */
class InterestFilterTable {
public:
  /**
   * An Entry holds an interestFilterId, an InterestFilter and the
   * OnInterestCallback with its related Face.
   */
  class Entry {
  public:
    /**
     * Create a new Entry with the given values.
     * @param interestFilterId The ID from Node::getNextEntryId().
     * @param filter A shared_ptr for the InterestFilter for this entry.
     * @param onInterest A function object to call when a matching data packet
     * is received.
     * @param face The face on which was called registerPrefix or
     * setInterestFilter which is passed to the onInterest callback.
     */
    Entry
      (uint64_t interestFilterId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter,
       const OnInterestCallback& onInterest, Face* face)
    : interestFilterId_(interestFilterId), filter_(filter),
      prefix_(new Name(filter->getPrefix())), onInterest_(onInterest), face_(face)
    {
    }

    /**
     * Return the interestFilterId given to the constructor.
     * @return The interestFilterId.
     */
    uint64_t
    getInterestFilterId() { return interestFilterId_; }

    /**
     * Get the InterestFilter given to the constructor.
     * @return The InterestFilter.
     */
    const ptr_lib::shared_ptr<const InterestFilter>&
    getFilter() { return filter_; }

    /**
     * Get the prefix from the filter as a shared_ptr. We keep this cached value
     * so that we don't have to copy the name to pass a shared_ptr each time
     * we call the OnInterestCallback.
     * @return The filter's prefix.
     */
    const ptr_lib::shared_ptr<const Name>&
    getPrefix() { return prefix_; }

    /**
     * Get the OnInterestCallback given to the constructor.
     * @return The OnInterest callback.
     */
    const OnInterestCallback&
    getOnInterest() { return onInterest_; }

    /**
     * Get the Face given to the constructor.
     * @return The Face.
     */
    Face&
    getFace() { return *face_; }

  private:
    uint64_t interestFilterId_;  /**< A unique identifier for this entry so it can be deleted */
    ptr_lib::shared_ptr<const InterestFilter> filter_;
    ptr_lib::shared_ptr<const Name> prefix_;
    const OnInterestCallback onInterest_;
    Face* face_;
  };

  /**
   * Add a new entry to the table.
   * @param interestFilterId The ID from Node.getNextEntryId().
   * @param filter The InterestFilter for this entry.
   * @param onInterest The callback to call.
   * @param face The face on which was called registerPrefix or
   * setInterestFilter which is passed to the onInterest callback.
   */
  void
  setInterestFilter
    (uint64_t interestFilterId,
     const ptr_lib::shared_ptr<const InterestFilter>& filterCopy,
     const OnInterestCallback& onInterest, Face* face)
  {
    table_.push_back(ptr_lib::make_shared<Entry>
      (interestFilterId, filterCopy, onInterest, face));
  }

  /**
   * Find all entries from the interest filter table where the interest conforms
   * to the entry's filter, and add to the matchedFilters list.
   * @param interest The interest which may match the filter in multiple entries.
   * @param matchedFilters Add each matching InterestFilterTable.Entry from the
   * interest filter table.  The caller should pass in a reference to an empty
   * vector.
   */
  void
  getMatchedFilters
    (const Interest& interest,
      std::vector<ptr_lib::shared_ptr<Entry> > &matchedFilters);

  /**
   * Remove the interest filter entry which has the interestFilterId from the
   * interest filter table. This does not affect another interest filter with
   * a different interestFilterId, even if it has the same prefix name.
   * If there is no entry with the interestFilterId, do nothing.
   * @param interestFilterId The ID returned from setInterestFilter.
   */
  void
  unsetInterestFilter(uint64_t interestFilterId);

private:
  std::vector<ptr_lib::shared_ptr<Entry> > table_;
};

}

#endif
