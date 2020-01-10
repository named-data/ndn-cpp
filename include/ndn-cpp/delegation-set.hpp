/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
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

#ifndef NDN_DELEGATION_SET_HPP
#define NDN_DELEGATION_SET_HPP

#include "name.hpp"
#include "lite/delegation-set-lite.hpp"

namespace ndn {

/**
 * A DelegationSet holds a list of DelegationSet::Delegation entries which is
 * used as the content of a Link instance. If you add elements with add(), then
 * the list is a set sorted by preference number then by name. But wireDecode
 * will add the elements from the wire encoding, preserving the given order and
 * possible duplicates (in which case a DelegationSet really holds a "list" and
 * not necessarily a "set").
 */
class DelegationSet {
public:
  /**
   * Create a DelegationSet with an empty list of delegations.
   */
  DelegationSet()
  : getDefaultWireEncodingChangeCount_(0), changeCount_(0)
  {
  }

  /**
   * Create a DelegationSet, copying values from the other DelegationSet.
   * @param delegationSet The DelegationSet to copy values from.
   */
  DelegationSet(const DelegationSet& delegationSet)
  // A DelegationSet::Delegation is immutable, so just make a shallow copy.
  : delegations_(delegationSet.delegations_),
    changeCount_(delegationSet.changeCount_)
  {
    setDefaultWireEncoding
      (delegationSet.getDefaultWireEncoding(),
       delegationSet.defaultWireEncodingFormat_);
  }

  DelegationSet& operator=(const DelegationSet& delegationSet);

  /**
   * A DelegationSet::Delegation holds a preference number and delegation name.
   */
  class Delegation {
  public:
    /**
     * Create a new DelegationSet::Delegation with the given values.
     * @param preference The preference number.
     * @param name The delegation name. This makes a copy of the name.
     */
    Delegation(int preference, const Name& name)
    : preference_(preference), name_(name)
    {
    }

    /**
     * Create a new DelegationSet::Delegation by copying from delegationLite.
     * @param delegationLite A DelegationSetLite::Delegation object.
     */
    Delegation(const DelegationSetLite::Delegation& delegationLite);

    /**
     * Get the preference number.
     * @return The preference number.
     */
    int
    getPreference() const { return preference_; }

    /**
     * Get the delegation name.
     * @return The delegation name.
     */
    const Name&
    getName() const { return name_; }

    /**
     * Compare this Delegation with other according to the ordering, based first
     * on the preference number, then on the delegation name.
     * @param other The other Delegation to compare with.
     * @return 0 If they compare equal, -1 if this Delegation comes before other
     * in the ordering, or 1 if this Delegation comes after.
     */
    int
    compare(const Delegation& other);

    /**
     * Set delegationLite to point to the values in this delegation, without
     * copying any memory.
     * WARNING: The resulting pointers in delegationLite are invalid after a
     * further use of this object which could reallocate memory.
     * @param delegationLite A DelegationSetLite::Delegation where the name
     * components array is already allocated.
     */
    void
    get(DelegationSetLite::Delegation& delegationLite) const;

  private:
    int preference_;
    Name name_;
  };

  /**
   * Add a new DelegationSet::Delegation to the list of delegations, sorted by
   * preference number then by name. If there is already a delegation with the
   * same name, update its preference, and remove any extra delegations with the
   * same name.
   * @param preference The preference number.
   * @param name The delegation name. This makes a copy of the name.
   */
  void
  add(int preference, const Name& name);

  /**
   * Add the DelegationSet::Delegation to the end of the list of delegations,
   * without sorting or updating any existing entries. This is useful for adding
   * preferences from a wire encoding, preserving the supplied ordering and
   * possible duplicates.
   * @param preference The preference number.
   * @param name The delegation name. This makes a copy of the name.
   */
  void
  addUnsorted(const ptr_lib::shared_ptr<Delegation>& delegation)
  {
    delegations_.push_back(delegation);
    ++changeCount_;
  }

  /**
   * Remove every DelegationSet::Delegation with the given name.
   * @param name The name to match the name of the delegation(s) to be removed.
   * @return True if a DelegationSet::Delegation was removed, otherwise false.
   */
  bool
  remove(const Name& name);

  /**
   * Clear the list of delegations.
   */
  void
  clear()
  {
    delegations_.clear();
    ++changeCount_;
  }

  /**
   * Get the number of delegation entries.
   * @return The number of delegation entries.
   */
  size_t
  size() const { return delegations_.size(); }

  /**
   * Get the delegation at the given index, according to the ordering described
   * in add().
   * @param i The index of the component, starting from 0.
   * @return The delegation at the index.
   * @throws runtime_error If index is out of range.
   */
  const Delegation&
  get(size_t i) const;

  /**
   * Find the first delegation with the given name and return its index.
   * @param name Then name of the delegation to find.
   * @return The index of the delegation, or -1 if not found.
   */
  int
  find(const Name& name) const;

  /**
   * Encode this DelegationSet for a particular wire format. If wireFormat is
   * the default wire format, also set the defaultWireEncoding field to the
   * encoded result. Even though this is const, if wireFormat is the default
   * wire format we update the defaultWireEncoding.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * DelegationSet. If omitted, use WireFormat::getDefaultWireFormat().
   * @return The encoded byte array.
   */
  Blob
  wireEncode
    (WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Decode the input using a particular wire format and update this
   * DelegationSet. If wireFormat is the default wire format, also set the
   * defaultWireEncoding field to a copy of the input. (To not copy the input,
   * see wireDecode(Blob).)
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const uint8_t *input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Decode the input using a particular wire format and update this
   * DelegationSet. If wireFormat is the default wire format, also set the
   * defaultWireEncoding field to a copy of the input. (To not copy the input,
   * see wireDecode(Blob).)
   * @param input The input byte array to be decoded.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const std::vector<uint8_t>& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Decode the input using a particular wire format and update this
   * DelegationSet. If wireFormat is the default wire format, also set the
   * defaultWireEncoding to another pointer to the input Blob.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Return a reference to the defaultWireEncoding, which was encoded with
   * getDefaultWireEncodingFormat().  The Blob may have a null pointer.
   */
  const Blob&
  getDefaultWireEncoding() const
  {
    if (getDefaultWireEncodingChangeCount_ != getChangeCount()) {
      // The values have changed, so the default wire encoding is invalidated.
      // This method can be called on a const object, but we want to be able to
      // update the default cached value.
      const_cast<DelegationSet*>(this)->defaultWireEncoding_ = Blob();
      const_cast<DelegationSet*>(this)->defaultWireEncodingFormat_ = 0;
      const_cast<DelegationSet*>(this)->getDefaultWireEncodingChangeCount_ =
        getChangeCount();
    }

    return defaultWireEncoding_;
  }

  /**
   * Get the WireFormat which is used by getDefaultWireEncoding().
   * @return The WireFormat, which is only meaningful if the
   * getDefaultWireEncoding() does not have a null pointer.
   */
  WireFormat*
  getDefaultWireEncodingFormat() const { return defaultWireEncodingFormat_; }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const { return changeCount_; }

private:
  void
  setDefaultWireEncoding
    (const Blob& defaultWireEncoding, WireFormat *defaultWireEncodingFormat)
  {
    defaultWireEncoding_ = defaultWireEncoding;
    defaultWireEncodingFormat_ = defaultWireEncodingFormat;
    // Set getDefaultWireEncodingChangeCount_ so that the next call to
    //   getDefaultWireEncoding() won't clear defaultWireEncoding_.
    getDefaultWireEncodingChangeCount_ = getChangeCount();
  }

  std::vector<ptr_lib::shared_ptr<Delegation> > delegations_;
  Blob defaultWireEncoding_;
  WireFormat *defaultWireEncodingFormat_;
  uint64_t getDefaultWireEncodingChangeCount_;
  uint64_t changeCount_;
};

}

#endif
