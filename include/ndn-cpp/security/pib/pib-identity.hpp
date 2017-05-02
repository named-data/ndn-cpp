/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/identity.hpp
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

#include "pib-key-container.hpp"

/**
 * The PibIdentity class provides access to an identity at the top level in the
 * PIB's Identity-Key-Certificate hierarchy. A PibIdentity object has a Name,
 * and contains zero or more PibKey objects, at most one of which is set as the
 * default key of this identity. A key can be directly accessed by getting a
 * PibKey object.
 */
#ifndef NDN_PIB_IDENTITY_HPP
#define NDN_PIB_IDENTITY_HPP

namespace ndn {

class PibIdentityImpl;

class PibIdentity {
public:
  /*
   * Get the name of the identity.
   * @return The name of the identity.
   * @throws std::domain_error the backend implementation instance is invalid.
   */
  const Name&
  getName() const;

  /**
   * Get the key with name keyName.
   * @param keyName The name of the key.
   * @return The PibKey object.
   * @throws std::invalid_argument if keyName does not match the identity name.
   * @throws Pib::Error if the key does not exist.
   */
  ptr_lib::shared_ptr<PibKey>
  getKey(const Name& keyName) const;

  /**
   * Get the default key of this Identity.
   * @throws Pib::Error if the default key has not been set.
   */
  const ptr_lib::shared_ptr<PibKey>&
  getDefaultKey() const;

private:
  friend class PibIdentityContainer;

  /**
   * Create a PibIdentity which uses the impl backend implementation. This
   * constructor should only be called by PibIdentityContainer.
   */
  PibIdentity(ptr_lib::weak_ptr<PibIdentityImpl> impl);

  /**
   * Check the validity of the impl_ instance.
   * @return A shared_ptr when the instance is valid.
   * @throws std::domain_error the backend implementation instance is invalid.
   */
  ptr_lib::shared_ptr<PibIdentityImpl>
  lock() const;

  // Disable the copy constructor and assignment operator.
  PibIdentity(const PibIdentity& other);
  PibIdentity& operator=(const PibIdentity& other);

  ptr_lib::weak_ptr<PibIdentityImpl> impl_;
};

}

#endif
