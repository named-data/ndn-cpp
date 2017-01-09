/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "lp/incoming-face-id.hpp"
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/interest.hpp>

using namespace std;

namespace ndn {

Interest& Interest::operator=(const Interest& interest)
{
  setName(interest.name_.get());
  setMinSuffixComponents(interest.minSuffixComponents_);
  setMaxSuffixComponents(interest.maxSuffixComponents_);
  setKeyLocator(interest.keyLocator_.get());
  setExclude(interest.exclude_.get());
  setChildSelector(interest.childSelector_);
  mustBeFresh_ = interest.mustBeFresh_;
  setInterestLifetimeMilliseconds(interest.interestLifetimeMilliseconds_);
  setNonce(interest.nonce_);

  linkWireEncoding_ = interest.linkWireEncoding_;
  linkWireEncodingFormat_ = interest.linkWireEncodingFormat_;
  if (interest.link_.get())
    link_.set(ptr_lib::make_shared<Link>(*interest.link_.get()));
  setSelectedDelegationIndex(interest.selectedDelegationIndex_);

  setDefaultWireEncoding
    (interest.getDefaultWireEncoding(), interest.defaultWireEncodingFormat_);

  return *this;
}

uint64_t
Interest::getIncomingFaceId() const
{
  ptr_lib::shared_ptr<IncomingFaceId> field;
  if (lpPacket_)
    field = IncomingFaceId::getFirstHeader(*lpPacket_);

  return field ? field->getFaceId() : (uint64_t)-1;
}

void
Interest::get(InterestLite& interestLite, WireFormat& wireFormat) const
{
  name_.get().get(interestLite.getName());
  interestLite.setMinSuffixComponents(minSuffixComponents_);
  interestLite.setMaxSuffixComponents(maxSuffixComponents_);
  keyLocator_.get().get(interestLite.getKeyLocator());
  exclude_.get().get(interestLite.getExclude());
  interestLite.setChildSelector(childSelector_);
  interestLite.setMustBeFresh(mustBeFresh_);
  interestLite.setInterestLifetimeMilliseconds(interestLifetimeMilliseconds_);
  interestLite.setNonce(nonce_);
  try {
    interestLite.setLinkWireEncoding(getLinkWireEncoding(wireFormat));
  } catch (...) {
    // Can't encode the link object.
    interestLite.setLinkWireEncoding(Blob());
  }
  interestLite.setSelectedDelegationIndex(selectedDelegationIndex_);
}

void
Interest::set(const InterestLite& interestLite, WireFormat& wireFormat)
{
  name_.get().set(interestLite.getName());
  setMinSuffixComponents(interestLite.getMinSuffixComponents());
  setMaxSuffixComponents(interestLite.getMaxSuffixComponents());

  keyLocator_.get().set(interestLite.getKeyLocator());

  exclude_.get().set(interestLite.getExclude());
  setChildSelector(interestLite.getChildSelector());
  mustBeFresh_ = (interestLite.getMustBeFresh());
  setInterestLifetimeMilliseconds(interestLite.getInterestLifetimeMilliseconds());
  if (interestLite.getLinkWireEncoding().buf())
    setLinkWireEncoding(interestLite.getLinkWireEncoding(), wireFormat);
  else
    unsetLink();
  setSelectedDelegationIndex(interestLite.getSelectedDelegationIndex());

  // Set the nonce last so that getNonceChangeCount_ is set correctly.
  nonce_ = Blob(interestLite.getNonce());
  // Set getNonceChangeCount_ so that the next call to getNonce() won't clear nonce_.
  getNonceChangeCount_ = getChangeCount();
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
  selectors << "&ndn.MustBeFresh=" << (mustBeFresh_ ? 1 : 0);
  if (interestLifetimeMilliseconds_ >= 0)
    selectors << "&ndn.InterestLifetime=" << (uint64_t)round(interestLifetimeMilliseconds_);
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

bool
Interest::matchesData(const Data& data, WireFormat& wireFormat) const
{
  // Imitate ndn-cxx Interest::matchesData.
  size_t interestNameLength = getName().size();
  const Name& dataName = data.getName();
  size_t fullNameLength = dataName.size() + 1;

  // Check MinSuffixComponents.
  bool hasMinSuffixComponents = (getMinSuffixComponents() >= 0);
  int minSuffixComponents =
    (hasMinSuffixComponents ? getMinSuffixComponents() : 0);
  if (!(interestNameLength + minSuffixComponents <= fullNameLength))
    return false;

  // Check MaxSuffixComponents.
  bool hasMaxSuffixComponents = (getMaxSuffixComponents() >= 0);
  if (hasMaxSuffixComponents &&
      !(interestNameLength + getMaxSuffixComponents() >= fullNameLength))
    return false;

  // Check the prefix.
  if (interestNameLength == fullNameLength) {
    if (getName().get(-1).isImplicitSha256Digest()) {
      if (!getName().equals(*data.getFullName(wireFormat)))
        return false;
    }
    else
      // The Interest Name is the same length as the Data full Name, but the
      //   last component isn't a digest so there's no possibility of matching.
      return false;
  }
  else {
    // The Interest Name should be a strict prefix of the Data full Name,
    if (!getName().isPrefixOf(dataName))
      return false;
  }

  // Check the Exclude.
  // The Exclude won't be violated if the Interest Name is the same as the
  //   Data full Name.
  if (getExclude().size() > 0 && fullNameLength > interestNameLength) {
    if (interestNameLength == fullNameLength - 1) {
      // The component to exclude is the digest.
      if (getExclude().matches
          (data.getFullName(wireFormat)->get(interestNameLength)))
        return false;
    }
    else {
      // The component to exclude is not the digest.
      if (getExclude().matches(dataName.get(interestNameLength)))
        return false;
    }
  }

  // Check the KeyLocator.
  const KeyLocator& publisherPublicKeyLocator = getKeyLocator();
  if (publisherPublicKeyLocator.getType() >= 0) {
    const Signature* signature = data.getSignature();
    if (!KeyLocator::canGetFromSignature(signature))
      // No KeyLocator in the Data packet.
      return false;
    if (!publisherPublicKeyLocator.equals
        (KeyLocator::getFromSignature(signature)))
      return false;
  }

  return true;
}

Link*
Interest::getLink()
{
  if (link_.get())
    return link_.get();
  else if (!linkWireEncoding_.isNull()) {
    // Decode the link object from linkWireEncoding_.
    ptr_lib::shared_ptr<Link> link(new Link());
    link->wireDecode(linkWireEncoding_, *linkWireEncodingFormat_);
    link_.set(link);

    // Clear linkWireEncoding_ since it is now managed by the link object.
    linkWireEncoding_ = Blob();
    linkWireEncodingFormat_ = 0;

    return link.get();
  }
  else
    return 0;
}

Blob
Interest::getLinkWireEncoding(WireFormat& wireFormat) const
{
  if (!linkWireEncoding_.isNull() && linkWireEncodingFormat_ == &wireFormat)
    return linkWireEncoding_;

  const Link* link = getLink();
  if (link)
    return link->wireEncode(wireFormat);
  else
    return Blob();
}

void
Interest::refreshNonce()
{
  Blob currentNonce = getNonce();
  if (currentNonce.size() == 0)
    return;

  ptr_lib::shared_ptr<vector<uint8_t> > newNonce
    (new vector<uint8_t>(currentNonce.size()));
  while (true) {
    ndn_Error error;
    if ((error = CryptoLite::generateRandomBytes
         (&newNonce->front(), newNonce->size())))
      throw runtime_error(ndn_getErrorString(error));
    // Use the vector equals operator.
    if (*newNonce != *currentNonce)
      break;
  }

  nonce_ = Blob(newNonce, false);
  // Set getNonceChangeCount_ so that the next call to getNonce() won't clear
  // nonce_.
  ++changeCount_;
  getNonceChangeCount_ = getChangeCount();
}

}

