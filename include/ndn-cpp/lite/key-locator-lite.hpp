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

#ifndef NDN_KEY_LOCATOR_LITE_HPP
#define NDN_KEY_LOCATOR_LITE_HPP

#include "name-lite.hpp"
#include "../c/key-types.h"

namespace ndn {

class KeyLocatorLite : private ndn_KeyLocator {
public:
  KeyLocatorLite(ndn_NameComponent* keyNameComponents, size_t maxKeyNameComponents);

  ndn_KeyLocatorType
  getType() const { return type; }

  BlobLite&
  getKeyData() { return BlobLite::upCast(keyData); }

  const BlobLite&
  getKeyData() const { return BlobLite::upCast(keyData); }

  NameLite&
  getKeyName() { return NameLite::upCast(keyName); }

  const NameLite&
  getKeyName() const { return NameLite::upCast(keyName); }

  ndn_KeyNameType
  getKeyNameType() const { return keyNameType; }

  void
  setKeyName(const NameLite& keyName)
  {
    NameLite::upCast(this->keyName) = keyName;
  }

  void
  setType(ndn_KeyLocatorType type) { this->type = type; }

  void
  setKeyData(const BlobLite& keyData)
  {
    BlobLite::upCast(this->keyData) = keyData;
  }

  void
  setKeyNameType(ndn_KeyNameType keyNameType) { this->keyNameType = keyNameType; }

  /**
   * Upcast the reference to the ndn_KeyLocator struct to a KeyLocatorLite.
   * @param keyLocator A reference to the ndn_KeyLocator struct.
   * @return The same reference as KeyLocatorLite.
   */
  static KeyLocatorLite&
  upCast(ndn_KeyLocator& keyLocator) { return *(KeyLocatorLite*)&keyLocator; }

  static const KeyLocatorLite&
  upCast(const ndn_KeyLocator& keyLocator) { return *(KeyLocatorLite*)&keyLocator; }
};

}

#endif
