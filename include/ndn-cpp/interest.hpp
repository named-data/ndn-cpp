/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2016 Regents of the University of California.
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

#ifndef NDN_INTEREST_HPP
#define NDN_INTEREST_HPP

#include "name.hpp"
#include "link.hpp"
#include "key-locator.hpp"
#include "lite/interest-lite.hpp"
#include "encoding/wire-format.hpp"
#include "util/signed-blob.hpp"
#include "util/change-counter.hpp"
#include "exclude.hpp"

namespace ndn {

/**
 * An Interest holds a Name and other fields for an interest.
 */
class Interest {
public:
  /**
   * Create a new Interest with the given name and interest lifetime and "none" for other values.
   * @param name The name for the interest.
   * @param interestLifetimeMilliseconds The interest lifetime in milliseconds, or -1 for none.
   */
  Interest(const Name& name, Milliseconds interestLifetimeMilliseconds)
  : name_(name), getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
  {
    construct();
    interestLifetimeMilliseconds_ = interestLifetimeMilliseconds;
  }

  /**
   * Create a new Interest with the given name and "none" for other values.
   * @param name The name for the interest.
   */
  Interest(const Name& name)
  : name_(name), getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
  {
    construct();
  }

  Interest(const Interest& interest)
  : name_(interest.name_), minSuffixComponents_(interest.minSuffixComponents_),
    maxSuffixComponents_(interest.maxSuffixComponents_),
    keyLocator_(interest.keyLocator_), exclude_(interest.exclude_),
    childSelector_(interest.childSelector_),
    mustBeFresh_(interest.mustBeFresh_),
    interestLifetimeMilliseconds_(interest.interestLifetimeMilliseconds_),
    nonce_(interest.nonce_), getNonceChangeCount_(0),
    linkWireEncoding_(interest.linkWireEncoding_),
    linkWireEncodingFormat_(interest.linkWireEncodingFormat_),
    selectedDelegationIndex_(interest.selectedDelegationIndex_),
    changeCount_(0)
  {
    if (interest.link_.get())
      link_.set(ptr_lib::make_shared<Link>(*interest.link_.get()));

    setDefaultWireEncoding
      (interest.getDefaultWireEncoding(), interest.defaultWireEncodingFormat_);
  }

  /**
   * Create a new Interest with an empty name and "none" for all values.
   */
  Interest()
  : getNonceChangeCount_(0), changeCount_(0), getDefaultWireEncodingChangeCount_(0)
  {
    construct();
  }

  Interest& operator=(const Interest& interest);

  /**
   * Encode this Interest for a particular wire format. If wireFormat is the
   * default wire format, also set the defaultWireEncoding field to the encoded
   * result. Even though this is const, if wireFormat is the default wire format
   * we update the defaultWireEncoding.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * Interest. If omitted, use WireFormat::getDefaultWireFormat().
   * @return The encoded byte array.
   */
  SignedBlob
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Decode the input using a particular wire format and update this Interest.
   * If wireFormat is the default wire format, also set the defaultWireEncoding
   * to another pointer to the input Blob.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Decode the input using a particular wire format and update this Interest.
   * If wireFormat is the default wire format, also set the defaultWireEncoding
   * field to a copy of the input. (To not copy the input, see wireDecode(Blob).)
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
   * Decode the input using a particular wire format and update this Interest.
   * If wireFormat is the default wire format, also set the defaultWireEncoding
   * field to a copy of the input. (To not copy the input, see wireDecode(Blob).)
   * @param input The input byte array to be decoded.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode(const std::vector<uint8_t>& input, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Encode the name according to the "NDN URI Scheme".  If there are interest
   * selectors, append "?" and add the selectors as a query string.  For example
   * "/test/name?ndn.ChildSelector=1".
   * @return The URI string.
   * @note This is an experimental feature.  See the API docs for more detail at
   * http://named-data.net/doc/ndn-ccl-api/interest.html#interest-touri-method .
   */
  std::string
  toUri() const;

  /**
   * Set interestLite to point to the values and name components in this
   * interest, without copying any memory.
   * WARNING: The resulting pointers in interestLite are invalid after a further
   * use of this object which could reallocate memory.
   * @param interestLite A InterestLite where the name components array is
   * already allocated.
   * @param wireFormat The desired wire format for encoding the link object (if
   * necessary).
   */
  void
  get(InterestLite& interestLite, WireFormat& wireFormat) const;

  /**
   * Clear this interest, and set the values by copying from interestLite.
   * @param interestLite An InterestLite object.
   * @param wireFormat The wire format of the encoding, to be used later if
   * necessary to decode the link wire encoding.
   */
  void
  set(const InterestLite& interestLite, WireFormat& wireFormat);

  Name&
  getName() { return name_.get(); }

  const Name&
  getName() const { return name_.get(); }

  int
  getMinSuffixComponents() const { return minSuffixComponents_; }

  int
  getMaxSuffixComponents() const { return maxSuffixComponents_; }

  const KeyLocator&
  getKeyLocator() const { return keyLocator_.get(); }

  KeyLocator&
  getKeyLocator() { return keyLocator_.get(); }

  Exclude&
  getExclude() { return exclude_.get(); }

  const Exclude&
  getExclude() const { return exclude_.get(); }

  int
  getChildSelector() const { return childSelector_; }

  /**
   * Return true if the content must be fresh. The default is true.
   * @return true if must be fresh, otherwise false.
   */
  bool
  getMustBeFresh() const { return mustBeFresh_; }

  Milliseconds
  getInterestLifetimeMilliseconds() const { return interestLifetimeMilliseconds_; }

  /**
   * Return the nonce value from the incoming interest.  If you change any of the fields in this Interest object,
   * then the nonce value is cleared.
   * @return
   */
  const Blob&
  getNonce() const
  {
    if (getNonceChangeCount_ != getChangeCount()) {
      // The values have changed, so the existing nonce is invalidated.
      // This method can be called on a const object, but we want to be able to update the default cached value.
      const_cast<Interest*>(this)->nonce_ = Blob();
      const_cast<Interest*>(this)->getNonceChangeCount_ = getChangeCount();
    }

    return nonce_;
  }

  /**
   * Check if this interest has a link object (or a link wire encoding which
   * can be decoded to make the link object).
   * @return True if this interest has a link object, false if not.
   */
  bool
  hasLink() const
  {
    return link_.get() || !linkWireEncoding_.isNull();
  }


  /**
   * Get the link object. If necessary, decode it from the link wire encoding.
   * @return  The link object, or 0 if not specified.
   * @throws runtime_error For error decoding the link wire encoding (if
   * necessary).
   */
  Link*
  getLink();

  const Link*
  getLink() const { return const_cast<Interest*>(this)->getLink(); }

  /**
   * Get the wire encoding of the link object. If there is already a wire
   * encoding then return it. Otherwise encode from the link object (if
   * available).
   * @param wireFormat (optional) The desired wire format for the encoding.
   * If omitted, use WireFormat::getDefaultWireFormat().
   * @return The wire encoding, or an isNull Blob if the link is not specified.
   * @throws runtime_error for error encoding the link object.
   */
  Blob
  getLinkWireEncoding
    (WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Get the selected delegation index.
   * @return The selected delegation index. If not specified, return -1.
   */
  int
  getSelectedDelegationIndex() const { return selectedDelegationIndex_; }

  /**
   * Set the interest name.
   * @note You can also call getName and change the name values directly.
   * @param name The interest name. This makes a copy of the name.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setName(const Name& name)
  {
    name_.set(name);
    ++changeCount_;
    return *this;
  }

  /**
   * Set the min suffix components count.
   * @param minSuffixComponents The min suffix components count. If not
   * specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setMinSuffixComponents(int minSuffixComponents)
  {
    minSuffixComponents_ = minSuffixComponents;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the max suffix components count.
   * @param maxSuffixComponents The max suffix components count. If not
   * specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setMaxSuffixComponents(int maxSuffixComponents)
  {
    maxSuffixComponents_ = maxSuffixComponents;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the child selector.
   * @param childSelector The child selector. If not specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setChildSelector(int childSelector)
  {
    childSelector_ = childSelector;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the MustBeFresh flag.
   * @param mustBeFresh True if the content must be fresh, otherwise false. If
   * you do not set this flag, the default value is true.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setMustBeFresh(bool mustBeFresh)
  {
    mustBeFresh_ = mustBeFresh;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the interest lifetime.
   * @param interestLifetimeMilliseconds The interest lifetime in milliseconds.
   * If not specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setInterestLifetimeMilliseconds(Milliseconds interestLifetimeMilliseconds)
  {
    interestLifetimeMilliseconds_ = interestLifetimeMilliseconds;
    ++changeCount_;
    return *this;
  }

  /**
   * @deprecated You should let the wire encoder generate a random nonce internally before sending the interest.
   */
  Interest&
  DEPRECATED_IN_NDN_CPP setNonce(const Blob& nonce)
  {
    nonce_ = nonce;
    // Set getNonceChangeCount_ so that the next call to getNonce() won't clear nonce_.
    ++changeCount_;
    getNonceChangeCount_ = getChangeCount();
    return *this;
  }

  /**
   * Set this interest to use a copy of the given KeyLocator object.
   * @note You can also call getKeyLocator and change the key locator directly.
   * @param keyLocator The KeyLocator object. This makes a copy of the object.
   * If no key locator is specified, set to a new default KeyLocator(), or to a
   * KeyLocator with an unspecified type.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setKeyLocator(const KeyLocator& keyLocator)
  {
    keyLocator_ = keyLocator;
    ++changeCount_;
    return *this;
  }

  /**
   * Set this interest to use a copy of the given Exclude object.
   * @note You can also call getExclude and change the exclude entries directly.
   * @param exclude The Exclude object. This makes a copy of the object. If no
   * exclude is specified, set to a new default Exclude(), or to an Exclude with
   * size() 0.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setExclude(const Exclude& exclude)
  {
    exclude_ = exclude;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the link wire encoding bytes, without decoding them. If there is
   * a link object, set it to 0. If you later call getLink(), it will
   * decode the wireEncoding to create the link object.
   * @param encoding The buffer with the bytes of the link wire encoding.
   * If no link is specified, set to an empty Blob() or call unsetLink().
   * @param wireFormat (optional) The wire format of the encoding, to be used
   * later if necessary to decode. If omitted, use
   * WireFormat::getDefaultWireFormat().
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setLinkWireEncoding
    (Blob encoding,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    linkWireEncoding_ = encoding;
    linkWireEncodingFormat_ = &wireFormat;

    // Clear the link object, assuming that it has a different encoding.
    link_.set(ptr_lib::shared_ptr<Link>());

    ++changeCount_;
    return *this;
  }

  /**
   * Clear the link wire encoding and link object so that getLink() returns null.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  unsetLink()
  {
    WireFormat* wireFormat = 0;
    return setLinkWireEncoding(Blob(), *wireFormat);
  }

  /**
   * Set the selected delegation index.
   * @param selectedDelegationIndex The selected delegation index. If not
   * specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest*
  setSelectedDelegationIndex(int selectedDelegationIndex)
  {
    selectedDelegationIndex_ = selectedDelegationIndex;
    ++changeCount_;
    return this;
  }

  /**
   * Update the bytes of the nonce with new random values. This ensures that the
   * new nonce value is different than the current one. If the current nonce is
   * not specified, this does nothing.
   */
  void
  refreshNonce();

  /**
   * Check if this Interest's name matches the given name (using Name::match)
   * and the given name also conforms to the interest selectors.
   * @param name The name to check.
   * @return True if the name and interest selectors match, otherwise false.
   */
  bool
  matchesName(const Name& name) const;

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
      const_cast<Interest*>(this)->defaultWireEncoding_ = SignedBlob();
      const_cast<Interest*>(this)->defaultWireEncodingFormat_ = 0;
      const_cast<Interest*>(this)->getDefaultWireEncodingChangeCount_ = getChangeCount();
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
   * Get the change count, which is incremented each time this object (or a child object) is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const
  {
    // Make sure each of the checkChanged is called.
    bool changed = name_.checkChanged();
    changed = keyLocator_.checkChanged() || changed;
    changed = exclude_.checkChanged() || changed;
    changed = link_.checkChanged() || changed;
    if (changed)
      // A child object has changed, so update the change count.
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      ++const_cast<Interest*>(this)->changeCount_;

    return changeCount_;
  }

private:
  void
  construct()
  {
    minSuffixComponents_ = -1;
    maxSuffixComponents_ = -1;
    childSelector_ = -1;
    mustBeFresh_ = true;
    interestLifetimeMilliseconds_ = -1.0;
    linkWireEncodingFormat_ = 0;
    selectedDelegationIndex_ = -1;
  }

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

  ChangeCounter<Name> name_;
  int minSuffixComponents_; /**< -1 for none */
  int maxSuffixComponents_; /**< -1 for none */
  ChangeCounter<KeyLocator> keyLocator_;
  ChangeCounter<Exclude> exclude_;
  int childSelector_;       /**< -1 for none */
  bool mustBeFresh_;
  Milliseconds interestLifetimeMilliseconds_; /**< -1 for none */
  Blob nonce_;
  uint64_t getNonceChangeCount_;
  Blob linkWireEncoding_;
  WireFormat* linkWireEncodingFormat_;
  SharedPointerChangeCounter<Link> link_;
  int selectedDelegationIndex_; /**< -1 for none */
  SignedBlob defaultWireEncoding_;
  WireFormat *defaultWireEncodingFormat_;
  uint64_t getDefaultWireEncodingChangeCount_;
  uint64_t changeCount_;
};

}

#endif
