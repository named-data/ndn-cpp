/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/identity-container.hpp
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

#ifndef NDN_PIB_IDENTITY_CONTAINER_HPP
#define NDN_PIB_IDENTITY_CONTAINER_HPP

#include <set>
#include <map>
#include "pib-identity.hpp"

// Give friend access to the tests.
class TestPibIdentityContainer_Basic_Test;
class TestPibIdentityContainer_Errors_Test;
class TestKeyChain_Management_Test;

namespace ndn {

class PibImpl;
class PibIdentityImpl;

/**
 * A PibIdentityContainer is used to search/enumerate the identities in a PIB.
 * (A PibIdentityContainer object can only be created by the Pib class.)
 */
class PibIdentityContainer {
public:
  /**
   * Get the number of identities in the container.
   * @return The number of identities.
   */
  size_t
  size() const { return identityNames_.size(); }

  /**
   * Add an identity with name identityName into the container. Create the
   * identity if it does not exist.
   * @param identityName The name of the identity, which is copied.
   * @return The PibIdentity object.
   */
  ptr_lib::shared_ptr<PibIdentity>
  add(const Name& identityName);

  /**
   * Remove the identity with name identityName from the container, and its
   * related keys and certificates. If the default identity is being removed,
   * no default identity will be selected.  If the identity does not exist, do
   * nothing.
   * @param identityName The name of the identity.
   */
  void
  remove(const Name& identityName);

  /**
   * Get the identity with name identityName from the container.
   * @param identityName The name of the identity.
   * @return The PibIdentity object.
   * @throws Pib::Error if the identity does not exist.
   */
  ptr_lib::shared_ptr<PibIdentity>
  get(const Name& identityName);

  /**
   * Reset the state of the container. This method removes all loaded identities
   * and retrieves identity names from the PIB implementation.
   */
  void
  reset();

  /**
   * Check if the container is consistent with the backend storage.
   * @return True if the container is consistent, false otherwise.
   * @note This method is heavy-weight and should be used in a debugging mode
   * only.
   */
  bool
  isConsistent() const;

private:
  friend class Pib;
  // Give friend access to the tests.
  friend class ::TestPibIdentityContainer_Basic_Test;
  friend class ::TestPibIdentityContainer_Errors_Test;
  friend class ::TestKeyChain_Management_Test;

  /**
   * Create a PibIdentityContainer using to use the pibImpl backend
   * implementation. This constructor is only called by the Pib class.
   * @param pibImpl The PIB backend implementation.
   */
  PibIdentityContainer(const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  // Disable the copy constructor and assignment operator.
  PibIdentityContainer(const PibIdentityContainer& other);
  PibIdentityContainer& operator=(const PibIdentityContainer& other);

  std::set<Name> identityNames_;
  // Cache of loaded PibIdentityImpl objects.
  std::map<Name, ptr_lib::shared_ptr<PibIdentityImpl>> identities_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
