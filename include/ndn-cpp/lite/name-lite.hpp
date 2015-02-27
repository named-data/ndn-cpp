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
#include "../c/name-types.h"

namespace ndn {

/**
 * A NameLite holds an array of NameLite::Component.
 */
class NameLite : private ndn_Name {
public:
  /**
   * A NameLite::Component holds a pointer to the component value.
   */
  class Component : private ndn_NameComponent {
  public:
    /**
     * Create a NameLite::Component of zero size.
     */
    Component();

    /**
     * Create a NameLite::Component with the given value.
     * @param value The pre-allocated buffer for the value.
     * @param valueLength The number of bytes in value.
     */
    Component(const uint8_t* value, size_t valueLength);

    const BlobLite&
    getValue() const { return *(BlobLite*)&value; }

    /**
     * Interpret the name component as a network-ordered number and return an integer.
     * @return The integer number.
     */
    uint64_t
    toNumber() const;

    /**
     * Interpret the name component as a network-ordered number with a marker
     * and return an integer.
     * @param marker The required first byte of the component.
     * @param result Return the integer number.
     * @return 0 for success, or an error code if the first byte of the component
     * does not equal the marker.
     */
    ndn_Error
    toNumberWithMarker(uint8_t marker, uint64_t& result) const;

    /**
     * Interpret the name component as a network-ordered number with a prefix
     * and return an integer.
     * @param prefix The required prefix of the component.
     * @param prefixLength The length of prefix.
     * @param result Return the integer number.
     * @return 0 for success, or an error code if the first bytes of the
     * component do not equal the prefix.
     */
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

  /**
   * Create a NameLite to use the components array.
   * @param components the pre-allocated array of ndn_NameComponent. Instead of
   * an array of NameLite::Component, this is an array of the underlying
   * ndn_NameComponent struct so that it doesn't run the default constructor
   * unnecessarily.
   * @param maxComponents the number of elements in the allocated components array.
   */
  NameLite(ndn_NameComponent* components, size_t maxComponents);

  /**
   * Get the component at the given index. This does not check if i is out of bounds.
   * @param i The index of the component, starting from 0.  However, if i is
   * negative, return the component at size() - (-i).
   * @return The name component at the index.
   */
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

  /**
   * Get the number of components.
   * @return The number of components.
   */
  size_t
  size() const { return nComponents; }

  /**
   * Check if the N components of this name are the same as the first N
   * components of the given name.
   * @param name The Name to check.
   * @return true if this matches the given name, otherwise false.
   * This always returns true if this name is empty.
   */
  bool
  match(const NameLite& name) const;

  /**
   * Append a component to this name with the bytes in the given buffer.
   * @param value A pointer to the buffer with the bytes of the component.
   * This does not copy the bytes.
   * @param valueLength The number of bytes in value.
   * @return 0 for success, or an error code if there is no more room in the
   * components array.
   */
  ndn_Error
  append(const uint8_t* value, size_t valueLength);

  /**
   * Append a component to this name with the bytes in the given blob.
   * @param value A BlobLite with the bytes of the component.  This does not
   * copy the bytes.
   * @return 0 for success, or an error code if there is no more room in the
   * components array.
   */
  ndn_Error
  append(const BlobLite& value) { return append(value.buf(), value.size()); }

  /**
   * Append a component to this name with the bytes in the given component.
   * @param component A Component with the bytes of the component.  This does not
   * copy the bytes.
   * @return 0 for success, or an error code if there is no more room in the
   * components array.
   */
  ndn_Error
  append(const NameLite::Component& component)
  {
    return append(component.getValue().buf(), component.getValue().size());
  }

  /**
   * Append a component to this name with the bytes in raw string value.
   * @param value The null-terminated string, treated as a byte array.  This
   * does not copy the bytes.
   * @return 0 for success, or an error code if there is no more room in the
   * components array (nComponents is already maxComponents).
   */
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
