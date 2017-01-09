/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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

#include <ndn-cpp/security//security-exception.hpp>
#include "../../c/util/crypto.h"
#include "../../encoding/der/der-node.hpp"
#include "../../encoding/der/der-exception.hpp"
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/security/ec-public-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/certificate/public-key.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;

static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";
static const char *EC_ENCRYPTION_OID = "1.2.840.10045.2.1";

PublicKey::PublicKey(const Blob& keyDer)
{
  keyDer_ = keyDer;

  // Get the public key OID.
  string oidString;
  try {
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (keyDer.buf(), 0);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& rootChildren =
      parsedNode->getChildren();
    const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
      DerNode::getSequence(rootChildren, 0).getChildren();
    oidString = algorithmIdChildren[0]->toVal().toRawStr();
  }
  catch (DerDecodingException& ex) {
    throw UnrecognizedKeyFormatException
      (string("PublicKey: Error decoding the public key: ") + ex.what());
  }

  // Verify that the we can decode.
  // Use a temporary pointer since d2i updates it.
#if NDN_CPP_HAVE_LIBCRYPTO
  if (oidString == RSA_ENCRYPTION_OID) {
    keyType_ = KEY_TYPE_RSA;

    RsaPublicKeyLite publicKey;
    if (publicKey.decode(keyDer) != NDN_ERROR_success)
      throw UnrecognizedKeyFormatException("Error decoding RSA public key DER");
  }
  else if (oidString == EC_ENCRYPTION_OID) {
    keyType_ = KEY_TYPE_ECDSA;

    EcPublicKeyLite publicKey;
    if (publicKey.decode(keyDer) != NDN_ERROR_success)
      throw UnrecognizedKeyFormatException("Error decoding EC public key DER");
  }
  else
#endif
    throw UnrecognizedKeyFormatException("PublicKey: Unrecognized OID");
}

ptr_lib::shared_ptr<DerNode>
PublicKey::toDer()
{
  return DerNode::parse(keyDer_.buf());
}

Blob
PublicKey::getDigest(DigestAlgorithm digestAlgorithm) const
{
  if (digestAlgorithm == DIGEST_ALGORITHM_SHA256) {
    uint8_t digest[ndn_SHA256_DIGEST_SIZE];
    CryptoLite::digestSha256(keyDer_, digest);

    return Blob(digest, sizeof(digest));
  }
  else
    throw UnrecognizedDigestAlgorithmException("Wrong format!");
}

}
