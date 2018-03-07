/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/key-container.cpp
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

#include <ndn-cpp/security/pib/pib-impl.hpp>
#include "detail/pib-key-impl.hpp"
#include <ndn-cpp/security/pib/pib-key-container.hpp>

using namespace std;

namespace ndn {

PibKeyContainer::PibKeyContainer
  (const Name& identityName, const ptr_lib::shared_ptr<PibImpl>& pibImpl)
  // This copies the Name.
: identityName_(identityName), pibImpl_(pibImpl)
{
  if (!pibImpl)
    throw runtime_error("The pibImpl is null");

  keyNames_ = pibImpl_->getKeysOfIdentity(identityName);
}

ptr_lib::shared_ptr<PibKey>
PibKeyContainer::add(const uint8_t* key, size_t keyLength, const Name& keyName)
{
  if (identityName_ != PibKey::extractIdentityFromKeyName(keyName))
    throw invalid_argument
      ("The key name `" + keyName.toUri() +
       "` does not match the identity name `" + identityName_.toUri() + "`");

  keyNames_.insert(keyName);
  keys_[keyName] = ptr_lib::make_shared<PibKeyImpl>
    (keyName, key, keyLength, pibImpl_);

  return get(keyName);
}

void
PibKeyContainer::remove(const Name& keyName)
{
  if (identityName_ != PibKey::extractIdentityFromKeyName(keyName))
    throw invalid_argument
      ("Key name `" + keyName.toUri() + "` does not match identity `" +
       identityName_.toUri() + "`");

  keyNames_.erase(keyName);
  keys_.erase(keyName);
  pibImpl_->removeKey(keyName);
}

ptr_lib::shared_ptr<PibKey>
PibKeyContainer::get(const Name& keyName)
{
  if (identityName_ != PibKey::extractIdentityFromKeyName(keyName))
    throw invalid_argument
      ("Key name `" + keyName.toUri() + "` does not match identity `" +
       identityName_.toUri() + "`");

  map<Name, ptr_lib::shared_ptr<PibKeyImpl>>::const_iterator it =
    keys_.find(keyName);

  ptr_lib::shared_ptr<PibKeyImpl> pibKeyImpl;
  if (it != keys_.end())
    pibKeyImpl = it->second;
  else {
    pibKeyImpl = ptr_lib::make_shared<PibKeyImpl>(keyName, pibImpl_);
    keys_[keyName] = pibKeyImpl;
  }

  return ptr_lib::shared_ptr<PibKey>(new PibKey(pibKeyImpl));
}

ptr_lib::shared_ptr<std::vector<Name> >
PibKeyContainer::getKeyNames() const
{
  ptr_lib::shared_ptr<vector<Name> > result(new vector<Name>);

  for (map<Name, ptr_lib::shared_ptr<PibKeyImpl>>::const_iterator it =
         keys_.begin();
       it != keys_.end();
       ++it)
    result->push_back(it->first);

  return result;
}

bool
PibKeyContainer::isConsistent() const
{
  return keyNames_ == pibImpl_->getKeysOfIdentity(identityName_);
}

}
