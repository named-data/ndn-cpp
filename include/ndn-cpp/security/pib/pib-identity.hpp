/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/identity.hpp
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

/**
 * The PibIdentity class provides access to an identity at the top level in the
 * PIB's Identity-Key-Certificate hierarchy. A PibIdentity object has a Name,
 * and contains zero or more PibKey objects, at most one of which is set as the
 * default key of this identity. A key can be directly accessed by getting a
 * PibKey object.
 */
#ifndef NDN_PIB_IDENTITY_HPP
#define NDN_PIB_IDENTITY_HPP

#include "pib-key-container.hpp"

// Give friend access to the tests.
class TestKeyChain_Management_Test;

namespace ndn {

class PibIdentityImpl;

/**
 * PibIdentity is at the top level in PIB's Identity-Key-Certificate hierarchy.
 * An identity has a Name, and contains zero or more keys, at most one of which
 * is set as the default key of this identity.  Properties of a key can be
 * accessed after obtaining a PibKey object.
 */
class PibIdentity {
public:
  /*
   * Get the name of the identity.
   * @return The name of the identity.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  const Name&
  getName();

  /**
   * Get the key with name keyName.
   * @param keyName The name of the key.
   * @return The PibKey object.
   * @throws std::domain_error if the backend implementation instance is invalid.
   * @throws std::invalid_argument if keyName does not match the identity name.
   * @throws Pib::Error if the key does not exist.
   */
  ptr_lib::shared_ptr<PibKey>
  getKey(const Name& keyName);

  /**
   * Get the default key of this Identity.
   * @return The default PibKey.
   * @throws std::domain_error if the backend implementation instance is invalid.
   * @throws Pib::Error if the default key has not been set.
   */
  ptr_lib::shared_ptr<PibKey>&
  getDefaultKey();

private:
  friend class PibIdentityContainer;
  friend class KeyChain;
  // Give friend access to the tests.
  friend class ::TestKeyChain_Management_Test;

  /**
   * Create a PibIdentity which uses the impl backend implementation. This
   * constructor should only be called by PibIdentityContainer.
   */
  PibIdentity(ptr_lib::weak_ptr<PibIdentityImpl> impl);

  /**
   * Add the key. If a key with the same name already exists, overwrite the key.
   * If no default key for the identity has been set, then set the added key as
   * default for the identity.
   * @param key The public key bits. This copies the array.
   * @param keyLength The length of the public key bits array.
   * @param keyName The name of the key. This copies the name.
   * @return The PibKey object.
   */
  ptr_lib::shared_ptr<PibKey>
  addKey(const uint8_t* key, size_t keyLength, const Name& keyName);

  /**
   * Remove the key with keyName and its related certificates. If the key does
   * not exist, do nothing.
   * @param keyName The name of the key.
   */
  void
  removeKey(const Name& keyName);

  /**
   * Set the key with name keyName as the default key of the identity.
   * @param keyName The name of the key. This copies the name.
   * @return The PibKey object of the default key.
   * @throws std::invalid_argument if the name of the key does not match the
   * identity name.
   * @throws Pib::Error if the key does not exist.
   */
  ptr_lib::shared_ptr<PibKey>&
  setDefaultKey(const Name& keyName);

  /**
   * Add a key with name keyName and set it as the default key of the identity.
   * @param key The array of encoded key bytes.
   * @param keyLength The number of bytes in the key array.
   * @param keyName The name of the key, which is copied.
   * @return The PibKey object of the default key.
   * @throws std::invalid_argument if the name of the key does not match the
   * identity name.
   * @throws Pib::Error if a key with the same name already exists.
   */
  ptr_lib::shared_ptr<PibKey>&
  setDefaultKey(const uint8_t* key, size_t keyLength, const Name& keyName);

  /**
   * Get the PibKeyContainer in the PibIdentityImpl. This should only be called
   * by KeyChain.
   */
  PibKeyContainer&
  getKeys();

  /**
   * Check the validity of the impl_ instance.
   * @return A shared_ptr when the instance is valid.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  ptr_lib::shared_ptr<PibIdentityImpl>
  lock();

  // Disable the copy constructor and assignment operator.
  PibIdentity(const PibIdentity& other);
  PibIdentity& operator=(const PibIdentity& other);

  ptr_lib::weak_ptr<PibIdentityImpl> impl_;
};

}

#endif
