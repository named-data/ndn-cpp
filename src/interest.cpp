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

#include <math.h>
#include <stdexcept>
#include <ndn-cpp/common.hpp>
#include "c/interest.h"
#include <ndn-cpp/interest.hpp>

using namespace std;

namespace ndn {

Interest::Interest(const Name& name, int minSuffixComponents, int maxSuffixComponents,
  const PublisherPublicKeyDigest& publisherPublicKeyDigest, const Exclude& exclude, int childSelector, int answerOriginKind,
  int scope, Milliseconds interestLifetimeMilliseconds, const Blob& nonce)
: name_(name), minSuffixComponents_(minSuffixComponents), maxSuffixComponents_(maxSuffixComponents),
  publisherPublicKeyDigest_(publisherPublicKeyDigest), exclude_(exclude), childSelector_(childSelector),
  answerOriginKind_(answerOriginKind), scope_(scope), interestLifetimeMilliseconds_(interestLifetimeMilliseconds),
  nonce_(nonce), getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("publisherPublicKeyDigest and answerOriginKind are for NDNx and are deprecated. To enable while you upgrade your code to use NFD's KeyLocator and setMustBeFresh(), set WireFormat::ENABLE_NDNX = true");
}

Interest::Interest(const Name& name, int minSuffixComponents, int maxSuffixComponents,
  const PublisherPublicKeyDigest& publisherPublicKeyDigest, const Exclude& exclude, int childSelector, int answerOriginKind,
  int scope, Milliseconds interestLifetimeMilliseconds)
: name_(name), minSuffixComponents_(minSuffixComponents), maxSuffixComponents_(maxSuffixComponents),
  publisherPublicKeyDigest_(publisherPublicKeyDigest), exclude_(exclude), childSelector_(childSelector),
  answerOriginKind_(answerOriginKind), scope_(scope), interestLifetimeMilliseconds_(interestLifetimeMilliseconds),
  getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("publisherPublicKeyDigest and answerOriginKind are for NDNx and are deprecated. To enable while you upgrade your code to use NFD's KeyLocator and setMustBeFresh(), set WireFormat::ENABLE_NDNX = true");
}

Interest::Interest(const Name& name, int minSuffixComponents, int maxSuffixComponents,
  const KeyLocator& keyLocator, const Exclude& exclude, int childSelector, int answerOriginKind,
  int scope, Milliseconds interestLifetimeMilliseconds)
: name_(name), minSuffixComponents_(minSuffixComponents), maxSuffixComponents_(maxSuffixComponents),
  keyLocator_(keyLocator), exclude_(exclude), childSelector_(childSelector),
  answerOriginKind_(answerOriginKind), scope_(scope), interestLifetimeMilliseconds_(interestLifetimeMilliseconds),
  getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("answerOriginKind is for NDNx and is deprecated. To enable while you upgrade your code to use NFD's setMustBeFresh(), set WireFormat::ENABLE_NDNX = true");
}

Interest& Interest::operator=(const Interest& interest)
{
  setName(interest.name_.get());
  setMinSuffixComponents(interest.minSuffixComponents_);
  setMaxSuffixComponents(interest.maxSuffixComponents_);
  publisherPublicKeyDigest_.set(interest.publisherPublicKeyDigest_.get());;
  setKeyLocator(interest.keyLocator_.get());
  setExclude(interest.exclude_.get());
  setChildSelector(interest.childSelector_);
  answerOriginKind_ = interest.answerOriginKind_;
  setScope(interest.scope_);
  setInterestLifetimeMilliseconds(interest.interestLifetimeMilliseconds_);
  setNonce(interest.nonce_);
  setDefaultWireEncoding
    (interest.defaultWireEncoding_, interest.defaultWireEncodingFormat_);

  return *this;
}

void
Interest::set(const struct ndn_Interest& interestStruct)
{
  name_.get().set(interestStruct.name);
  setMinSuffixComponents(interestStruct.minSuffixComponents);
  setMaxSuffixComponents(interestStruct.maxSuffixComponents);

  publisherPublicKeyDigest_.get().set(interestStruct.publisherPublicKeyDigest);
  keyLocator_.get().set(interestStruct.keyLocator);

  exclude_.get().set(interestStruct.exclude);
  setChildSelector(interestStruct.childSelector);
  answerOriginKind_ = interestStruct.answerOriginKind;
  setScope(interestStruct.scope);
  setInterestLifetimeMilliseconds(interestStruct.interestLifetimeMilliseconds);
  // Set the nonce last so that getNonceChangeCount_ is set correctly.
  nonce_ = Blob(interestStruct.nonce);
  // Set getNonceChangeCount_ so that the next call to getNonce() won't clear nonce_.
  getNonceChangeCount_ = getChangeCount();
}

void
Interest::get(struct ndn_Interest& interestStruct) const
{
  name_.get().get(interestStruct.name);
  interestStruct.minSuffixComponents = minSuffixComponents_;
  interestStruct.maxSuffixComponents = maxSuffixComponents_;
  publisherPublicKeyDigest_.get().get(interestStruct.publisherPublicKeyDigest);
  keyLocator_.get().get(interestStruct.keyLocator);
  exclude_.get().get(interestStruct.exclude);
  interestStruct.childSelector = childSelector_;
  interestStruct.answerOriginKind = answerOriginKind_;
  interestStruct.scope = scope_;
  interestStruct.interestLifetimeMilliseconds = interestLifetimeMilliseconds_;
  getNonce().get(interestStruct.nonce);
}

SignedBlob
Interest::wireEncode(WireFormat& wireFormat) const
{
  if (getDefaultWireEncoding() && getDefaultWireEncodingFormat() == &wireFormat)
    // We already have an encoding in the desired format.
    return getDefaultWireEncoding();

  size_t signedPortionBeginOffset, signedPortionEndOffset;
  Blob encoding(wireFormat.encodeInterest(*this, &signedPortionBeginOffset,
                &signedPortionEndOffset));
  SignedBlob wireEncoding = SignedBlob
    (encoding, signedPortionBeginOffset, signedPortionEndOffset);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    const_cast<Interest*>(this)->setDefaultWireEncoding
      (wireEncoding, WireFormat::getDefaultWireFormat());

  return wireEncoding;
}

void
Interest::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  wireFormat.decodeInterest
    (*this, input.buf(), input.size(), &signedPortionBeginOffset,
     &signedPortionEndOffset);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    // Take a pointer to the input Blob without copying.
    setDefaultWireEncoding
      (SignedBlob(input, signedPortionBeginOffset, signedPortionEndOffset),
       WireFormat::getDefaultWireFormat());
  else
    setDefaultWireEncoding(SignedBlob(), 0);
}

void
Interest::wireDecode
  (const uint8_t *input, size_t inputLength, WireFormat& wireFormat)
{
  size_t signedPortionBeginOffset, signedPortionEndOffset;
  wireFormat.decodeInterest(*this, input, inputLength, &signedPortionBeginOffset, &signedPortionEndOffset);

  if (&wireFormat == WireFormat::getDefaultWireFormat())
    // This is the default wire encoding.
    // The input is not an immutable Blob, so we need to copy.
    setDefaultWireEncoding
      (SignedBlob(input, inputLength, signedPortionBeginOffset, signedPortionEndOffset),
       WireFormat::getDefaultWireFormat());
  else
    setDefaultWireEncoding(SignedBlob(), 0);
}

string
Interest::toUri() const
{
  ostringstream selectors;

  if (minSuffixComponents_ >= 0)
    selectors << "&ndn.MinSuffixComponents=" << minSuffixComponents_;
  if (maxSuffixComponents_ >= 0)
    selectors << "&ndn.MaxSuffixComponents=" << maxSuffixComponents_;
  if (childSelector_ >= 0)
    selectors << "&ndn.ChildSelector=" << childSelector_;
  if (answerOriginKind_ >= 0)
    selectors << "&ndn.AnswerOriginKind=" << answerOriginKind_;
  if (scope_ >= 0)
    selectors << "&ndn.Scope=" << scope_;
  if (interestLifetimeMilliseconds_ >= 0)
    selectors << "&ndn.InterestLifetime=" << (uint64_t)round(interestLifetimeMilliseconds_);
  if (publisherPublicKeyDigest_.get().getPublisherPublicKeyDigest().size() > 0) {
    selectors << "&ndn.PublisherPublicKeyDigest=";
    Name::toEscapedString(*publisherPublicKeyDigest_.get().getPublisherPublicKeyDigest(), selectors);
  }
  if (getNonce().size() > 0) {
    selectors << "&ndn.Nonce=";
    Name::toEscapedString(*getNonce(), selectors);
  }
  if (exclude_.get().size() > 0)
    selectors << "&ndn.Exclude=" << exclude_.get().toUri();

  ostringstream result;

  result << name_.get().toUri();
  string selectorsString(selectors.str());
  if (selectorsString.size() > 0) {
    // Replace the first & with ?.
    result << "?";
    result.write(&selectorsString[1], selectorsString.size() - 1);
  }

  return result.str();
}

bool
Interest::matchesName(const Name& name) const
{
  if (!getName().match(name))
    return false;

  if (minSuffixComponents_ >= 0 &&
    // Add 1 for the implicit digest.
    !(name.size() + 1 - getName().size() >= (size_t)minSuffixComponents_))
    return false;
  if (maxSuffixComponents_ >= 0 &&
    // Add 1 for the implicit digest.
    !(name.size() + 1 - getName().size() <= (size_t)maxSuffixComponents_))
    return false;
  if (getExclude().size() > 0 && name.size() > getName().size() &&
      getExclude().matches(name.get(getName().size())))
    return false;

  return true;
}

int
Interest::getAnswerOriginKind() const
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("getAnswerOriginKind is for NDNx and is deprecated. To enable while you upgrade your code to use NFD's getMustBeFresh(), set WireFormat::ENABLE_NDNX = true");

  return answerOriginKind_;
}

Interest&
Interest::setAnswerOriginKind(int answerOriginKind)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("setAnswerOriginKind is for NDNx and is deprecated. To enable while you upgrade your code to use NFD's setMustBeFresh(), set WireFormat::ENABLE_NDNX = true");

  answerOriginKind_ = answerOriginKind;
  ++changeCount_;
  return *this;
}

}

