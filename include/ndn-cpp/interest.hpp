/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2019 Regents of the University of California.
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
#include "delegation-set.hpp"

namespace ndn {

class LpPacket;
class Data;

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
  : name_(name), getNonceChangeCount_(0), getDefaultWireEncodingChangeCount_(0), changeCount_(0)
  {
    construct();
    interestLifetimeMilliseconds_ = interestLifetimeMilliseconds;
  }

  /**
   * Create a new Interest with the given name and "none" for other values.
   * @param name The name for the interest.
   */
  Interest(const Name& name)
  : name_(name), getNonceChangeCount_(0), getDefaultWireEncodingChangeCount_(0), changeCount_(0)
  {
    construct();
  }

  Interest(const Interest& interest)
  : name_(interest.name_), minSuffixComponents_(interest.minSuffixComponents_),
    maxSuffixComponents_(interest.maxSuffixComponents_),
    didSetCanBePrefix_(interest.didSetCanBePrefix_),
    keyLocator_(interest.keyLocator_), exclude_(interest.exclude_),
    childSelector_(interest.childSelector_),
    mustBeFresh_(interest.mustBeFresh_),
    interestLifetimeMilliseconds_(interest.interestLifetimeMilliseconds_),
    nonce_(interest.nonce_), getNonceChangeCount_(0),
    forwardingHint_(interest.forwardingHint_), 
    applicationParameters_(interest.applicationParameters_),
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
  : getNonceChangeCount_(0), getDefaultWireEncodingChangeCount_(0), changeCount_(0)
  {
    construct();
  }

  Interest& operator=(const Interest& interest);

  /**
   * Get the default value of the CanBePrefix flag used in the Interest
   * constructor. You can change this with setDefaultCanBePrefix().
   * @return The default value of the CanBePrefix flag.
   */
  static bool
  getDefaultCanBePrefix() { return InterestLite::getDefaultCanBePrefix(); }

  /**
   * Set the default value of the CanBePrefix flag used in the Interest
   * constructor. The default is currently true, but will be changed at a later
   * date. The application should call this before creating any Interest
   * (even to set the default again to true), or the application should
   * explicitly call setCanBePrefix() after creating the Interest. Otherwise
   * wireEncode will print a warning message. This is to avoid breaking any code
   * when the library default for CanBePrefix is changed at a later date.
   * @param defaultCanBePrefix The default value of the CanBePrefix flag.
   */
  static void
  setDefaultCanBePrefix(bool defaultCanBePrefix)
  {
    InterestLite::setDefaultCanBePrefix(defaultCanBePrefix);
    didSetDefaultCanBePrefix_ = true;
  }

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

  /**
   * Get the CanBePrefix flag. If not specified, the default is true, or the
   * value from setDefaultCanBePrefix().
   * @return The CanBePrefix flag.
   */
  bool
  getCanBePrefix() const
  {
    // Use the closest v0.2 semantics. CanBePrefix is the opposite of exact
    // match where MaxSuffixComponents is 1 (for the implicit digest).
    return maxSuffixComponents_ != 1;
  }

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
   * Get the forwarding hint object which you can modify to add or remove
   * forwarding hints.
   * @return The forwarding hint as a DelegationSet.
   */
  DelegationSet&
  getForwardingHint() { return forwardingHint_.get(); }

  const DelegationSet&
  getForwardingHint() const { return forwardingHint_.get(); }

  /**
   * Check if the application parameters are specified.
   * @return True if the application parameters are specified, false if not.
   */
  bool
  hasApplicationParameters() const { return applicationParameters_.size() > 0; }

  /**
   * @deprecated Use hasApplicationParameters.
   */
  bool
  DEPRECATED_IN_NDN_CPP hasParameters() const { return hasApplicationParameters(); }

  /**
   * Get the application parameters.
   * @return The application parameters as a Blob, which isNull() if unspecified.
   */
  const Blob&
  getApplicationParameters() const { return applicationParameters_; }

  /**
   * @deprecated Use getApplicationParameters.
   */
  const Blob&
  DEPRECATED_IN_NDN_CPP getParameters() const { return getApplicationParameters(); }

  /**
   * Check if this interest has a link object (or a link wire encoding which
   * can be decoded to make the link object).
   * @return True if this interest has a link object, false if not.
   * @deprecated Use getForwardingHint.
   */
  bool
  DEPRECATED_IN_NDN_CPP hasLink() const
  {
    return link_.get() || !linkWireEncoding_.isNull();
  }


  /**
   * Get the link object. If necessary, decode it from the link wire encoding.
   * @return  The link object, or 0 if not specified.
   * @throws runtime_error For error decoding the link wire encoding (if
   * necessary).
   * @deprecated Use getForwardingHint.
   */
  Link*
  DEPRECATED_IN_NDN_CPP getLink();

  const Link*
  DEPRECATED_IN_NDN_CPP getLink() const { return const_cast<Interest*>(this)->getLink(); }

  /**
   * Get the wire encoding of the link object. If there is already a wire
   * encoding then return it. Otherwise encode from the link object (if
   * available).
   * @param wireFormat (optional) The desired wire format for the encoding.
   * If omitted, use WireFormat::getDefaultWireFormat().
   * @return The wire encoding, or an isNull Blob if the link is not specified.
   * @throws runtime_error for error encoding the link object.
   * @deprecated Use getForwardingHint.
   */
  Blob
  DEPRECATED_IN_NDN_CPP getLinkWireEncoding
    (WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

  /**
   * Get the selected delegation index.
   * @return The selected delegation index. If not specified, return -1.
   * @deprecated Use getForwardingHint.
   */
  int
  DEPRECATED_IN_NDN_CPP getSelectedDelegationIndex() const { return selectedDelegationIndex_; }

  /**
   * Get the incoming face ID according to the incoming packet header.
   * @return The incoming face ID. If not specified, return (uint64_t)-1.
   */
  uint64_t
  getIncomingFaceId() const;

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
   * Set the CanBePrefix flag.
   * @param canBePrefix True if the Interest name can be a prefix.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setCanBePrefix(int canBePrefix)
  {
    // Use the closest v0.2 semantics. CanBePrefix is the opposite of exact
    // match where MaxSuffixComponents is 1 (for the implicit digest).
    maxSuffixComponents_ = (canBePrefix ? -1 : 1);
    didSetCanBePrefix_ = true;
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
   * Set this interest to use a copy of the given DelegationSet object as the
   * forwarding hint.
   * @note You can also call getForwardingHint and change the forwarding hint
   * directly.
   * @param forwardingHint The DelegationSet object to use as the forwarding
   * hint. This makes a copy of the object. If no forwarding hint is specified,
   * set to a new default DelegationSet() with no entries.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setForwardingHint(const DelegationSet& forwardingHint)
  {
    forwardingHint_ = forwardingHint;
    ++changeCount_;
    return *this;
  }

  /**
   * Set the application parameters to the given value.
   * @param applicationParameters The application parameters Blob.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  setApplicationParameters(const Blob& applicationParameters)
  {
    applicationParameters_ = applicationParameters;
    ++changeCount_;
    return *this;
  }

  /**
   * @deprecated Use setApplicationParameters.
   */
  Interest&
  DEPRECATED_IN_NDN_CPP setParameters(const Blob& applicationParameters)
  {
    return setApplicationParameters(applicationParameters);
  }

  /**
   * Append the digest of the application parameters to the Name as a
   * ParametersSha256DigestComponent. However, if the application parameters is
   * unspecified, do nothing. This does not check if the Name already has a
   * parameters digest component, so calling again will append another component.
   * @return This Interest so that you can chain calls to update values.
   */
  Interest&
  appendParametersDigestToName();

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
   * @deprecated Use setForwardingHint.
   */
  Interest&
  DEPRECATED_IN_NDN_CPP setLinkWireEncoding
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
   * @deprecated Use setForwardingHint.
   */
  Interest&
  DEPRECATED_IN_NDN_CPP unsetLink()
  {
    WireFormat* wireFormat = 0;
    return setLinkWireEncoding(Blob(), *wireFormat);
  }

  /**
   * Set the selected delegation index.
   * @param selectedDelegationIndex The selected delegation index. If not
   * specified, set to -1.
   * @return This Interest so that you can chain calls to update values.
   * @deprecated Use setForwardingHint.
   */
  Interest&
  DEPRECATED_IN_NDN_CPP setSelectedDelegationIndex(int selectedDelegationIndex)
  {
    selectedDelegationIndex_ = selectedDelegationIndex;
    ++changeCount_;
    return *this;
  }

  /**
   * An internal library method to set the LpPacket for an incoming packet. The
   * application should not call this.
   * @param lpPacket The LpPacket. This does not make a copy.
   * @return This Interest so that you can chain calls to update values.
   * @note This is an experimental feature. This API may change in the future.
   */
  Interest&
  setLpPacket(const ptr_lib::shared_ptr<LpPacket>& lpPacket)
  {
    lpPacket_ = lpPacket;
    // Don't update changeCount_ since this doesn't affect the wire encoding.
    return *this;
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
   * Check if the given Data packet can satisfy this Interest. This method
   * considers the Name, MinSuffixComponents, MaxSuffixComponents,
   * PublisherPublicKeyLocator, and Exclude. It does not consider the
   * ChildSelector or MustBeFresh. This uses the given wireFormat to get the
   * Data packet encoding for the full Name.
   * @param data The Data packet to check.
   * @param wireFormat A WireFormat object used to encode the Data packet to
   * get its full Name. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if the given Data packet can satisfy this Interest.
   */
  bool
  matchesData
    (const Data& data,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const;

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
    changed = forwardingHint_.checkChanged() || changed;
    changed = link_.checkChanged() || changed;
    if (changed)
      // A child object has changed, so update the change count.
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      ++const_cast<Interest*>(this)->changeCount_;

    return changeCount_;
  }

  /**
   * This internal library method gets didSetCanBePrefix_ which is set true when
   * the application calls setCanBePrefix(), or if the application had already
   * called setDefaultCanBePrefix() before creating the Interest.
   */
  bool
  getDidSetCanBePrefix_() const { return didSetCanBePrefix_; }

private:
  void
  construct()
  {
    minSuffixComponents_ = -1;
    maxSuffixComponents_ = (getDefaultCanBePrefix() ? -1 : 1);
    // didSetCanBePrefix_ is true if the app already called setDefaultCanBePrefix().
    didSetCanBePrefix_ = didSetDefaultCanBePrefix_;
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
  bool didSetCanBePrefix_;
  ChangeCounter<KeyLocator> keyLocator_;
  ChangeCounter<Exclude> exclude_;
  int childSelector_;       /**< -1 for none */
  bool mustBeFresh_;
  Milliseconds interestLifetimeMilliseconds_; /**< -1 for none */
  Blob nonce_;
  uint64_t getNonceChangeCount_;
  ChangeCounter<DelegationSet> forwardingHint_;
  Blob applicationParameters_;
  Blob linkWireEncoding_;
  WireFormat* linkWireEncodingFormat_;
  SharedPointerChangeCounter<Link> link_;
  int selectedDelegationIndex_; /**< -1 for none */
  SignedBlob defaultWireEncoding_;
  WireFormat *defaultWireEncodingFormat_;
  uint64_t getDefaultWireEncodingChangeCount_;
  ptr_lib::shared_ptr<LpPacket> lpPacket_;
  uint64_t changeCount_;
  static bool didSetDefaultCanBePrefix_;
};

}

#endif
