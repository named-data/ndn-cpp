/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include <ndn-cpp/common.hpp>
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
  setDefaultWireEncoding(data.defaultWireEncoding_);
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
  setDefaultWireEncoding(data.defaultWireEncoding_);
  
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
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  Blob encoding = wireFormat.encodeData(*this, &signedPortionBeginOffset, &signedPortionEndOffset);
  SignedBlob wireEncoding = SignedBlob(encoding, signedPortionBeginOffset, signedPortionEndOffset);
  
  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    const_cast<Data*>(this)->setDefaultWireEncoding(wireEncoding);
  
  return wireEncoding;
}

void 
Data::wireDecode(const uint8_t* input, size_t inputLength, WireFormat& wireFormat) 
{
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  wireFormat.decodeData(*this, input, inputLength, &signedPortionBeginOffset, &signedPortionEndOffset);
  
  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    setDefaultWireEncoding(SignedBlob(input, inputLength, signedPortionBeginOffset, signedPortionEndOffset));
  else
    setDefaultWireEncoding(SignedBlob());
}

}
