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

#ifndef NDN_EC_PRIVATE_KEY_H
#define NDN_EC_PRIVATE_KEY_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/errors.h>
#include <ndn-cpp/c/util/ec-private-key-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the ndn_EcPrivateKey struct with a null value.
 * @param self A pointer to the ndn_EcPrivateKey struct.
 */
static __inline void
ndn_EcPrivateKey_initialize(struct ndn_EcPrivateKey *self)
{
  self->privateKey = 0;
}

/**
 * Finalize the ndn_EcPrivateKey struct, freeing memory if needed.
 * @param self A pointer to the ndn_EcPrivateKey struct.
 */
void
ndn_EcPrivateKey_finalize(struct ndn_EcPrivateKey *self);

/**
 * Decode the privateKeyDer and set the ndn_EcPrivateKey struct, allocating
 * memory as needed. You must call ndn_EcPrivateKey_finalize to free it.
 * @param self A pointer to the ndn_EcPrivateKey struct.
 * @param privateKeyDer A pointer to the DER-encoded private key.
 * @param privateKeyDerLength The length of privateKeyDer.
 * @return 0 for success, else NDN_ERROR_Error_decoding_key if privateKeyDer
 * can't be decoded as an EC private key.
 */
ndn_Error
ndn_EcPrivateKey_decode
  (struct ndn_EcPrivateKey *self, const uint8_t *privateKeyDer,
   size_t privateKeyDerLength);

/**
 * Set the the private key from the given curveId, using the value to create a
 * BIGNUM, allocating memory as needed. You must call ndn_EcPrivateKey_finalize
 * to free it.
 * @param self A pointer to the ndn_EcPrivateKey struct.
 * @param curveId The OpenSSL curve ID such as NID_secp384r1.
 * @param value A pointer to the value array for the BIGNUM.
 * @param valueLength The length of value.
 * @return 0 for success, else NDN_ERROR_Error_decoding_key if can't create
 * the private key from the curveId or value.
 */
ndn_Error
ndn_EcPrivateKey_setByCurve
  (struct ndn_EcPrivateKey *self, int curveId, const uint8_t *value,
   size_t valueLength);

/**
 * Use the private key to sign the data using EcdsaWithSha256.
 * @param self A pointer to the ndn_EcPrivateKey struct.
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
ndn_EcPrivateKey_signWithSha256
  (const struct ndn_EcPrivateKey *self, const uint8_t *data, size_t dataLength,
   const uint8_t *signature, size_t *signatureLength);

#ifdef __cplusplus
}
#endif

#endif
