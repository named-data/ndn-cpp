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

#ifndef NDN_SHA256_WITH_RSA_SIGNATURE_HPP
#define NDN_SHA256_WITH_RSA_SIGNATURE_HPP

#include "signature.hpp"
#include "key-locator.hpp"
#include "publisher-public-key-digest.hpp"
#include "util/change-counter.hpp"

namespace ndn {

/**
 * A Sha256WithRsaSignature extends Signature and holds the signature bits and other info representing a
 * SHA256-with-RSA signature in a data packet.
 */
class Sha256WithRsaSignature : public Signature {
public:
  Sha256WithRsaSignature()
  : changeCount_(0)
  {
  }

  /**
   * Return a pointer to a new Sha256WithRsaSignature which is a copy of this signature.
   */
  virtual ptr_lib::shared_ptr<Signature>
  clone() const;

  /**
   * Set the signatureStruct to point to the values in this signature object, without copying any memory.
   * WARNING: The resulting pointers in signatureStruct are invalid after a further use of this object which could reallocate memory.
   * @param signatureStruct a C ndn_Signature struct where the name components array is already allocated.
   */
  virtual void
  get(struct ndn_Signature& signatureStruct) const;

  /**
   * Clear this signature, and set the values by copying from the ndn_Signature struct.
   * @param signatureStruct a C ndn_Signature struct
   */
  virtual void
  set(const struct ndn_Signature& signatureStruct);

  /**
   * @deprecated digestAlgorithm is deprecated.
   */
  const Blob&
  DEPRECATED_IN_NDN_CPP getDigestAlgorithm() const;

  /**
   * @deprecated Witness is deprecated.
   */
  const Blob&
  DEPRECATED_IN_NDN_CPP getWitness() const;

  /**
   * Get the signature bytes.
   * @return The signature bytes. If not specified, the value isNull().
   */
  virtual const Blob&
  getSignature() const;

  /**
   * @deprecated.  The Signature publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest,
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest.
   */
  const PublisherPublicKeyDigest&
  DEPRECATED_IN_NDN_CPP getPublisherPublicKeyDigest() const { return publisherPublicKeyDigest_.get(); }

  /**
   * @deprecated.  The Signature publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest,
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest.
   */
  PublisherPublicKeyDigest&
  DEPRECATED_IN_NDN_CPP getPublisherPublicKeyDigest() { return publisherPublicKeyDigest_.get(); }

  const KeyLocator&
  getKeyLocator() const { return keyLocator_.get(); }

  KeyLocator&
  getKeyLocator() { return keyLocator_.get(); }

  /**
   * @deprecated digestAlgorithm is deprecated.
   */
  void
  DEPRECATED_IN_NDN_CPP setDigestAlgorithm(const Blob& digestAlgorithm);

  /**
   * @deprecated Witness is deprecated.
   */
  void
  DEPRECATED_IN_NDN_CPP setWitness(const Blob& witness);

  /**
   * Set the signature bytes to the given value.
   * @param signature A Blob with the signature bytes.
   */
  virtual void
  setSignature(const Blob& signature);

  /**
   * @deprecated.  The Signature publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest,
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest.
   */
  void
  DEPRECATED_IN_NDN_CPP setPublisherPublicKeyDigest(const PublisherPublicKeyDigest& publisherPublicKeyDigest)
  {
    publisherPublicKeyDigest_.set(publisherPublicKeyDigest);
    ++changeCount_;
  }

  void
  setKeyLocator(const KeyLocator& keyLocator)
  {
    keyLocator_.set(keyLocator);
    ++changeCount_;
  }

  /**
   * Clear all the fields.
   */
  void
  clear()
  {
    digestAlgorithm_.reset();
    witness_.reset();
    signature_.reset();
    publisherPublicKeyDigest_.get().clear();
    keyLocator_.get().clear();
    ++changeCount_;
  }

  /**
   * Get the change count, which is incremented each time this object (or a child object) is changed.
   * @return The change count.
   */
  virtual uint64_t
  getChangeCount() const
  {
    // Make sure each of the checkChanged is called.
    bool changed = publisherPublicKeyDigest_.checkChanged();
    changed = keyLocator_.checkChanged() || changed;
    if (changed)
      // A child object has changed, so update the change count.
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      ++const_cast<Sha256WithRsaSignature*>(this)->changeCount_;

    return changeCount_;
  }

private:
  Blob digestAlgorithm_; /**< if empty, the default is 2.16.840.1.101.3.4.2.1 (sha-256) */
  /** @deprecated Witness is deprecated. */
  Blob witness_;
  Blob signature_;
  /** @deprecated.  The Signature publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest,
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest. */
  ChangeCounter<PublisherPublicKeyDigest> publisherPublicKeyDigest_;
  ChangeCounter<KeyLocator> keyLocator_;
  uint64_t changeCount_;
};

}

#endif
