/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/identity.cpp
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

#include "detail/pib-identity-impl.hpp"
#include <ndn-cpp/security/pib/pib-identity.hpp>

using namespace std;

namespace ndn {

const Name&
PibIdentity::getName() { return lock()->getName(); }

ptr_lib::shared_ptr<PibKey>
PibIdentity::getKey(const Name& keyName) { return lock()->getKey(keyName); }

ptr_lib::shared_ptr<PibKey>&
PibIdentity::getDefaultKey() { return lock()->getDefaultKey(); }


PibIdentity::PibIdentity(ptr_lib::weak_ptr<PibIdentityImpl> impl)
: impl_(impl)
{
}

ptr_lib::shared_ptr<PibKey>
PibIdentity::addKey(const uint8_t* key, size_t keyLength, const Name& keyName)
{
  return lock()->addKey(key, keyLength, keyName);
}

/**
 * Remove the key with keyName and its related certificates. If the key does
 * not exist, do nothing.
 * @param keyName The name of the key.
 */
void
PibIdentity::removeKey(const Name& keyName)
{
  lock()->removeKey(keyName);
}

/**
 * Set the key with name keyName as the default key of the identity.
 * @param keyName The name of the key. This copies the name.
 * @return The PibKey object of the default key.
 * @throws std::invalid_argument if the name of the key does not match the
 * identity name.
 * @throws Pib::Error if the key does not exist.
 */
ptr_lib::shared_ptr<PibKey>&
PibIdentity::setDefaultKey(const Name& keyName)
{
  return lock()->setDefaultKey(keyName);
}

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
PibIdentity::setDefaultKey
  (const uint8_t* key, size_t keyLength, const Name& keyName)
{
  return lock()->setDefaultKey(key, keyLength, keyName);
}

PibKeyContainer&
PibIdentity::getKeys() { return lock()->keys_; }

ptr_lib::shared_ptr<PibIdentityImpl>
PibIdentity::lock()
{
  // TODO: What is this needed for? Use something more general than weak_ptr?
  ptr_lib::shared_ptr<PibIdentityImpl> impl = impl_.lock();

  if (!impl)
    throw domain_error("Invalid Identity instance");

  return impl;
}

}
