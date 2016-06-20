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

#include "../util/crypto.h"
#include "ec-public-key.h"

#if NDN_CPP_HAVE_LIBCRYPTO

#include <openssl/ssl.h>

void
ndn_EcPublicKey_finalize(struct ndn_EcPublicKey *self)
{
  if (self->publicKey) {
    EC_KEY_free(self->publicKey);
    self->publicKey = 0;
  }
}

ndn_Error
ndn_EcPublicKey_decode
  (struct ndn_EcPublicKey *self, const uint8_t *publicKeyDer,
   size_t publicKeyDerLength)
{
  if (self->publicKey)
    // Free a previous value.
    EC_KEY_free(self->publicKey);

  self->publicKey = d2i_EC_PUBKEY(NULL, &publicKeyDer, publicKeyDerLength);
  if (!self->publicKey)
    return NDN_ERROR_Error_decoding_key;
  return NDN_ERROR_success;
}

int
ndn_EcPublicKey_verifyWithSha256
  (const struct ndn_EcPublicKey *self, const uint8_t *signature,
   size_t signatureLength, const uint8_t *data, size_t dataLength)
{
  // Set digest to the digest of the signed portion of the signedBlob.
  uint8_t digest[ndn_SHA256_DIGEST_SIZE];
  ndn_digestSha256(data, dataLength, digest);
  return ECDSA_verify
    (NID_sha256, digest, sizeof(digest), (uint8_t *)signature, signatureLength,
     self->publicKey) == 1;
}

#endif // NDN_CPP_HAVE_LIBCRYPTO
