/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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

#include "../../c/util/crypto.h"
#include "../../c/util/ndn_memory.h"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/security/ec-public-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/policy/policy-manager.hpp>

using namespace std;

namespace ndn {

bool
PolicyManager::verifySignature
  (const Signature* signature, const SignedBlob& signedBlob,
   const Blob& publicKeyDer)
{
  ndn_Error error;
  bool verified;

  if (dynamic_cast<const DigestSha256Signature *>(signature))
    return CryptoLite::verifyDigestSha256Signature
      (signature->getSignature(), signedBlob.getSignedPortionBlobLite());
#if NDN_CPP_HAVE_LIBCRYPTO
  else if (dynamic_cast<const Sha256WithRsaSignature *>(signature)) {
    if (publicKeyDer.isNull())
      return false;
    if ((error = RsaPublicKeyLite::verifySha256WithRsaSignature
         (signature->getSignature(), signedBlob.getSignedPortionBlobLite(),
          publicKeyDer, verified)) != 0) {
      if (error == NDN_ERROR_Error_decoding_key)
        throw UnrecognizedKeyFormatException("Error decoding public key");
      else
        throw SecurityException(ndn_getErrorString(error));
    }
    return verified;
  }
  else if (dynamic_cast<const Sha256WithEcdsaSignature *>(signature)) {
    if (publicKeyDer.isNull())
      return false;
    if ((error = EcPublicKeyLite::verifySha256WithEcdsaSignature
         (signature->getSignature(), signedBlob.getSignedPortionBlobLite(),
          publicKeyDer, verified)) != 0) {
      if (error == NDN_ERROR_Error_decoding_key)
        throw UnrecognizedKeyFormatException("Error decoding public key");
      else
        throw SecurityException(ndn_getErrorString(error));
    }
    return verified;
  }
  else
#endif
    throw SecurityException("PolicyManager::verify: Signature type is unknown");
}

}
