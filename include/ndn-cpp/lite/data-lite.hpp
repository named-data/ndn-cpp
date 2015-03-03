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
   * Create a DataLite where the name and KeyLocator keyName have no components
   * array. You must later user setName(), etc.
   */
  DataLite();

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
   * Set the data packet's name.
   * @param name The data packet's name. This only copies the pointer to the name
   * components array, but does not copy the component values.
   */
  void
  setName(const NameLite& name)
  {
    NameLite::upCast(this->name) = name;
  }

  /**
   * Set the data packet's content.
   * @param content The data packet's content. This does not copy the bytes of
   * the content.
   */
  void
  setContent(const BlobLite& content)
  {
    BlobLite::upCast(this->content) = content;
  }

  /**
   * Upcast the reference to the ndn_Data struct to a DataLite.
   * @param data A reference to the ndn_Data struct.
   * @return The same reference as DataLite.
   */
  static DataLite&
  upCast(ndn_Data& data) { return *(DataLite*)&data; }

  static const DataLite&
  upCast(const ndn_Data& data) { return *(DataLite*)&data; }
};

}

#endif
