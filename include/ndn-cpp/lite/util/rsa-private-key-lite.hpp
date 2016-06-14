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

#ifndef NDN_RSA_PRIVATE_KEY_LITE_HPP
#define NDN_RSA_PRIVATE_KEY_LITE_HPP

#include "blob-lite.hpp"
#include "../../c/util/rsa-private-key-types.h"

namespace ndn {

class RsaPrivateKeyLite : private ndn_RsaPrivateKey {
public:
  /**
   * Create an RsaPrivateKeyLite with a null value.
   */
  RsaPrivateKeyLite();

  /**
   * Finalize the RsaPrivateKeyLite, freeing memory if needed.
   */
  ~RsaPrivateKeyLite();

  /**
   * Decode the privateKeyDer and set this RsaPrivateKeyLite, allocating
   * memory as needed.
   * @param privateKeyDer A pointer to the DER-encoded private key.
   * @param privateKeyDerLength The length of privateKeyDer.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if privateKeyDer
   * can't be decoded as an RSA private key.
   */
  ndn_Error
  decode(const uint8_t* privateKeyDer, size_t privateKeyDerLength);

  /**
   * Decode the privateKeyDer and set this RsaPrivateKeyLite, allocating
   * memory as needed.
   * @param privateKeyDer The DER-encoded private key.
   * @return 0 for success, else NDN_ERROR_Error_decoding_key if privateKeyDer
   * can't be decoded as an RSA private key.
   */
  ndn_Error
  decode(const BlobLite& privateKeyDer)
  {
    return decode(privateKeyDer.buf(), privateKeyDer.size());
  }

  /**
   * Use this private key to sign the data using RsaWithSha256.
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
   * Use this private key to sign the data using RsaWithSha256.
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
   * Downcast the reference to the ndn_RsaPrivateKey struct to a RsaPrivateKeyLite.
   * @param blob A reference to the ndn_RsaPrivateKey struct.
   * @return The same reference as RsaPrivateKeyLite.
   */
  static RsaPrivateKeyLite&
  downCast(ndn_RsaPrivateKey& blob) { return *(RsaPrivateKeyLite*)&blob; }

  static const RsaPrivateKeyLite&
  downCast(const ndn_RsaPrivateKey& blob) { return *(RsaPrivateKeyLite*)&blob; }
};

}

#endif
