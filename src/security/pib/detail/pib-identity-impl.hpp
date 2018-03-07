/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/detail/identity-impl.hpp
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

#ifndef NDN_PIB_IDENTITY_IMPL_HPP
#define NDN_PIB_IDENTITY_IMPL_HPP

#include <ndn-cpp/security/pib/pib-key-container.hpp>

// Give friend access to the tests.
class TestPibIdentityImpl_KeyOperation_Test;

namespace ndn {

class PibKey;

/**
 * PibIdentityImpl provides the backend implementation for PibIdentity. A
 * PibIdentity has only one backend instance, but may have multiple frontend
 * handles. Each frontend handle is associated with the only one backend
 * PibIdentityImpl.
 */
class PibIdentityImpl {
public:
  /**
   * Create a PibIdentityImpl with identityName.
   * @param identityName The name of the identity, which is copied.
   * @param pibImpl The Pib backend implementation.
   * @param needInit If true and the identity does not exist in the pibImpl back 
   * end, then create it (and If no default identity has been set, identityName
   * becomes the default). If false, then throw Pib::Error if the identity does
   * not exist in the pibImpl back end.
   * @throws Pib::Error if the identity does not exist in the pibImpl back end
   * and needInit is false.
   */
  PibIdentityImpl
    (const Name& identityName, const ptr_lib::shared_ptr<PibImpl>& pibImpl,
     bool needInit);

  /*
   * Get the name of the identity.
   * @return The name of the identity.
   */
  const Name&
  getName() { return identityName_; }

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
   * Get the key with name keyName.
   * @param keyName The name of the key.
   * @return The PibKey object.
   * @throws std::invalid_argument if keyName does not match the identity name.
   * @throws Pib::Error if the key does not exist.
   */
  ptr_lib::shared_ptr<PibKey>
  getKey(const Name& keyName);

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
   * Get the default key of this Identity.
   * @return The default PibKey.
   * @throws Pib::Error if the default key has not been set.
   */
  ptr_lib::shared_ptr<PibKey>&
  getDefaultKey();

private:
  friend class PibIdentity;
  // Give friend access to the tests.
  friend TestPibIdentityImpl_KeyOperation_Test;

  // Disable the copy constructor and assignment operator.
  PibIdentityImpl(const PibIdentityImpl& other);
  PibIdentityImpl& operator=(const PibIdentityImpl& other);

  Name identityName_;
  ptr_lib::shared_ptr<PibKey> defaultKey_;

  PibKeyContainer keys_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
