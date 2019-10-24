/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/certificate.hpp
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

#ifndef NDN_CERTIFICATE_V2_HPP
#define NDN_CERTIFICATE_V2_HPP

#include <stdexcept>
#include <ndn-cpp/security/validity-period.hpp>
#include <ndn-cpp/data.hpp>

namespace ndn {

/**
 * CertificateV2 represents a certificate following the certificate format
 * naming convention.
 *
 * Overview of the NDN certificate format:
 *
 *     CertificateV2 ::= DATA-TLV TLV-LENGTH
 *                         Name      (= /<NameSpace>/KEY/[KeyId]/[IssuerId]/[Version])
 *                         MetaInfo  (.ContentType = KEY)
 *                         Content   (= X509PublicKeyContent)
 *                         SignatureInfo (= CertificateV2SignatureInfo)
 *                         SignatureValue
 *
 *     X509PublicKeyContent ::= CONTENT-TLV TLV-LENGTH
 *                                BYTE+ (= public key bits in PKCS#8 format)
 *
 *     CertificateV2SignatureInfo ::= SIGNATURE-INFO-TYPE TLV-LENGTH
 *                                      SignatureType
 *                                      KeyLocator
 *                                      ValidityPeriod
 *                                      ... optional critical or non-critical extension blocks ...
 *
 * An example of NDN certificate name:
 *
 *     /edu/ucla/cs/yingdi/KEY/%03%CD...%F1/%9F%D3...%B7/%FD%d2...%8E
 *     \_________________/    \___________/ \___________/\___________/
 *    Certificate Namespace      Key Id       Issuer Id     Version
 *         (Identity)
 *     \__________________________________/
 *                   Key Name
 *
 * Notes:
 *
 * - `Key Id` is an opaque name component to identify the instance of the public
 *   key for the certificate namespace. The value of `Key ID` is controlled by
 *   the namespace owner. The library includes helpers for generating key IDs
 *   using an 8-byte random number, SHA-256 digest of the public key, timestamp,
 *   and the specified numerical identifiers.
 *
 * - `Issuer Id` is sn opaque name component to identify the issuer of the
 *   certificate. The value is controlled by the issuer. The library includes
 *   helpers to set issuer the ID to an 8-byte random number, SHA-256 digest of
 *   the issuer's public key, and the specified numerical identifiers.
 *
 * - `Key Name` is a logical name of the key used for management purposes. the
 *    Key Name includes the certificate namespace, keyword `KEY`, and `KeyId`
 *    components.
 *
 * @see https://github.com/named-data/ndn-cxx/blob/master/docs/specs/certificate-format.rst
 */
class CertificateV2 : public Data {
public:
  /**
   * A CertificateV2::Error extends runtime_error and represents errors for not
   * complying with the certificate format.
   */
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  /**
   * Create a CertificateV2 with content type KEY and default or unspecified
   * values.
   */
  CertificateV2();

  /**
   * Create a CertificateV2 from the content in the Data packet.
   * @param data The data packet with the content to copy.
   * @throws CertificateV2::Error If data does not follow the certificate format.
   */
  CertificateV2(const Data& data);

  /**
   * Get key name from the certificate name.
   * @return The key name as a new Name.
   */
  Name
  getKeyName() const { return getName().getPrefix(KEY_ID_OFFSET + 1); }

  /**
   * Get the identity name from the certificate name.
   * @return The identity name as a new Name.
   */
  Name
  getIdentity() const { return getName().getPrefix(KEY_COMPONENT_OFFSET); }

  /**
   * Get the key ID component from the certificate name.
   * @return The key ID name component.
   */
  Name::Component
  getKeyId() const { return getName().get(KEY_ID_OFFSET); }

  /**
   * Get issuer ID component from the certificate name.
   * @return The issuer ID component.
   */
  Name::Component
  getIssuerId() const { return getName().get(ISSUER_ID_OFFSET); }

  /**
   * Get the public key DER encoding.
   * @return The DER encoding Blob.
   * @throws CertificateV2::Error If the public key is not set.
   */
  const Blob&
  getPublicKey() const;

  /**
   * Get the certificate validity period from the SignatureInfo.
   * @return The ValidityPeriod object.
   * @throws runtime_error If the SignatureInfo doesn't have a ValidityPeriod.
   */
  ValidityPeriod&
  getValidityPeriod();

  const ValidityPeriod&
  getValidityPeriod() const
  {
    return const_cast<CertificateV2*>(this)->getValidityPeriod();
  }

  /**
   * Check if the time falls within the validity period.
   * @param time (optional) The time to check as milliseconds since Jan 1,
   * 1970 UTC. If omitted, use the current time.
   * @return True if the beginning of the validity period is less than or equal
   * to time and time is less than or equal to the end of the validity period.
   * @throws runtime_error If the SignatureInfo doesn't have a ValidityPeriod.
   */
  bool
  isValid(MillisecondsSince1970 time = -1.0) const
  {
    return getValidityPeriod().isValid(time);
  }

  // TODO: getExtension

  /**
   * Print the certificate information to the given output stream.
   * @param output The output stream.
   */
  void
  printCertificate(std::ostream& output) const;

  /**
   * Print the certificate information to std::cout.
   */
  void
  printCertificate() const { printCertificate(std::cout); }

  /**
   * Override to call the base class wireDecode then check the certificate
   * format.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat A WireFormat object used to decode the input. If omitted,
   * use WireFormat getDefaultWireFormat().
   */
  virtual void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  void
  wireDecode
    (const uint8_t* input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(Blob(input, inputLength), wireFormat);
  }

  void
  wireDecode
    (const std::vector<uint8_t>& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Check if certificateName follows the naming convention for a certificate.
   * @param certificateName The name of the certificate.
   * @return True if certificateName follows the naming convention.
   */
  static bool
  isValidName(const Name& certificateName);

  /**
   * Extract the identity namespace from certificateName.
   * @param certificateName The name of the certificate.
   * @return The identity namespace as a new Name.
   */
  static Name
  extractIdentityFromCertName(const Name& certificateName);

  /**
   * Extract key name from certificateName.
   * @param certificateName The name of the certificate.
   * @return The key name as a new Name.
   */
  static Name
  extractKeyNameFromCertName(const Name& certificateName);

  /**
   * Get the name component for "KEY". This is a method because not all C++
   * environments support static constructors.
   * @return The name component for "KEY".
   */
  static const Name::Component&
  getKEY_COMPONENT();

  static const int VERSION_OFFSET = -1;
  static const int ISSUER_ID_OFFSET = -2;
  static const int KEY_ID_OFFSET = -3;
  static const int KEY_COMPONENT_OFFSET = -4;
  static const int MIN_CERT_NAME_LENGTH = 4;
  static const int MIN_KEY_NAME_LENGTH = 2;

private:
  static Name::Component* KEY_COMPONENT;

  void
  checkFormat();
};

inline std::ostream&
operator << (std::ostream& os, const CertificateV2& certificate)
{
  certificate.printCertificate(os);
  return os;
}

}

#endif
