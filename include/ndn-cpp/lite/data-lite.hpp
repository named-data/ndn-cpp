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

#ifndef NDN_DATA_LITE_HPP
#define NDN_DATA_LITE_HPP

#include "meta-info-lite.hpp"
#include "signature-lite.hpp"
#include "../c/data-types.h"

namespace ndn {

/**
 * A DataLite holds a NameLite and other fields to represent an NDN Data packet.
 */
class DataLite : private ndn_Data {
public:
  /**
   * Create a DataLite with the pre-allocated nameComponents and
   * keyNameComponents, and defaults for all the values.
   * @param nameComponents The pre-allocated array of ndn_NameComponent. Instead
   * of an array of NameLite::Component, this is an array of the underlying
   * ndn_NameComponent struct so that it doesn't run the default constructor
   * unnecessarily.
   * @param maxNameComponents The number of elements in the allocated
   * nameComponents array.
   * @param keyNameComponents The pre-allocated array of ndn_NameComponent for
   * the signature.keyLocator. Instead of an array of NameLite::Component, this
   * is an array of the underlying ndn_NameComponent struct so that it doesn't
   * run the default constructor unnecessarily.
   * @param maxKeyNameComponents The number of elements in the allocated
   * keyNameComponents array.
   */
  DataLite(ndn_NameComponent* nameComponents, size_t maxNameComponents,
   ndn_NameComponent* keyNameComponents, size_t maxKeyNameComponents);

  const SignatureLite&
  getSignature() const { return SignatureLite::upCast(signature); }

  SignatureLite&
  getSignature() { return SignatureLite::upCast(signature); }

  const NameLite&
  getName() const { return NameLite::upCast(name); }

  NameLite&
  getName() { return NameLite::upCast(name); }

  const MetaInfoLite&
  getMetaInfo() const { return MetaInfoLite::upCast(metaInfo); }

  MetaInfoLite&
  getMetaInfo() { return MetaInfoLite::upCast(metaInfo); }

  const BlobLite&
  getContent() const { return BlobLite::upCast(content); }

  /**
   * Set this data packet's name to have the values from the given name.
   * @param name The name to get values from.
   * @return 0 for success, or an error code if there is not enough room in this
   * object's name components array.
   */
  ndn_Error
  setName(const NameLite& name)
  {
    return NameLite::upCast(this->name).set(name);
  }

  /**
   * Set this data packet's content.
   * @param content The data packet's content. This does not copy the bytes of
   * the content.
   * @return This DataLite so that you can chain calls to update values.
   */
  DataLite&
  setContent(const BlobLite& content)
  {
    BlobLite::upCast(this->content) = content;
    return *this;
  }

  /**
   * Set this data packet object to have the values from the other data.
   * @param other The other DataLite to get values from.
   * @return 0 for success, or an error code if there is not enough room in this
   * object's name or key locator keyName components array.
   */
  ndn_Error
  set(const DataLite& other);

  /**
   * Upcast the reference to the ndn_Data struct to a DataLite.
   * @param data A reference to the ndn_Data struct.
   * @return The same reference as DataLite.
   */
  static DataLite&
  upCast(ndn_Data& data) { return *(DataLite*)&data; }

  static const DataLite&
  upCast(const ndn_Data& data) { return *(DataLite*)&data; }

private:
  // Declare friends who can downcast to the private base.
  friend class Tlv0_1_1WireFormatLite;

  /**
   * Don't allow the copy constructor. Instead use set(const DataLite&)
   * which can return an error if there is no more room in the name components
   * array.
   */
  DataLite(DataLite& other);
  DataLite(const DataLite& other);

  /**
   * Don't allow the assignment operator. Instead use set(const DataLite&)
   * which can return an error if there is no more room in the name components
   * array.
   */
  DataLite& operator=(const DataLite& other);
};

}

#endif
