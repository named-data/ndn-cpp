/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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
PibIdentity::getName() const { return lock()->getName(); }

ptr_lib::shared_ptr<PibKey>
PibIdentity::getKey(const Name& keyName) const { return lock()->getKey(keyName); }

const ptr_lib::shared_ptr<PibKey>&
PibIdentity::getDefaultKey() const { return lock()->getDefaultKey(); }


PibIdentity::PibIdentity(ptr_lib::weak_ptr<PibIdentityImpl> impl)
: impl_(impl)
{
}

PibKeyContainer&
PibIdentity::getKeys() { return lock()->keys_; }

ptr_lib::shared_ptr<PibIdentityImpl>
PibIdentity::lock() const
{
  // TODO: What is this needed for? Use something more general than weak_ptr?
  ptr_lib::shared_ptr<PibIdentityImpl> impl = impl_.lock();

  if (!impl)
    throw domain_error("Invalid Identity instance");

  return impl;
}

}
