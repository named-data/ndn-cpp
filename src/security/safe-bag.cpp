/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/safe-bag.cpp
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

#include <ndn-cpp/security/certificate/public-key.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>
#include <ndn-cpp/security/safe-bag.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<CertificateV2>
SafeBag::makeSelfSignedCertificate
  (const Name& keyName, Blob privateKeyBag, Blob publicKeyEncoding,
   const uint8_t* password, size_t passwordLength,
   DigestAlgorithm digestAlgorithm, WireFormat& wireFormat)
{
  ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());

  // Set the name.
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  Name certificateName(keyName);
  certificateName.append("self").appendVersion((uint64_t)now);
  certificate->setName(certificateName);

  // Set the MetaInfo.
  certificate->getMetaInfo().setType(ndn_ContentType_KEY);
  // Set a one-hour freshness period.
  certificate->getMetaInfo().setFreshnessPeriod(3600 * 1000.0);

  // Set the content.
  PublicKey publicKey(publicKeyEncoding);
  certificate->setContent(publicKey.getKeyDer());

  // Create a temporary in-memory Tpm and import the private key.
  Tpm tpm("", "", ptr_lib::make_shared<TpmBackEndMemory>());
  tpm.importPrivateKey
    (keyName, privateKeyBag.buf(), privateKeyBag.size(), password,
     passwordLength);

  // Set the signature info.
  if (publicKey.getKeyType() == KEY_TYPE_RSA)
    certificate->setSignature(Sha256WithRsaSignature());
  else if (publicKey.getKeyType() == KEY_TYPE_EC)
    certificate->setSignature(Sha256WithEcdsaSignature());
  else
    throw invalid_argument("Unsupported key type");
  Signature* signatureInfo = certificate->getSignature();
  KeyLocator::getFromSignature(signatureInfo).setType(ndn_KeyLocatorType_KEYNAME);
  KeyLocator::getFromSignature(signatureInfo).setKeyName(keyName);

  // Set a 20-year validity period.
  ValidityPeriod::getFromSignature(signatureInfo).setPeriod
    (now, now + 20 * 365 * 24 * 3600 * 1000.0);

  // Encode once to get the signed portion.
  SignedBlob encoding = certificate->wireEncode(wireFormat);

  Blob signatureBytes = tpm.sign
    (encoding.signedBuf(), encoding.signedSize(), keyName, digestAlgorithm);
  signatureInfo->setSignature(signatureBytes);

  // Encode again to include the signature.
  certificate->wireEncode(wireFormat);

  return certificate;
}


}
