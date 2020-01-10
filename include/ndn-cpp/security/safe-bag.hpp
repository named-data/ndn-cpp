/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/safe-bag.hpp
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

#ifndef NDN_SAFE_BAG_HPP
#define NDN_SAFE_BAG_HPP

#include "security-common.hpp"
#include "v2/certificate-v2.hpp"

namespace ndn {

class TlvEncoder;

/**
 * A SafeBag represents a container for sensitive related information such as a
 * certificate and private key.
 */
class SafeBag {
public:
  /**
   * Create a SafeBag with the given certificate and private key.
   * @param certificate The certificate data packet. This copies the object.
   * @param privateKeyBag The encoded private key. If encrypted, this is a
   * PKCS #8 EncryptedPrivateKeyInfo. If not encrypted, this is an unencrypted
   * PKCS #8 PrivateKeyInfo.
   */
  SafeBag(const Data& certificate, Blob privateKeyBag)
  {
    certificate_ = ptr_lib::make_shared<Data>(certificate);
    privateKeyBag_ = privateKeyBag;
  }

  /**
   * Create a SafeBag with given private key and a new self-signed certificate
   * for the given public key.
   * @param keyName The name of the public key. The certificate name will be
   * {keyName}/self/{version} where the version is based on the current time.
   * This copies the Name.
   * @param privateKeyBag The encoded private key. If encrypted, this is a
   * PKCS #8 EncryptedPrivateKeyInfo. If not encrypted, this is an unencrypted
   * PKCS #8 PrivateKeyInfo.
   * @param publicKeyEncoding The encoded public key for the certificate.
   * @param password (optional) The password for decrypting the private key in
   * order to sign the self-signed certificate, which should have characters in
   * the range of 1 to 127. If the password is supplied, use it to decrypt the
   * PKCS #8 EncryptedPrivateKeyInfo. If the password is omitted or null,
   * privateKeyBag is an unencrypted PKCS #8 PrivateKeyInfo.
   * @param passwordLength (optional) The length of the password. If password is
   * omitted or null, this is ignored.
   * @param digestAlgorithm (optional) The digest algorithm for signing the
   * self-signed certificate. If omitted, use DIGEST_ALGORITHM_SHA256 .
   * @param wireFormat (optional) A WireFormat object used to encode the
   * self-signed certificate in order to sign it. If omitted, use WireFormat
   * getDefaultWireFormat().
   */
  SafeBag
    (const Name& keyName, Blob privateKeyBag, Blob publicKeyEncoding,
     const uint8_t* password = 0, size_t passwordLength = 0,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    certificate_ = makeSelfSignedCertificate
      (keyName, privateKeyBag, publicKeyEncoding, password, passwordLength,
       digestAlgorithm, wireFormat);
    privateKeyBag_ = privateKeyBag;
  }

  /**
   * Create a SafeBag by decoding the input as an NDN-TLV SafeBag.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  SafeBag(const uint8_t* input, size_t inputLength)
  {
    wireDecode(input, inputLength);
  }

  /**
   * Create a SafeBag by decoding the input as an NDN-TLV SafeBag.
   * @param input The input buffer to decode.
   */
  SafeBag(const Blob& input)
  {
    wireDecode(input);
  }

  /**
   * Get the certificate data packet.
   * @return The certificate as a Data packet. If you need to process it as a
   * certificate object then you must create a new CertificateV2(data).
   */
  const ptr_lib::shared_ptr<Data>& getCertificate() const { return certificate_; }

  /**
   * Get the encoded private key.
   * @return The encoded private key. If encrypted, this is a PKCS #8
   * EncryptedPrivateKeyInfo. If not encrypted, this is an unencrypted PKCS #8
   * PrivateKeyInfo.
   */
  const Blob& getPrivateKeyBag() const { return privateKeyBag_; }

  /**
   * Decode the input as an NDN-TLV SafeBag and update this object.
   * @param input A pointer to the input buffer to decode.
   * @param inputLength The number of bytes in input.
   */
  void
  wireDecode(const uint8_t* input, size_t inputLength);

  /**
   * Decode the input as an NDN-TLV SafeBag and update this object.
   * @param input The input buffer to decode.
   */
  void
  wireDecode(const Blob& input)
  {
    wireDecode(input.buf(), input.size());
  }

  /**
   * Encode this as an NDN-TLV SafeBag.
   * @return The encoding as a Blob.
   */
  Blob
  wireEncode();

private:
  static ptr_lib::shared_ptr<CertificateV2>
  makeSelfSignedCertificate
    (const Name& keyName, Blob privateKeyBag, Blob publicKeyEncoding,
     const uint8_t* password, size_t passwordLength,
     DigestAlgorithm digestAlgorithm, WireFormat& wireFormat);

  static void
  encodeValue(const void *context, TlvEncoder &encoder);

  ptr_lib::shared_ptr<Data> certificate_;
  Blob privateKeyBag_;
};

}

#endif
