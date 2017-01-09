/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include "../../encoding/der/der-node.hpp"
#include "../../c/util/crypto.h"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/private-key-storage.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;

PrivateKeyStorage::~PrivateKeyStorage() {}

Blob
PrivateKeyStorage::encodePkcs8PrivateKey
  (const vector<uint8_t>& privateKeyDer, const OID& oid,
   const ptr_lib::shared_ptr<DerNode>& parameters)
{
  ptr_lib::shared_ptr<DerSequence> algorithmIdentifier(new DerSequence());
  algorithmIdentifier->addChild(ptr_lib::make_shared<DerNode::DerOid>(oid));
  algorithmIdentifier->addChild(parameters);

  DerSequence result;
  result.addChild(ptr_lib::make_shared<DerNode::DerInteger>(0));
  result.addChild(algorithmIdentifier);
  result.addChild(ptr_lib::make_shared<DerNode::DerOctetString>
    (&privateKeyDer[0], privateKeyDer.size()));

  return result.encode();
}

Blob
PrivateKeyStorage::encodeSubjectPublicKeyInfo
  (const OID& oid, const ptr_lib::shared_ptr<DerNode>& parameters,
   const ptr_lib::shared_ptr<DerNode>& bitString)
{
  ptr_lib::shared_ptr<DerSequence> algorithmIdentifier(new DerSequence());
  algorithmIdentifier->addChild(ptr_lib::make_shared<DerNode::DerOid>(oid));
  algorithmIdentifier->addChild(parameters);

  DerSequence result;
  result.addChild(algorithmIdentifier);
  result.addChild(bitString);

  return result.encode();
}

#if NDN_CPP_HAVE_LIBCRYPTO
void
PrivateKeyStorage::decodeEcPrivateKey
  (const ptr_lib::shared_ptr<DerNode>& algorithmParameters,
   const Blob& privateKeyDer, EcPrivateKeyLite& privateKey)
{
  // Find the curveId in EC_KEY_INFO.
  int curveId = -1;
  string oidString = algorithmParameters->toVal().toRawStr();
  for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
    const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
    OID curveOid(info->oidIntegerList, info->oidIntegerListLength);
    if (curveOid.toString() == oidString) {
      curveId = info->curveId;
      break;
    }
  }
  if (curveId == -1)
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Unrecognized EC algorithm parameters");

  // Get the value in the octet string.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(privateKeyDer.buf(), 0);
  DerNode::DerOctetString* octetString = dynamic_cast<DerNode::DerOctetString*>
    (parsedNode->getChildren()[1].get());
  if (!octetString)
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Can't get the private key octet string");
  Blob octetStringValue = octetString->toVal();

  ndn_Error error;
  if ((error = privateKey.setByCurve(curveId, octetStringValue)))
    throw SecurityException
      (string("PrivateKeyStorage::decodeEcPrivateKey ") + ndn_getErrorString(error));
}
#endif

}
