/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#include <openssl/ssl.h>
#include "../../c/util/crypto.h"
#include "../../c/util/ndn_memory.h"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/security/policy/policy-manager.hpp>

using namespace std;

namespace ndn {

bool
PolicyManager::verifySignature
  (const Signature* signature, const SignedBlob& signedBlob,
   const Blob& publicKeyDer)
{
  if (dynamic_cast<const Sha256WithRsaSignature *>(signature))
    return verifySha256WithRsaSignature
      (signature->getSignature(), signedBlob, publicKeyDer);
  else if (dynamic_cast<const Sha256WithEcdsaSignature *>(signature))
    return verifySha256WithEcdsaSignature
      (signature->getSignature(), signedBlob, publicKeyDer);
  else if (dynamic_cast<const DigestSha256Signature *>(signature))
    return verifyDigestSha256Signature(signature->getSignature(), signedBlob);
  else
    throw SecurityException("PolicyManager::verify: Signature type is unknown");
}

bool
PolicyManager::verifySha256WithEcdsaSignature
  (const Blob& signature, const SignedBlob& signedBlob, const Blob& publicKeyDer)
{
  // Set signedPortionDigest to the digest of the signed portion of the signedBlob.
  uint8_t signedPortionDigest[SHA256_DIGEST_LENGTH];
  ndn_digestSha256
    (signedBlob.signedBuf(), signedBlob.signedSize(), signedPortionDigest);

  // Verify the signedPortionDigest.
  // Use a temporary pointer since d2i updates it.
  const uint8_t *derPointer = publicKeyDer.buf();
  EC_KEY *ecPublicKey = d2i_EC_PUBKEY(NULL, &derPointer, publicKeyDer.size());
  if (!ecPublicKey)
    throw UnrecognizedKeyFormatException
      ("Error decoding public key in d2i_EC_PUBKEY");
  int success = ECDSA_verify
    (NID_sha256, signedPortionDigest, sizeof(signedPortionDigest),
     (uint8_t *)signature.buf(),signature.size(), ecPublicKey);
  // Free the public key before checking for success.
  EC_KEY_free(ecPublicKey);

  // ECDSA_verify returns 1 for a valid signature.
  return (success == 1);
}

bool
PolicyManager::verifySha256WithRsaSignature
  (const Blob& signature, const SignedBlob& signedBlob, const Blob& publicKeyDer)
{
  // Set signedPortionDigest to the digest of the signed portion of the signedBlob.
  uint8_t signedPortionDigest[SHA256_DIGEST_LENGTH];
  ndn_digestSha256
    (signedBlob.signedBuf(), signedBlob.signedSize(), signedPortionDigest);

  // Verify the signedPortionDigest.
  // Use a temporary pointer since d2i updates it.
  const uint8_t *derPointer = publicKeyDer.buf();
  RSA *rsaPublicKey = d2i_RSA_PUBKEY(NULL, &derPointer, publicKeyDer.size());
  if (!rsaPublicKey)
    throw UnrecognizedKeyFormatException("Error decoding public key in d2i_RSAPublicKey");
  int success = RSA_verify
    (NID_sha256, signedPortionDigest, sizeof(signedPortionDigest),
     (uint8_t *)signature.buf(), signature.size(), rsaPublicKey);
  // Free the public key before checking for success.
  RSA_free(rsaPublicKey);

  // RSA_verify returns 1 for a valid signature.
  return (success == 1);
}

bool
PolicyManager::verifyDigestSha256Signature
  (const Blob& signature, const SignedBlob& signedBlob)
{
  // Set signedPortionDigest to the digest of the signed portion of the signedBlob.
  uint8_t signedPortionDigest[SHA256_DIGEST_LENGTH];
  ndn_digestSha256
    (signedBlob.signedBuf(), signedBlob.signedSize(), signedPortionDigest);

  return signature.size() == sizeof(signedPortionDigest) && ndn_memcmp
    (signature.buf(), signedPortionDigest, sizeof(signedPortionDigest)) == 0;
}

}
