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

#ifndef NDN_EC_PRIVATE_KEY_LITE_HPP
#define NDN_EC_PRIVATE_KEY_LITE_HPP

#include "blob-lite.hpp"
#include "../../c/util/ec-private-key-types.h"

namespace ndn {

class EcPrivateKeyLite : private ndn_EcPrivateKey {
public:
  /**
   * Create an EcPrivateKeyLite with a null value.
   */
  EcPrivateKeyLite();

  /**
   * Finalize the EcPrivateKeyLite, freeing memory if needed.
   */
  ~EcPrivateKeyLite();

  /**
   * Decode the privateKeyDer and set this EcPrivateKeyLite, allocating
   * memory as needed.
   * @param privateKeyDer A pointer to the DER-encoded private key.
   * @param privateKeyDerLength The length of privateKeyDer.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if privateKeyDer
   * can't be decoded as an EC private key.
   */
  ndn_Error
  decode(const uint8_t* privateKeyDer, size_t privateKeyDerLength);

  /**
   * Decode the privateKeyDer and set this EcPrivateKeyLite, allocating
   * memory as needed.
   * @param privateKeyDer The DER-encoded private key.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if privateKeyDer
   * can't be decoded as an EC private key.
   */
  ndn_Error
  decode(const BlobLite& privateKeyDer)
  {
    return decode(privateKeyDer.buf(), privateKeyDer.size());
  }

  /**
   * Set the the private key from the given curveId, using the value to create a
   * BIGNUM, allocating memory as needed.
   * @param curveId The OpenSSL curve ID such as NID_secp384r1.
   * @param value A pointer to the value array for the BIGNUM.
   * @param valueLength The length of value.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if can't create
   * the private key from the curveId or value.
   */
  ndn_Error
  setByCurve(int curveId, const uint8_t* value, size_t valueLength);

  /**
   * Set the the private key from the given curveId, using the value to create a
   * BIGNUM, allocating memory as needed.
   * @param curveId The OpenSSL curve ID such as NID_secp384r1.
   * @param value The value array for the BIGNUM.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if can't create
   * the private key from the curveId or value.
   */
  ndn_Error
  setByCurve(int curveId, const BlobLite& value)
  {
    return setByCurve(curveId, value.buf(), value.size());
  }

  /**
   * Use this private key to sign the data using EcdsaWithSha256.
   * @param data A pointer to the input byte array to sign.
   * @param dataLength The length of data.
   * @param signature A pointer to the signature output buffer. The caller must
   * provide a buffer large enough to receive the signature bytes.
   * @param signatureLength Set signatureLength to the number of bytes place in
   * the signature buffer.
   * @return 0 for success, else NDN_ERROR_Error_in_sign_operation if can't
   * complete the sign operation.
   */
  ndn_Error
  signWithSha256
    (const uint8_t* data, size_t dataLength, const uint8_t* signature,
     size_t& signatureLength) const;

  /**
   * Use this private key to sign the data using EcdsaWithSha256.
   * @param data The input byte array to sign.
   * @param signature A pointer to the signature output buffer. The caller must
   * provide a buffer large enough to receive the signature bytes.
   * @param signatureLength Set signatureLength to the number of bytes place in
   * the signature buffer.
   * @return 0 for success, else NDN_ERROR_Error_in_sign_operation if can't
   * complete the sign operation.
   */
  ndn_Error
  signWithSha256
    (const BlobLite& data, const uint8_t* signature, size_t& signatureLength) const
  {
    return signWithSha256(data.buf(), data.size(), signature, signatureLength);
  }

  /**
   * Downcast the reference to the ndn_EcPrivateKey struct to a EcPrivateKeyLite.
   * @param blob A reference to the ndn_EcPrivateKey struct.
   * @return The same reference as EcPrivateKeyLite.
   */
  static EcPrivateKeyLite&
  downCast(ndn_EcPrivateKey& blob) { return *(EcPrivateKeyLite*)&blob; }

  static const EcPrivateKeyLite&
  downCast(const ndn_EcPrivateKey& blob) { return *(EcPrivateKeyLite*)&blob; }
};

}

#endif
