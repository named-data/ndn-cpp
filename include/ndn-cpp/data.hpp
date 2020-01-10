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

#ifndef NDN_DATA_HPP
#define NDN_DATA_HPP

#include "name.hpp"
#include "signature.hpp"
#include "meta-info.hpp"
#include "util/signed-blob.hpp"
#include "encoding/wire-format.hpp"
#include "util/change-counter.hpp"
#include "lite/data-lite.hpp"

namespace ndn {

class LpPacket;

class Data {
public:
  /**
   * Create a new Data object with default values and where the signature is a blank Sha256WithRsaSignature.
   */
  Data();

  /**
   * Create a new Data object with the given name and default values and where the signature is a blank Sha256WithRsaSignature.
   * @param name A reference to the name which is copied.
   */
  Data(const Name& name);

  /**
   * The copy constructor: Create a deep copy of the given data object, including a clone of the signature object.
   * @param data The data object to copy.
   */
  Data(const Data& data);

  /**
   * The virtual destructor.
   */
  virtual ~Data();

  /**
   * The assignment operator: Copy fields and make a clone of the signature.
   * @param data The other object to copy from.
   * @return A reference to this object.
   */
  Data& operator=(const Data& data);

  /**
   * Encode this Data for a particular wire format. If wireFormat is the default wire format, also set the defaultWireEncoding
   * field to the encoded result.
   * Even though this is const, if wireFormat is the default wire format we update the defaultWireEncoding.
   * @param wireFormat A WireFormat object used to encode the input. If omitted, use WireFormat getDefaultWireFormat().
   * @return The encoded byte array.
   */
  SignedBlob
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Decode the input using a particular wire format and update this Data. If wireFormat is the default wire format, also
   * set the defaultWireEncoding to another pointer to the input Blob.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat A WireFormat object used to decode the input. If omitted, use WireFormat getDefaultWireFormat().
   */
  virtual void
  wireDecode(const Blob& input, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Decode the input using a particular wire format and update this Data. If wireFormat is the default wire format, also
   * set the defaultWireEncoding field to a copy of the input. (To not copy the input, see wireDecode(Blob).)
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   * @param wireFormat A WireFormat object used to decode the input. If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  wireDecode
    (const uint8_t* input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(Blob(input, inputLength), wireFormat);
  }

  /**
   * Decode the input using a particular wire format and update this Data. If wireFormat is the default wire format, also
   * set the defaultWireEncoding field to a copy of the input. (To not copy the input, see wireDecode(Blob).)
   * @param input The input byte array to be decoded.
   * @param wireFormat A WireFormat object used to decode the input. If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  wireDecode(const std::vector<uint8_t>& input, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Set dataLite to point to the values in this Data object, without copying
   * any memory.
   * WARNING: The resulting pointers in dataLite are invalid after a further use
   * of this object which could reallocate memory.
   * @param dataLite a DataLite object where the name components array is
   * already allocated.
   */
  void
  get(DataLite& dataLite) const;

  /**
   * Clear this data object, and set the values by copying from dataLite.
   * @param dataLite A DataLite object.
   */
  void
  set(const DataLite& dataLite);

  const Signature*
  getSignature() const { return signature_.get(); }

  Signature*
  getSignature() { return signature_.get(); }

  const Name&
  getName() const { return name_.get(); }

  Name&
  getName() { return name_.get(); }

  const MetaInfo&
  getMetaInfo() const { return metaInfo_.get(); }

  MetaInfo& getMetaInfo() { return metaInfo_.get(); }

  const Blob&
  getContent() const { return content_; }

  /**
   * Get the incoming face ID according to the incoming packet header.
   * @return The incoming face ID. If not specified, return (uint64_t)-1.
   */
  uint64_t
  getIncomingFaceId() const;

  /**
   * Get the congestion mark according to the incoming packet header.
   * @return The congestion mark. If not specified, return 0.
   */
  uint64_t
  getCongestionMark() const;

  /**
   * Get the Data packet's full name, which includes the final
   * ImplicitSha256Digest component based on the wire encoding for a particular
   * wire format.
   * @param wireFormat A WireFormat object used to encode the Data packet. If
   * omitted, use WireFormat getDefaultWireFormat().
   * @return The full name. You must not change the Name objects - if you need
   * to change it then make a copy.
   */
  ptr_lib::shared_ptr<Name>
  getFullName(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Return a reference to the defaultWireEncoding, which was encoded with
   * getDefaultWireEncodingFormat().  The SignedBlob may have a null pointer.
   */
  const SignedBlob&
  getDefaultWireEncoding() const
  {
    if (getDefaultWireEncodingChangeCount_ != getChangeCount()) {
      // The values have changed, so the default wire encoding is invalidated.
      // This method can be called on a const object, but we want to be able to update the default cached value.
      const_cast<Data*>(this)->defaultWireEncoding_ = SignedBlob();
      const_cast<Data*>(this)->defaultWireEncodingFormat_ = 0;
      const_cast<Data*>(this)->getDefaultWireEncodingChangeCount_ = getChangeCount();
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
   * Set the signature to a copy of the given signature.
   * @param signature The signature object which is cloned.
   * @return This Data so that you can chain calls to update values.
   */
  Data&
  setSignature(const Signature& signature)
  {
    signature_.set(signature.clone());
    ++changeCount_;
    return *this;
  }

  /**
   * Set name to a copy of the given Name.  This is virtual so that a subclass can override to validate the name.
   * @param name The Name which is copied.
   * @return This Data so that you can chain calls to update values.
   */
  virtual Data&
  setName(const Name& name);

  /**
   * Set metaInfo to a copy of the given MetaInfo.
   * @param metaInfo The MetaInfo which is copied.
   * @return This Data so that you can chain calls to update values.
   */
  Data&
  setMetaInfo(const MetaInfo& metaInfo)
  {
    metaInfo_.set(metaInfo);
    ++changeCount_;
    return *this;
  }

  /**
   * Set the content to a copy of the data in the vector.
   * @param content A vector whose contents are copied.
   * @return This Data so that you can chain calls to update values.
   */
  Data&
  setContent(const std::vector<uint8_t>& content)
  {
    return setContent(Blob(content));
  }

  Data&
  setContent(const uint8_t* content, size_t contentLength)
  {
    return setContent(Blob(content, contentLength));
  }

  Data&
  setContent(const Blob& content)
  {
    content_ = content;
    ++changeCount_;
    return *this;
  }

  /**
   * An internal library method to set the LpPacket for an incoming packet. The
   * application should not call this.
   * @param lpPacket The LpPacket. This does not make a copy.
   * @return This Data so that you can chain calls to update values.
   * @note This is an experimental feature. This API may change in the future.
   */
  Data&
  setLpPacket(const ptr_lib::shared_ptr<LpPacket>& lpPacket)
  {
    lpPacket_ = lpPacket;
    // Don't update changeCount_ since this doesn't affect the wire encoding.
    return *this;
  }

  /**
   * Get the change count, which is incremented each time this object (or a child object) is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const
  {
    // Make sure each of the checkChanged is called.
    bool changed = signature_.checkChanged();
    changed = name_.checkChanged() || changed;
    changed = metaInfo_.checkChanged() || changed;
    if (changed)
      // A child object has changed, so update the change count.
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      ++const_cast<Data*>(this)->changeCount_;

    return changeCount_;
  }

private:
  void
  setDefaultWireEncoding
    (const SignedBlob& defaultWireEncoding,
     WireFormat *defaultWireEncodingFormat)
  {
    defaultWireEncoding_ = defaultWireEncoding;
    defaultWireEncodingFormat_ = defaultWireEncodingFormat;
    // Set getDefaultWireEncodingChangeCount_ so that the next call to
    //   getDefaultWireEncoding() won't clear defaultWireEncoding_.
    getDefaultWireEncodingChangeCount_ = getChangeCount();
  }

  SharedPointerChangeCounter<Signature> signature_;
  ChangeCounter<Name> name_;
  ChangeCounter<MetaInfo> metaInfo_;
  Blob content_;
  SignedBlob defaultWireEncoding_;
  WireFormat *defaultWireEncodingFormat_;
  ptr_lib::shared_ptr<Name> defaultFullName_;
  uint64_t getDefaultWireEncodingChangeCount_;
  ptr_lib::shared_ptr<LpPacket> lpPacket_;
  uint64_t changeCount_;
};

}

#endif
