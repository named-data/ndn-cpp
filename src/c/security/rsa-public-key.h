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

#ifndef NDN_RSA_PUBLIC_KEY_H
#define NDN_RSA_PUBLIC_KEY_H

#include <ndn-cpp/c/common.h>
#include <ndn-cpp/c/errors.h>
#include <ndn-cpp/c/security/rsa-public-key-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the ndn_RsaPublicKey struct with a null value.
 * @param self A pointer to the ndn_RsaPublicKey struct.
 */
static __inline void
ndn_RsaPublicKey_initialize(struct ndn_RsaPublicKey *self)
{
  self->publicKey = 0;
}

/**
 * Finalize the ndn_RsaPublicKey struct, freeing memory if needed.
 * @param self A pointer to the ndn_RsaPrivateKey struct.
 */
void
ndn_RsaPublicKey_finalize(struct ndn_RsaPublicKey *self);

/**
 * Decode the publicKeyDer and set the ndn_RsaPublicKey struct, allocating
 * memory as needed. You must call ndn_RsaPublicKey_finalize to free it.
 * @param self A pointer to the ndn_RsaPublicKey struct.
 * @param publicKeyDer A pointer to the DER-encoded public key.
 * @param publicKeyDerLength The length of publicKeyDer.
 * @return 0 for success, else NDN_ERROR_Error_decoding_key if publicKeyDer
 * can't be decoded as an RSA public key.
 */
ndn_Error
ndn_RsaPublicKey_decode
  (struct ndn_RsaPublicKey *self, const uint8_t *publicKeyDer,
   size_t publicKeyDerLength);

/**
 * Use the public key to verify the data using RsaWithSha256.
 * @param self A pointer to the ndn_RsaPublicKey struct.
 * @param signature A pointer to the signature bytes.
 * @param signatureLength The length of signature.
 * @param data A pointer to the input byte array to verify.
 * @param dataLength The length of data.
 * @return Nonzero if the signature verifies, 0 if not.
 */
int
ndn_RsaPublicKey_verifyWithSha256
  (const struct ndn_RsaPublicKey *self, const uint8_t *signature,
   size_t signatureLength, const uint8_t *data, size_t dataLength);

#ifdef __cplusplus
}
#endif

#endif
