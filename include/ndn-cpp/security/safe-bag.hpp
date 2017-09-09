/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/safe-bag.hpp
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

/**
 * A SafeBag represents a container for sensitive related information such as a
 * certificate and private key.
 */
class SafeBag {
public:
  /**
   * Create a new Safe object with the given certificate and private key.
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
   * Get the certificate data packet.
   * @return The certificate as a Data packet, or null if it has not been set.
   * If you need to process it as a certificate object then you must create a
   * new CertificateV2(data).
   */
  const ptr_lib::shared_ptr<Data>& getCertificate() const { return certificate_; }

  /**
   * Get the encoded private key.
   * @return The encoded private key, or an isNull Blob if it hasn't been set.
   * If encrypted, this is a PKCS #8 EncryptedPrivateKeyInfo. If not encrypted,
   * this is an unencrypted PKCS #8 PrivateKeyInfo.
   */
  const Blob& getPrivateKeyBag() const { return privateKeyBag_; }

private:
  static ptr_lib::shared_ptr<CertificateV2>
  makeSelfSignedCertificate
    (const Name& keyName, Blob privateKeyBag, Blob publicKeyEncoding,
     const uint8_t* password, size_t passwordLength,
     DigestAlgorithm digestAlgorithm, WireFormat& wireFormat);

  ptr_lib::shared_ptr<Data> certificate_;
  Blob privateKeyBag_;
};

}

#endif
