/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/pib.cpp
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
#include <ndn-cpp/security/pib/pib.hpp>

using namespace std;

namespace ndn {

Pib::Pib(const string& scheme, const string& location,
         const ptr_lib::shared_ptr<PibImpl>& pibImpl)
: scheme_(scheme),
  location_(location),
  identities_(pibImpl),
  pibImpl_(pibImpl)
{
  if (!pibImpl)
    throw runtime_error("The pibImpl is null");
}

void
Pib::setTpmLocator(const string& tpmLocator)
{
  if (tpmLocator == pibImpl_->getTpmLocator())
    return;

  reset();
  pibImpl_->setTpmLocator(tpmLocator);
}

string
Pib::getTpmLocator()
{
  string tpmLocator = pibImpl_->getTpmLocator();
  if (tpmLocator == "")
    throw Pib::Error("TPM info does not exist");

  return tpmLocator;
}

void
Pib::reset()
{
  pibImpl_->clearIdentities();
  pibImpl_->setTpmLocator("");
  defaultIdentity_.reset();
  identities_.reset();
}

ptr_lib::shared_ptr<PibIdentity>
Pib::addIdentity(const Name& identityName)
{
  // BOOST_ASSERT(identities_.isConsistent());

  return identities_.add(identityName);
}

void
Pib::removeIdentity(const Name& identityName)
{
  // BOOST_ASSERT(identities_.isConsistent());

  if (defaultIdentity_ && defaultIdentity_->getName() == identityName)
    defaultIdentity_.reset();

  identities_.remove(identityName);
}

ptr_lib::shared_ptr<PibIdentity>
Pib::setDefaultIdentity(const Name& identityName)
{
  // BOOST_ASSERT(identities_.isConsistent());

  defaultIdentity_ = identities_.add(identityName);

  pibImpl_->setDefaultIdentity(identityName);
  return defaultIdentity_;
}

ptr_lib::shared_ptr<PibIdentity>
Pib::getIdentity(const Name& identityName)
{
  // BOOST_ASSERT(identities_.isConsistent());

  return identities_.get(identityName);
}

ptr_lib::shared_ptr<PibIdentity>&
Pib::getDefaultIdentity()
{
  // BOOST_ASSERT(identities_.isConsistent());

  if (!defaultIdentity_)
    defaultIdentity_ = identities_.get(pibImpl_->getDefaultIdentity());

  // BOOST_ASSERT(pibImpl_->getDefaultIdentity() == defaultIdentity_->getName());

  return defaultIdentity_;
}

void
Pib::getAllIdentityNames(vector<Name>& nameList)
{
  for (set<Name>::const_iterator it = identities_.identityNames_.begin();
       it != identities_.identityNames_.end(); ++it)
    // This copies the name.
    nameList.push_back(*it);
}

}
