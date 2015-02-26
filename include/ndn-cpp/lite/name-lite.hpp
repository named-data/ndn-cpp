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

#ifndef NDN_NAME_LITE_HPP
#define NDN_NAME_LITE_HPP

#include "../c/errors.h"
#include "util/blob-lite.hpp"

namespace ndn {

class NameLite : private ndn_Name {
public:
  class Component : private ndn_NameComponent {
  public:
    Component();

    Component(const uint8_t* value, size_t valueLength);

    const BlobLite&
    getValue() const { return *(BlobLite*)&value; }

    uint64_t
    toNumber() const;

    ndn_Error
    toNumberWithMarker(uint8_t marker, uint64_t& result) const;

    ndn_Error
    toNumberWithPrefix
      (const uint8_t* prefix, size_t prefixLength, uint64_t& result);

    /**
     * Upcast the reference to the ndn_NameComponent struct to a NameLite::Component.
     * @param component A reference to the ndn_NameComponent struct.
     * @return The same reference as NameLite::Component.
     */
    static Component&
    upCast(ndn_NameComponent& component) { return *(Component*)&component; }

    static const Component&
    upCast(const ndn_NameComponent& component) { return *(Component*)&component; }
  };

  NameLite(ndn_NameComponent* components, size_t maxComponents);

  const Component&
  get(int i) const
  {
    // TODO: Range check.
    if (i >= 0)
      return Component::upCast(components[i]);
    else
      // Negative index.
      return Component::upCast(components[nComponents - (-i)]);
  }

  size_t
  size() const { return nComponents; }

  bool
  match(const NameLite& name) const;

  ndn_Error
  append(const uint8_t* value, size_t valueLength);

  ndn_Error
  append(const NameLite::Component& component)
  {
    return append(component.getValue().buf(), component.getValue().size());
  }

  ndn_Error
  append(const BlobLite& value) { return append(value.buf(), value.size()); }

  ndn_Error
  append(const char *value);

  /**
   * Upcast the reference to the ndn_Name struct to a NameLite.
   * @param name A reference to the ndn_Name struct.
   * @return The same reference as NameLite.
   */
  static NameLite&
  upCast(ndn_Name& name) { return *(NameLite*)&name; }

  static const NameLite&
  upCast(const ndn_Name& name) { return *(NameLite*)&name; }
};

}

#endif
