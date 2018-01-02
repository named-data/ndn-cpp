/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include <stdexcept>
#include <ndn-cpp/delegation-set.hpp>

using namespace std;

namespace ndn {

DelegationSet::Delegation::Delegation
  (const DelegationSetLite::Delegation& delegationLite)
{
  preference_ = delegationLite.getPreference();
  name_.set(delegationLite.getName());
}

int
DelegationSet::Delegation::compare(const Delegation& other)
{
  if (preference_ < other.preference_)
    return -1;
  if (preference_ > other.preference_)
    return 1;

  return name_.compare(other.name_);
}

void
DelegationSet::Delegation::get(DelegationSetLite::Delegation& delegationLite) const
{
  delegationLite.setPreference(preference_);
  name_.get(delegationLite.getName());
}

DelegationSet& DelegationSet::operator=(const DelegationSet& delegationSet)
{
  // A DelegationSet::Delegation is immutable, so just make a shallow copy.
  delegations_ = delegationSet.delegations_;
  setDefaultWireEncoding
    (delegationSet.getDefaultWireEncoding(),
     delegationSet.defaultWireEncodingFormat_);

  return *this;
}

void
DelegationSet::add(int preference, const Name& name)
{
  remove(name);

  ptr_lib::shared_ptr<Delegation> newDelegation(new Delegation(preference, name));
  // Find the index of the first entry where it is not less than newDelegation.
  size_t i = 0;
  while (i < delegations_.size()) {
    if (delegations_[i]->compare(*newDelegation) >= 0)
      break;

    ++i;
  }

  delegations_.insert(delegations_.begin() + i, newDelegation);
  ++changeCount_;
}

bool
DelegationSet::remove(const Name& name)
{
  bool wasRemoved = false;
  // Go backwards through the list so we can remove entries.
  for (int i = delegations_.size() - 1; i >= 0; --i) {
    if (delegations_[i]->getName().equals(name)) {
      wasRemoved = true;
      delegations_.erase(delegations_.begin() + i);
    }
  }

  if (wasRemoved)
    ++changeCount_;
  return wasRemoved;
}

const DelegationSet::Delegation&
DelegationSet::get(size_t i) const
{
  if (i >= delegations_.size())
    throw runtime_error("DelegationSet::get: Index is out of bounds");

  return *delegations_[i];
}

int
DelegationSet::find(const Name& name) const
{
  for (size_t i = 0; i < delegations_.size(); ++i) {
    if (delegations_[i]->getName().equals(name))
      return i;
  }

  return -1;
}

Blob
DelegationSet::wireEncode(WireFormat& wireFormat) const
{
  if (getDefaultWireEncoding() && getDefaultWireEncodingFormat() == &wireFormat)
    // We already have an encoding in the desired format.
    return getDefaultWireEncoding();

  Blob wireEncoding = wireFormat.encodeDelegationSet(*this);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    const_cast<DelegationSet*>(this)->setDefaultWireEncoding
      (wireEncoding, WireFormat::getDefaultWireFormat());

  return wireEncoding;
}

void
DelegationSet::wireDecode
  (const uint8_t *input, size_t inputLength, WireFormat& wireFormat)
{
  wireFormat.decodeDelegationSet(*this, input, inputLength);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    // The input is not an immutable Blob, so we need to copy.
    setDefaultWireEncoding
      (Blob(input, inputLength), WireFormat::getDefaultWireFormat());
  else
    setDefaultWireEncoding(Blob(), 0);
}

void
DelegationSet::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  wireDecode(input.buf(), input.size(), wireFormat);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    // Take a pointer to the input Blob without copying.
    setDefaultWireEncoding(input, WireFormat::getDefaultWireFormat());
  else
    setDefaultWireEncoding(Blob(), 0);
}

}
