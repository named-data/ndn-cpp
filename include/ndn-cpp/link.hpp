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

#ifndef NDN_LINK_HPP
#define NDN_LINK_HPP

#include "data.hpp"
#include "delegation-set.hpp"

namespace ndn {

/**
 * The Link class extends Data and represents a Link instance where the Data
 * content is an encoded delegation set. The format is defined in "link.pdf"
 * attached to Redmine issue http://redmine.named-data.net/issues/2587 .
 */
class Link : public Data {
public:
  /**
   * Create a Link with default values and where the list of delegations is
   * empty and the meta info type is LINK.
   */
  Link()
  {
    getMetaInfo().setType(ndn_ContentType_LINK);
  }

  /**
   * Create a Link with the given name and default values and where the list of
   * delegations is empty and the meta info type is LINK.
   * @param name The name which is copied.
   */
  Link(const Name& name)
  : Data(name)
  {
    getMetaInfo().setType(ndn_ContentType_LINK);
  }

  /**
   * Create a Link, copying values from the other Data object. If the content
   * can be decoded using the default wire encoding, then update the list
   * of delegations.
   * @param data The Data object to copy values from.
   */
  Link(const Data& data);

  /**
   * Override to call the base class wireDecode then populate the list of
   * delegations from the content.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  virtual void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Add a new delegation to the list of delegations, sorted by
   * preference number then by name. Re-encode this object's content using the
   * given wireFormat.
   * @param preference The preference number.
   * @param name The delegation name. This makes a copy of the name. If there
   * is already a delegation with the same name, this updates its preference.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * DelegationSet. If omitted, use WireFormat::getDefaultWireFormat().
   * @return This Link so that you can chain calls to update values.
   */
  Link&
  addDelegation
    (int preference, const Name& name,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    delegations_.add(preference, name);
    encodeContent(wireFormat);

    return *this;
  }

  /**
   * Remove every delegation with the given name. Re-encode this object's
   * content using the given wireFormat.
   * @param name Then name to match the name of the delegation(s) to be removed.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * DelegationSet. If omitted, use WireFormat::getDefaultWireFormat().
   * @return True if a delegation was removed, otherwise false.
   */
  bool
  removeDelegation
    (const Name& name,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    bool wasRemoved = delegations_.remove(name);
    if (wasRemoved)
      encodeContent(wireFormat);

    return wasRemoved;
  }

  /**
   * Get the list of delegation for read only.
   * @return The list of delegation for read only. To modify it, call
   * Link::addDelegation, etc.
   */
  const DelegationSet&
  getDelegations() const { return delegations_; }

private:
  /**
   * Encode the delegations_ and set this object's content. Also set the
   * meta info content type to LINK.
   * @param wireFormat A WireFormat object used to encode the DelegationSet.
   */
  void
  encodeContent(WireFormat& wireFormat)
  {
    setContent(delegations_.wireEncode(wireFormat));
    getMetaInfo().setType(ndn_ContentType_LINK);
  }

  DelegationSet delegations_;
};

}

#endif
