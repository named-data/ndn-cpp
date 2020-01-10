/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2020 Regents of the University of California.
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

#ifndef NDN_KEY_LOCATOR_HPP
#define NDN_KEY_LOCATOR_HPP

#include <vector>
#include "c/key-types.h"
#include "name.hpp"
#include "util/change-counter.hpp"
#include "lite/key-locator-lite.hpp"

namespace ndn {

class Signature;

class KeyLocator {
public:
  KeyLocator()
  : type_((ndn_KeyLocatorType)-1), changeCount_(0)
  {
  }

  /**
   * Clear the keyData and set the type to none.
   */
  void
  clear()
  {
    type_ = (ndn_KeyLocatorType)-1;
    keyData_.reset();
    setKeyName(Name());
    ++changeCount_;
  }

  /**
   * Set keyLocatorLite to point to the values in this key locator, without
   * copying any memory.
   * WARNING: The resulting pointers in keyLocatorLite are invalid after a
   * further use of this object which could reallocate memory.
   * @param keyLocatorLite A KeyLocatorLite where the name components array is
   * already allocated.
   */
  void
  get(KeyLocatorLite& keyLocatorLite) const;

  /**
   * Clear this key locator, and set the values by copying from keyLocatorLite.
   * @param keyLocatorLite A KeyLocatorLite object.
   */
  void
  set(const KeyLocatorLite& keyLocatorLite);

  ndn_KeyLocatorType
  getType() const { return type_; }

  const Blob&
  getKeyData() const { return keyData_; }

  const Name&
  getKeyName() const { return keyName_.get(); }

  Name&
  getKeyName() { return keyName_.get(); }

  void
  setType(ndn_KeyLocatorType type)
  {
    type_ = type;
    ++changeCount_;
  }

  void
  setKeyData(const Blob& keyData)
  {
    keyData_ = keyData;
    ++changeCount_;
  }

  void
  setKeyName(const Name &keyName)
  {
    keyName_.set(keyName);
    ++changeCount_;
  }

  /**
   * Check if this is the same key locator as other.
   * @param other The other KeyLocator to compare with.
   * @return true if the key locators are equal, otherwise false.
   */
  bool
  equals(const KeyLocator& other) const;

  /**
   * Check if this is the same key locator as other.
   * @param other The other KeyLocator to compare with.
   * @return true if the key locators are equal, otherwise false.
   */
  bool
  operator == (const KeyLocator& other) const { return equals(other); }

  /**
   * Check if this is not the same key locator as other.
   * @param other The other KeyLocator to compare with.
   * @return true if the key locators are not equal, otherwise false.
   */
  bool
  operator != (const KeyLocator& other) const { return !equals(other); }

  /**
   * If the signature is a type that has a KeyLocator (so that
   * getFromSignature will succeed), return true.
   * Note: This is a static method of KeyLocator instead of a method of
   * Signature so that the Signature base class does not need to be overloaded
   * with all the different kinds of information that various signature
   * algorithms may use.
   * @param signature An object of a subclass of Signature.
   * @return True if the signature is a type that has a KeyLocator, otherwise
   * false.
   */
  static bool
  canGetFromSignature(const Signature* signature);

  /**
   * If the signature is a type that has a KeyLocator, then return it. Otherwise
   * throw an error. To check if the signature has a KeyLocator without throwing
   * an error, you can use canGetFromSignature().
   * @param signature An object of a subclass of Signature.
   * @return The signature's KeyLocator. It is an error if signature doesn't
   * have a KeyLocator.
   */
  static KeyLocator&
  getFromSignature(Signature* signature);

  static const KeyLocator&
  getFromSignature(const Signature* signature)
  {
    return getFromSignature(const_cast<Signature*>(signature));
  }

  /**
   * Get the change count, which is incremented each time this object (or a child object) is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const
  {
    if (keyName_.checkChanged())
      // A child object has changed, so update the change count.
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      ++const_cast<KeyLocator*>(this)->changeCount_;

    return changeCount_;
  }

private:
  ndn_KeyLocatorType type_; /**< -1 for none */
  Blob keyData_; /**< An array for the key data as follows:
    * If type_ is ndn_KeyLocatorType_KEY_LOCATOR_DIGEST, the digest data.
    */
  ChangeCounter<Name> keyName_; /**< The key name (only used if type_ is ndn_KeyLocatorType_KEYNAME.) */
  uint64_t changeCount_;
};

}

#endif
