/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

struct ndn_Data;

namespace ndn {

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
   * Set the dataStruct to point to the values in this interest, without copying any memory.
   * WARNING: The resulting pointers in dataStruct are invalid after a further use of this object which could reallocate memory.
   * @param dataStruct a C ndn_Data struct where the name components array is already allocated.
   */
  void
  get(struct ndn_Data& dataStruct) const;

  /**
   * Clear this data object, and set the values by copying from the ndn_Data struct.
   * @param dataStruct a C ndn_Data struct
   */
  void
  set(const struct ndn_Data& dataStruct);

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
  uint64_t getDefaultWireEncodingChangeCount_;
  uint64_t changeCount_;
};

}

#endif
