/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_PUBLIC_KEY_HPP
#define NDN_PUBLIC_KEY_HPP

#include "../../util/blob.hpp"
#include "../../encoding/oid.hpp"
#include "../security-common.hpp"

namespace ndn {

class DerNode;

class PublicKey {
public:
  /**
   * The default constructor.
   */
  PublicKey()
  : keyType_((KeyType)-1)
  {}

  /**
   * Create a new PublicKey by decoding the keyDer. Set the key type from the
   * decoding.
   * @param keyDer The blob of the SubjectPublicKeyInfo DER.
   * @throws UnrecognizedKeyFormatException if can't decode the key DER.
   */
  PublicKey(const Blob& keyDer);

  /**
   * Encode the public key into DER.
   * @return the encoded DER syntax tree.
   */
  ptr_lib::shared_ptr<DerNode>
  toDer();

  KeyType getKeyType() const { return keyType_; }

  /*
   * Get the digest of the public key.
   * @param digestAlgorithm The digest algorithm. If omitted, use DIGEST_ALGORITHM_SHA256 by default.
   */
  Blob
  getDigest(DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256) const;

  /*
   * Get the raw bytes of the public key in DER format.
   */
  const Blob&
  getKeyDer() const { return keyDer_; }

private:
  KeyType keyType_;
  Blob keyDer_;   /**< SubjectPublicKeyInfo in DER */
};

}

#endif
