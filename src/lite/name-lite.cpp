/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#include "../c/name.h"
#include <ndn-cpp/lite/name-lite.hpp>

namespace ndn {

NameLite::Component::Component()
{
  ndn_NameComponent_initialize(this, 0, 0);
}

NameLite::Component::Component(const uint8_t* value, size_t valueLength)
{
  ndn_NameComponent_initialize(this, value, valueLength);
}

uint64_t
NameLite::Component::toNumber() const
{
  return ndn_NameComponent_toNumber(const_cast<Component*>(this));
}

ndn_Error
NameLite::Component::toNumberWithMarker(uint8_t marker, uint64_t& result) const
{
  return ndn_NameComponent_toNumberWithMarker(const_cast<Component*>(this), marker, &result);
}

ndn_Error
NameLite::Component::toNumberWithPrefix
  (const uint8_t* prefix, size_t prefixLength, uint64_t& result)
{
  return ndn_NameComponent_toNumberWithPrefix
    (this, prefix, prefixLength, &result);
}

int
NameLite::Component::compare(NameLite::Component& other)
{
  return ndn_NameComponent_compare(this, &other);
}

NameLite::NameLite(ndn_NameComponent* components, size_t maxComponents)
{
  ndn_Name_initialize(this, components, maxComponents);
}

bool
NameLite::match(const NameLite& name) const
{
  return ndn_Name_match(const_cast<NameLite*>(this), const_cast<NameLite*>(&name)) != 0;
}

ndn_Error
NameLite::append(const uint8_t* value, size_t valueLength)
{
  return ndn_Name_appendComponent(this, value, valueLength);
}

ndn_Error
NameLite::append(const char *value) { return ndn_Name_appendString(this, value); }

}
