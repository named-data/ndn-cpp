/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/certificate.cpp
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

#include <ndn-cpp/security/validity-period.hpp>
#include <ndn-cpp/encrypt/schedule.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "../../encoding/base64.hpp"
#include <ndn-cpp/security/v2/certificate-v2.hpp>

#include "ndn-cpp/key-locator.hpp"

using namespace std;

namespace ndn {

CertificateV2::CertificateV2()
{
  getMetaInfo().setType(ndn_ContentType_KEY);
}

CertificateV2::CertificateV2(const Data& data)
// Use the copy constructor.  It clones the signature object.
: Data(data)
{
  checkFormat();
}

void
CertificateV2::checkFormat()
{
  if (!isValidName(getName()))
    throw Error
      ("The Data Name does not follow the certificate naming convention");

  if (getMetaInfo().getType() != ndn_ContentType_KEY)
    throw Error("The Data ContentType is not KEY");

  if (getMetaInfo().getFreshnessPeriod() < 0.0)
    throw Error("The Data FreshnessPeriod is not set");

  if (getContent().size() == 0)
    throw Error("The Data Content is empty");
}

const Blob&
CertificateV2::getPublicKey() const
{
  if (getContent().size() == 0)
    throw Error("The public key is not set (the Data content is empty)");

  return getContent();
}

ValidityPeriod&
CertificateV2::getValidityPeriod()
{
  if (!ValidityPeriod::canGetFromSignature(getSignature()))
    throw invalid_argument("The SignatureInfo does not have a ValidityPeriod");

  return ValidityPeriod::getFromSignature(getSignature());
}

void
CertificateV2::printCertificate(ostream& output) const
{
  output << "Certificate name:\n";
  output << "  " << getName() << "\n";
  output << "Validity:\n";
  output << "  NotBefore: " << Schedule::toIsoString
    (getValidityPeriod().getNotBefore()) << "\n";
  output << "  NotAfter: "  << Schedule::toIsoString
    (getValidityPeriod().getNotAfter()) << "\n";

  /* TODO: Print the extension.
  try {
    const Block& info = cert.getSignature().getSignatureInfo()
      .getTypeSpecificTlv(tlv::AdditionalDescription);
    output << "Additional Description:\n";
    for (const auto& item : v2::AdditionalDescription(info)) {
      output << "  " << item.first << ": " << item.second << "\n";
    }
  }
  catch (const SignatureInfo::Error&) {
    // ignore
  }
  */

  output << "Public key bits:\n";
  output << toBase64(*getPublicKey(), true);

  output << "Signature Information:\n";
  output << "  Signature Type: ";
  if (dynamic_cast<const Sha256WithEcdsaSignature*>(getSignature()))
    output << "SignatureSha256WithEcdsa\n";
  else if (dynamic_cast<const Sha256WithRsaSignature*>(getSignature()))
    output << "SignatureSha256WithRsa\n";
  else
    output << "<unknown>\n";

  if (KeyLocator::canGetFromSignature(getSignature())) {
    output << "  Key Locator: ";
    const KeyLocator& keyLocator(KeyLocator::getFromSignature(getSignature()));
    if (keyLocator.getType() == ndn_KeyLocatorType_KEYNAME) {
      if (keyLocator.getKeyName().equals(getKeyName()))
        output << "Self-Signed ";

      output << "Name=" << keyLocator.getKeyName() << "\n";
    }
    else
      output << "<no KeyLocator key name>\n";
  }
}

void
CertificateV2::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  Data::wireDecode(input, wireFormat);
  checkFormat();
}

bool
CertificateV2::isValidName(const Name& certificateName)
{
  // /<NameSpace>/KEY/[KeyId]/[IssuerId]/[Version]
  return (certificateName.size() >= MIN_CERT_NAME_LENGTH &&
          certificateName.get(KEY_COMPONENT_OFFSET) == getKEY_COMPONENT());
}

Name
CertificateV2::extractIdentityFromCertName(const Name& certificateName)
{
  if (!isValidName(certificateName)) {
    throw invalid_argument
      ("Certificate name `" + certificateName.toUri() +
        "` does not follow the naming conventions");
  }

  return certificateName.getPrefix(KEY_COMPONENT_OFFSET);
}

Name
CertificateV2::extractKeyNameFromCertName(const Name& certificateName)
{
  if (!isValidName(certificateName)) {
    throw invalid_argument
      ("Certificate name `" + certificateName.toUri() +
        "` does not follow the naming conventions");
  }

  // Trim everything after the key ID.
  return certificateName.getPrefix(KEY_ID_OFFSET + 1);
}

const Name::Component&
CertificateV2::getKEY_COMPONENT()
{
  if (!KEY_COMPONENT)
    KEY_COMPONENT = new Name::Component("KEY");

  return *KEY_COMPONENT;
}

Name::Component* CertificateV2::KEY_COMPONENT = 0;

}
