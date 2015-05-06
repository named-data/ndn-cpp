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

#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "c/data.h"
#include <ndn-cpp/data.hpp>

using namespace std;

namespace ndn {

Data::Data()
: signature_(new Sha256WithRsaSignature()), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
{
}

Data::Data(const Name& name)
: name_(name), signature_(new Sha256WithRsaSignature()), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
{
}

Data::Data(const Data& data)
: name_(data.name_), metaInfo_(data.metaInfo_), content_(data.content_), changeCount_(0)
{
  if (data.signature_.get()) {
    signature_.set(data.signature_.get()->clone());
    ++changeCount_;
  }
  setDefaultWireEncoding
    (data.defaultWireEncoding_, data.defaultWireEncodingFormat_);
}

Data::~Data()
{
}

Data& Data::operator=(const Data& data)
{
  if (data.signature_.get())
    signature_.set(data.signature_.get()->clone());
  else
    signature_.set(ptr_lib::shared_ptr<Signature>());

  setName(data.name_.get());
  setMetaInfo(data.metaInfo_.get());
  setContent(data.content_);
  setDefaultWireEncoding
    (data.defaultWireEncoding_, data.defaultWireEncodingFormat_);

  return *this;
}

void
Data::get(struct ndn_Data& dataStruct) const
{
  signature_.get()->get(dataStruct.signature);
  name_.get().get(dataStruct.name);
  metaInfo_.get().get(dataStruct.metaInfo);
  content_.get(dataStruct.content);
}

void
Data::set(const struct ndn_Data& dataStruct)
{
  if (dataStruct.signature.type == ndn_SignatureType_Sha256WithRsaSignature)
    signature_.set(ptr_lib::shared_ptr<Signature>(new Sha256WithRsaSignature()));
  else if (dataStruct.signature.type == ndn_SignatureType_Sha256WithEcdsaSignature)
    signature_.set(ptr_lib::shared_ptr<Signature>(new Sha256WithEcdsaSignature()));
  else if (dataStruct.signature.type == ndn_SignatureType_DigestSha256Signature)
    signature_.set(ptr_lib::shared_ptr<Signature>(new DigestSha256Signature()));
  else
    // We don't expect this to happen.
    throw runtime_error("dataStruct.signature.type has an unrecognized value");

  signature_.get()->set(dataStruct.signature);

  name_.get().set(dataStruct.name);
  metaInfo_.get().set(dataStruct.metaInfo);
  setContent(Blob(dataStruct.content));
}

Data&
Data::setName(const Name& name)
{
  name_.set(name);
  ++changeCount_;
  return *this;
}

SignedBlob
Data::wireEncode(WireFormat& wireFormat) const
{
  if (getDefaultWireEncoding() && getDefaultWireEncodingFormat() == &wireFormat)
    // We already have an encoding in the desired format.
    return getDefaultWireEncoding();

  size_t signedPortionBeginOffset, signedPortionEndOffset;
  Blob encoding = wireFormat.encodeData(*this, &signedPortionBeginOffset, &signedPortionEndOffset);
  SignedBlob wireEncoding = SignedBlob(encoding, signedPortionBeginOffset, signedPortionEndOffset);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    const_cast<Data*>(this)->setDefaultWireEncoding
      (wireEncoding, WireFormat::getDefaultWireFormat());

  return wireEncoding;
}

void
Data::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  wireFormat.decodeData(*this, input.buf(), input.size(), &signedPortionBeginOffset, &signedPortionEndOffset);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    // Take a pointer to the input Blob without copying.
    setDefaultWireEncoding
      (SignedBlob(input, signedPortionBeginOffset, signedPortionEndOffset),
       WireFormat::getDefaultWireFormat());
  else
    setDefaultWireEncoding(SignedBlob(), 0);
}

}
