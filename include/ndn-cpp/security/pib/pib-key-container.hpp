/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/key-container.hpp
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

#ifndef NDN_PIB_KEY_CONTAINER_HPP
#define NDN_PIB_KEY_CONTAINER_HPP

#include <set>
#include <map>
#include "pib-key.hpp"

// Give friend access to the tests.
class TestPibKeyContainer_Basic_Test;
class TestPibKeyContainer_Errors_Test;

namespace ndn {

class PibKeyImpl;

/**
 * A PibKeyContainer is used to search/enumerate the keys of an identity.
 * (A PibKeyContainer object can only be created by PibIdentity.)
 */
class PibKeyContainer {
public:
  /**
   * Get the number of keys in the container.
   * @return The number of keys.
   */
  size_t
  size() const { return keyNames_.size(); }

  /**
   * Add a key with name keyName into the container. If a key with the same name
   * already exists, this replaces it.
   * @param key The array of encoded key bytes.
   * @param keyLength The number of bytes in the key array.
   * @param keyName The name of the key, which is copied.
   * @return The PibKey object.
   * @throws std::invalid_argument if the name of the key does not match the
   * identity name.
   */
  ptr_lib::shared_ptr<PibKey>
  add(const uint8_t* key, size_t keyLength, const Name& keyName);

  /**
   * Remove the key with name keyName from the container, and its related
   * certificates. If the key does not exist, do nothing.
   * @param keyName The name of the key.
   * @throws std::invalid_argument if keyName does not match the identity name.
   */
  void
  remove(const Name& keyName);

  /**
   * Get the key with name keyName from the container.
   * @param keyName The name of the key.
   * @return The PibKey object.
   * @throws std::invalid_argument if keyName does not match the identity name.
   * @throws Pib::Error if the key does not exist.
   */
  ptr_lib::shared_ptr<PibKey>
  get(const Name& keyName);

  /**
   * Get the names of all the keys in the container.
   * @return A new list of Name.
   */
  ptr_lib::shared_ptr<std::vector<Name> >
  getKeyNames() const;

  /**
   * Check if the container is consistent with the backend storage.
   * @return True if the container is consistent, false otherwise.
   * @note This method is heavy-weight and should be used in a debugging mode
   * only.
   */
  bool
  isConsistent() const;

private:
  friend class PibIdentityImpl;
  // Give friend access to the tests.
  friend class ::TestPibKeyContainer_Basic_Test;
  friend class ::TestPibKeyContainer_Errors_Test;

  /**
   * Create a PibKeyContainer for an identity with identityName. This
   * constructor should only be called by PibIdentityImpl.
   * @param identityName The name of the identity, which is copied.
   * @param pibImpl The PIB backend implementation.
   */
  PibKeyContainer
    (const Name& identityName, const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  // Disable the copy constructor and assignment operator.
  PibKeyContainer(const PibKeyContainer& other);
  PibKeyContainer& operator=(const PibKeyContainer& other);

  Name identityName_;
  std::set<Name> keyNames_;
  // Cache of loaded PibKeyImpl objects.
  std::map<Name, ptr_lib::shared_ptr<PibKeyImpl>> keys_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
