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

#ifndef NDN_REGISTERED_PREFIX_TABLE_HPP
#define NDN_REGISTERED_PREFIX_TABLE_HPP

#include <ndn-cpp/face.hpp>
#include "interest-filter-table.hpp"

namespace ndn {

/**
 * A RegisteredPrefixTable is an internal class to hold a list of registered
 * prefixes with information necessary to remove the registration later.
 */
class RegisteredPrefixTable {
public:
  /**
   * Create a new RegisteredPrefixTable with an empty table.
   * @param interestFilterTable See removeRegisteredPrefix(), which may call
   * interestFilterTable.unsetInterestFilter().
   */
  RegisteredPrefixTable(InterestFilterTable& interestFilterTable)
  : interestFilterTable_(interestFilterTable)
  {
  }

  /**
   * Add a new entry to the table. However, if removeRegisteredPrefix was already
   * called with the registeredPrefixId, don't add an entry and return false.
   * @param registeredPrefixId The ID from Node::getNextEntryId().
   * @param prefix A shared_ptr for the prefix.
   * @param relatedInterestFilterId (optional) The related interestFilterId
   * for the filter set in the same registerPrefix operation. If omitted, set
   * to 0.
   * @return True if added an entry, false if removeRegisteredPrefix was already
   * called with the registeredPrefixId.
   */
  bool
  add(uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
      uint64_t relatedInterestFilterId);

  /**
   * Remove the registered prefix entry with the registeredPrefixId from the
   * registered prefix table. This does not affect another registered prefix with
   * a different registeredPrefixId, even if it has the same prefix name. If an
   * interest filter was automatically created by registerPrefix, also call
   * interestFilterTable_.unsetInterestFilter to remove it.
   * If there is no entry with the registeredPrefixId, do nothing.
   * @param registeredPrefixId The ID returned from registerPrefix.
   */
  void
  removeRegisteredPrefix(uint64_t registeredPrefixId);

private:
  /**
   * A RegisteredPrefixTable::Entry holds a registeredPrefixId and information
   * necessary to remove the registration later. It optionally holds a related
   * interestFilterId if the InterestFilter was set in the same registerPrefix
   * operation.
   */
  class Entry {
  public:
    /**
     * Create a new RegisteredPrefixTable::Entry with the given values.
     * @param registeredPrefixId The ID from Node::getNextEntryId().
     * @param prefix A shared_ptr for the prefix.
     * @param relatedInterestFilterId (optional) The related interestFilterId
     * for the filter set in the same registerPrefix operation. If omitted, set
     * to 0.
     */
    Entry
      (uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
       uint64_t relatedInterestFilterId)
    : registeredPrefixId_(registeredPrefixId), prefix_(prefix),
      relatedInterestFilterId_(relatedInterestFilterId)
    {
    }

    /**
     * Get the registeredPrefixId given to the constructor.
     * @return The registeredPrefixId.
     */
    uint64_t
    getRegisteredPrefixId() { return registeredPrefixId_; }

    /**
     * Get the name prefix given to the constructor.
     * @return The name prefix.
     */
    const ptr_lib::shared_ptr<const Name>&
    getPrefix() { return prefix_; }

    /**
     * Get the related interestFilterId given to the constructor.
     * @return The related interestFilterId.
     */
    uint64_t
    getRelatedInterestFilterId() { return relatedInterestFilterId_; }

  private:
    uint64_t registeredPrefixId_; /**< A unique identifier for this entry so it can be deleted */
    ptr_lib::shared_ptr<const Name> prefix_;
    uint64_t relatedInterestFilterId_;
  };

  std::vector<ptr_lib::shared_ptr<Entry> > table_;
  InterestFilterTable& interestFilterTable_;
  std::vector<uint64_t> removeRequests_;
};

}

#endif
