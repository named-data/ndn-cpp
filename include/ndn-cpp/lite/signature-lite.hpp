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

#ifndef NDN_SIGNATURE_LITE_HPP
#define NDN_SIGNATURE_LITE_HPP

#include "publisher-public-key-digest-lite.hpp"
#include "key-locator-lite.hpp"
#include "../c/data-types.h"

namespace ndn {

class SignatureLite : private ndn_Signature {
public:
  SignatureLite(ndn_NameComponent* keyNameComponents, size_t maxKeyNameComponents);

  void
  clear();

  ndn_SignatureType
  getType() const { return type; }

  const BlobLite&
  getDigestAlgorithm() const { return BlobLite::upCast(digestAlgorithm); }

  const BlobLite&
  getWitness() const { return BlobLite::upCast(witness); }

  const BlobLite&
  getSignature() const { return BlobLite::upCast(signature); }

  const PublisherPublicKeyDigestLite&
  getPublisherPublicKeyDigest() const { return PublisherPublicKeyDigestLite::upCast(publisherPublicKeyDigest); }

  PublisherPublicKeyDigestLite&
  getPublisherPublicKeyDigest() { return PublisherPublicKeyDigestLite::upCast(publisherPublicKeyDigest); }

  const KeyLocatorLite&
  getKeyLocator() const { return KeyLocatorLite::upCast(keyLocator); }

  KeyLocatorLite&
  getKeyLocator() { return KeyLocatorLite::upCast(keyLocator); }

  void
  setType(ndn_SignatureType type) { this->type = type; }

  void
  setDigestAlgorithm(const BlobLite& digestAlgorithm)
  {
    BlobLite::upCast(this->digestAlgorithm) = digestAlgorithm;
  }

  void
  setWitness(const BlobLite& witness)
  {
    BlobLite::upCast(this->witness) = witness;
  }

  void
  setSignature(const BlobLite& signature)
  {
    BlobLite::upCast(this->signature) = signature;
  }

  /**
   * Upcast the reference to the ndn_Signature struct to a SignatureLite.
   * @param signature A reference to the ndn_Signature struct.
   * @return The same reference as SignatureLite.
   */
  static SignatureLite&
  upCast(ndn_Signature& signature) { return *(SignatureLite*)&signature; }

  static const SignatureLite&
  upCast(const ndn_Signature& signature) { return *(SignatureLite*)&signature; }
};

}

#endif
