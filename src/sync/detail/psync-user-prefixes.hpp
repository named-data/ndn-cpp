/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/detail/user-prefixes.hpp
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

#ifndef NDN_PSYNC_USER_PREFIXES_HPP
#define NDN_PSYNC_USER_PREFIXES_HPP

#include <map>
#include <ndn-cpp/name.hpp>

namespace ndn {

/**
 * PSyncUserPrefixes holds the prefixes_ map from prefix to sequence number,
 * used by PSyncPartialProducer and FullPSync2017WithUsers.
 */
class PSyncUserPrefixes {
public:
  /**
   * Check if the prefix is in prefixes_.
   *
   * @param prefix The prefix to check.
   * @return True if the prefix is in prefixes_.
   */
  bool
  isUserNode(const Name& prefix) const
  {
    return prefixes_.find(prefix) != prefixes_.end();
  }

  /**
   * Return the current sequence number of the given prefix.
   * @param prefix The prefix for the sequence number.
   * @return The sequence number for the prefix, or -1 if not found.
   */
  int
  getSequenceNo(const Name& prefix) const
  {
    std::map<Name, int>::const_iterator entry = prefixes_.find(prefix);
    if (entry == prefixes_.end())
      return -1;

    return entry->second;
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
  addUserNode(const Name& prefix);

  /**
   * Remove the user node from synchronization. If the prefix is not in
   * prefixes_, then do nothing.
   * The caller should first check isUserNode(prefix) and erase the prefix from
   * the IBLT and other maps if needed.
   * @param prefix The prefix Name of the user node to be removed.
   */
  void
  removeUserNode(const Name& prefix);

  /**
   * Update prefixes_ with the given prefix and sequence number. This does not
   * update the IBLT. This logs a message for the update.
   * Whoever calls this needs to make sure that isUserNode(prefix) is true.
   * @param prefix The prefix of the update.
   * @param sequenceNo The sequence number of the update.
   * @param oldSequenceNo This sets oldSequenceNo to the old sequence number for
   * the prefix. If this method returns true and oldSequenceNo is not zero, the
   * caller can remove the old prefix from the IBLT.
   * @return True if the sequence number was updated, false if the prefix was
   * not in prefixes_, or if the sequenceNo is less than or equal to the old
   * sequence number. If this returns false, the caller should not update the
   * IBLT.
   */
  bool
  updateSequenceNo(const Name& prefix, int sequenceNo, int& oldSequenceNo);

  // The key is the prefix Name. The value is the sequence number for the prefix.
  std::map<Name, int> prefixes_;
};

}

#endif
