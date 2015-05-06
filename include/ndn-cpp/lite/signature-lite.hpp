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

#include "key-locator-lite.hpp"
#include "../c/data-types.h"

namespace ndn {

/**
 * A SignatureLite holds a signature type, a KeyLocatorLite, the signature
 * bytes and other fields to represent the Signature block of a Data packet.
 * This has the union of fields needed to represent specific types of signature
 * such as Sha256WithRsaSignature and DigestSha256Signature.
 */
class SignatureLite : private ndn_Signature {
public:
  /**
   * Create a SignatureLite with values for none and the default digestAlgorithm.
   * @param keyNameComponents The pre-allocated array of ndn_NameComponent for
   * the keyLocatorLite. Instead of an array of NameLite::Component, this is an
   * array of the underlying ndn_NameComponent struct so that it doesn't run the
   * default constructor unnecessarily.
   * @param maxKeyNameComponents The number of elements in the allocated
   * keyNameComponents array.
   */
  SignatureLite(ndn_NameComponent* keyNameComponents, size_t maxKeyNameComponents);

  /**
   * Set the fields the values for none as in the constructor.
   */
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

  const KeyLocatorLite&
  getKeyLocator() const { return KeyLocatorLite::upCast(keyLocator); }

  KeyLocatorLite&
  getKeyLocator() { return KeyLocatorLite::upCast(keyLocator); }

  void
  setType(ndn_SignatureType type) { this->type = type; }

  /**
   * Set the bytes of the digest algorithm.
   * @param digestAlgorithm The bytes of the digest algorithm. This copies a
   * pointer to the bytes, but does not copy the bytes.
   */
  void
  setDigestAlgorithm(const BlobLite& digestAlgorithm)
  {
    BlobLite::upCast(this->digestAlgorithm) = digestAlgorithm;
  }

  /**
   * Set the witness.
   * @param witness The witness. This copies a pointer to the bytes, but does
   * not copy the bytes.
   */
  void
  setWitness(const BlobLite& witness)
  {
    BlobLite::upCast(this->witness) = witness;
  }

  /**
   * Set the signature bytes.
   * @param signature The signature bytes. This copies a pointer to the bytes,
   * but does not copy the bytes.
   */
  void
  setSignature(const BlobLite& signature)
  {
    BlobLite::upCast(this->signature) = signature;
  }

  /**
   * Set this signature to have the values from the other signature.
   * @param other The other SignatureLite to get values from.
   * @return 0 for success, or an error code if there is not enough room in this
   * object's key locator keyName components array.
   */
  ndn_Error
  set(const SignatureLite& other);

  /**
   * Upcast the reference to the ndn_Signature struct to a SignatureLite.
   * @param signature A reference to the ndn_Signature struct.
   * @return The same reference as SignatureLite.
   */
  static SignatureLite&
  upCast(ndn_Signature& signature) { return *(SignatureLite*)&signature; }

  static const SignatureLite&
  upCast(const ndn_Signature& signature) { return *(SignatureLite*)&signature; }

private:
  /**
   * Don't allow the copy constructor. Instead use set(const SignatureLite&)
   * which can return an error if there is no more room in the name components
   * array.
   */
  SignatureLite(SignatureLite& other);
  SignatureLite(const SignatureLite& other);

  /**
   * Don't allow the assignment operator. Instead use set(const SignatureLite&)
   * which can return an error if there is no more room in the name components
   * array.
   */
  SignatureLite& operator=(const SignatureLite& other);
};

}

#endif
