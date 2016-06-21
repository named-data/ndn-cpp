/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016 Regents of the University of California.
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

#ifndef NDN_EC_PUBLIC_KEY_LITE_HPP
#define NDN_EC_PUBLIC_KEY_LITE_HPP

#include "../util/blob-lite.hpp"
#include "../../c/security/ec-public-key-types.h"

namespace ndn {

/**
 * An EcPublicKeyLite holds a decoded EC public key for use in crypto operations.
 */
class EcPublicKeyLite : private ndn_EcPublicKey {
public:
  /**
   * Create an EcPublicKeyLite with a null value.
   */
  EcPublicKeyLite();

  /**
   * Finalize the EcPublicKeyLite, freeing memory if needed.
   */
  ~EcPublicKeyLite();

  /**
   * Decode the publicKeyDer and set this EcPublicKeyLite, allocating
   * memory as needed.
   * @param publicKeyDer A pointer to the DER-encoded public key.
   * @param publicKeyDerLength The length of publicKeyDer.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if publicKeyDer
   * can't be decoded as an EC public key.
   */
  ndn_Error
  decode(const uint8_t* publicKeyDer, size_t publicKeyDerLength);

  /**
   * Decode the publicKeyDer and set this EcPublicKeyLite, allocating
   * memory as needed.
   * @param publicKeyDer The DER-encoded public key.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if publicKeyDer
   * can't be decoded as an EC public key.
   */
  ndn_Error
  decode(const BlobLite& publicKeyDer)
  {
    return decode(publicKeyDer.buf(), publicKeyDer.size());
  }

  /**
   * Encode the DER-encoded EC SubjectPublicKeyInfo.
   * @param includeParameters If true, then include the EC parameters in the
   * encoding.
   * @param encoding A pointer to the encoding output buffer. If this is null
   * then only set encodingLength (which can be used to allocate a buffer of the
   * correct size). Otherwise, the caller must provide a buffer large enough to
   * receive the encoding bytes.
   * @param encodingLength Set encodingLength to the number of bytes in the
   * encoding.
   * @return 0 for success, else NDN_ERROR_Error_encoding_key if can't encode the
   * key.
   */
  ndn_Error
  encode(bool includeParameters, uint8_t* encoding, size_t& encodingLength) const;

  /**
   * Use this public key to verify the data using EcdsaWithSha256.
   * @param signature A pointer to the signature bytes.
   * @param signatureLength The length of signature.
   * @param data A pointer to the input byte array to verify.
   * @param dataLength The length of data.
   * @return True if the signature verifies, false if not.
   */
  bool
  verifyWithSha256
    (const uint8_t *signature, size_t signatureLength, const uint8_t *data,
     size_t dataLength) const;

  /**
   * Downcast the reference to the ndn_EcPublicKey struct to a EcPublicKeyLite.
   * @param blob A reference to the ndn_EcPublicKey struct.
   * @return The same reference as EcPublicKeyLite.
   */
  static EcPublicKeyLite&
  downCast(ndn_EcPublicKey& blob) { return *(EcPublicKeyLite*)&blob; }

  static const EcPublicKeyLite&
  downCast(const ndn_EcPublicKey& blob) { return *(EcPublicKeyLite*)&blob; }

public:
  // Don't allow copying since we don't reference count the allocated value.
  EcPublicKeyLite(const EcPublicKeyLite& other);
  EcPublicKeyLite& operator=(const EcPublicKeyLite& other);
};

}

#endif
