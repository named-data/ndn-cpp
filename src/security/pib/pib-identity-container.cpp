/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/identity-container.cpp
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
#include "detail/pib-identity-impl.hpp"
#include <ndn-cpp/security/pib/pib-identity-container.hpp>

using namespace std;

namespace ndn {

PibIdentityContainer::PibIdentityContainer
  (const ptr_lib::shared_ptr<PibImpl>& pibImpl)
: pibImpl_(pibImpl)
{
  if (!pibImpl)
    throw runtime_error("The pibImpl is null");

  identityNames_ = pibImpl_->getIdentities();
}

ptr_lib::shared_ptr<PibIdentity>
PibIdentityContainer::add(const Name& identityName)
{
  if (identityNames_.count(identityName) == 0) {
    identityNames_.insert(identityName);
    identities_[identityName] = ptr_lib::shared_ptr<PibIdentityImpl>
      (new PibIdentityImpl(identityName, pibImpl_, true));
  }

  return get(identityName);
}

void
PibIdentityContainer::remove(const Name& identityName)
{
  identityNames_.erase(identityName);
  identities_.erase(identityName);
  pibImpl_->removeIdentity(identityName);
}

ptr_lib::shared_ptr<PibIdentity>
PibIdentityContainer::get(const Name& identityName)
{
  map<Name, ptr_lib::shared_ptr<PibIdentityImpl>>::const_iterator it =
    identities_.find(identityName);

  ptr_lib::shared_ptr<PibIdentityImpl> pibIdentityImpl;
  if (it != identities_.end())
    pibIdentityImpl = it->second;
  else {
    pibIdentityImpl = ptr_lib::make_shared<PibIdentityImpl>
      (identityName, pibImpl_, false);
    identities_[identityName] = pibIdentityImpl;
  }

  return ptr_lib::shared_ptr<PibIdentity>(new PibIdentity(pibIdentityImpl));
}

void
PibIdentityContainer::reset()
{
  identities_.clear();
  identityNames_ = pibImpl_->getIdentities();
}

bool
PibIdentityContainer::isConsistent() const
{
  return identityNames_ == pibImpl_->getIdentities();
}

}
