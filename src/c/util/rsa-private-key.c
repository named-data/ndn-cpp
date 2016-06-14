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

#include "crypto.h"
#include "rsa-private-key.h"

#if NDN_CPP_HAVE_LIBCRYPTO

#include <openssl/ssl.h>

void
ndn_RsaPrivateKey_finalize(struct ndn_RsaPrivateKey *self)
{
  if (self->privateKey) {
    RSA_free(self->privateKey);
    self->privateKey = 0;
  }
}

ndn_Error
ndn_RsaPrivateKey_decode
  (struct ndn_RsaPrivateKey *self, const uint8_t *privateKeyDer,
   size_t privateKeyDerLength)
{
  if (self->privateKey)
    // Free a previous value.
    RSA_free(self->privateKey);

  self->privateKey = d2i_RSAPrivateKey(NULL, &privateKeyDer, privateKeyDerLength);
  if (!self->privateKey)
    return NDN_ERROR_Error_decoding_key;
  return NDN_ERROR_success;
}

ndn_Error
ndn_RsaPrivateKey_signWithSha256
  (const struct ndn_RsaPrivateKey *self, const uint8_t *data, size_t dataLength,
   const uint8_t *signature, size_t *signatureLength)
{
  uint8_t digest[ndn_SHA256_DIGEST_SIZE];
  ndn_digestSha256(data, dataLength, digest);

  // Make a temporary length variable of the correct type.
  unsigned int tempSignatureLength;
  if (!RSA_sign(NID_sha256, digest, sizeof(digest), (unsigned char *)signature,
                &tempSignatureLength, self->privateKey))
    return NDN_ERROR_Error_in_sign_operation;

  *signatureLength = tempSignatureLength;
  return NDN_ERROR_success;
}

#endif // NDN_CPP_HAVE_LIBCRYPTO
