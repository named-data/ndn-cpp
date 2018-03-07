/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/pib/detail/key-impl.hpp
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

#ifndef NDN_PIB_KEY_IMPL_HPP
#define NDN_PIB_KEY_IMPL_HPP

#include <ndn-cpp/security/pib/pib-certificate-container.hpp>
#include <ndn-cpp/security/security-common.hpp>

// Give friend access to the tests.
class TestPibKeyImpl_CertificateOperation_Test;

namespace ndn {

/**
 * PibKeyImpl provides the backend implementation for PibKey. A PibKey has only
 * one backend instance, but may have multiple frontend handles. Each frontend
 * handle is associated with the only one backend PibKeyImpl.
 */
class PibKeyImpl {
public:
  /**
   * Create a PibKeyImpl with keyName. If the key does not exist in the backend
   * implementation, add it by creating it from the keyEncoding. If a key with
   * keyName already exists, overwrite it.
   * @param keyName The name of the key, which is copied.
   * @param keyEncoding The array of encoded key bytes, which is copied.
   * @param keyEncodingLength The number of bytes in the key encoding array.
   * @param pibImpl The Pib backend implementation.
   */
  PibKeyImpl
    (const Name& keyName, const uint8_t* keyEncoding, size_t keyEncodingLength,
     const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  /**
   * Create a PibKeyImpl with keyName. Initialize the cached key encoding with
   * pibImpl->getKeyBits().
   * @param keyName The name of the key, which is copied.
   * @param pibImpl The Pib backend implementation.
   * @throws Pib::Error the key with keyName does not exist.
   */
  PibKeyImpl(const Name& keyName, const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  /*
   * Get the key name.
   * @return The key name.
   */
  const Name&
  getName() const { return keyName_; }

  /**
   * Get the name of the identity this key belongs to.
   * @return The name of the identity.
   */
  const Name&
  getIdentityName() const { return identityName_; }

  /**
   * Get the key type.
   * @return The key type.
   */
  KeyType
  getKeyType() const { return keyType_; }

  /**
   * Get the public key encoding.
   * @return The public key encoding.
   */
  const Blob&
  getPublicKey() const { return keyEncoding_; }

  /**
   * Add the certificate. If a certificate with the same name (without implicit
   * digest) already exists, then overwrite the certificate. If no default
   * certificate for the key has been set, then set the added certificate as
   * default for the key.
   * @param certificate The certificate to add. This copies the object.
   * @throws std::invalid_argument if the name of the certificate does not
   * match the key name.
   */
  void
  addCertificate(const CertificateV2& certificate);

  /**
   * Remove the certificate with name certificateName. If the certificate does
   * not exist, do nothing.
   * @param certificateName The name of the certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name.
   */
  void
  removeCertificate(const Name& certificateName);

  /**
   * Get the certificate with name certificateName.
   * @param certificateName The name of the certificate.
   * @return A copy of the CertificateV2 object.
   * @throws std::invalid_argument if certificateName does not match the key name.
   * @throws Pib::Error if the certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>
  getCertificate(const Name& certificateName);

  /**
   * Set the existing certificate with name certificateName as the default
   * certificate.
   * @param certificateName The name of the certificate.
   * @return The default certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name
   * @throws Pib::Error if the certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>&
  setDefaultCertificate(const Name& certificateName);

  /**
   * Add the certificate and set it as the default certificate of the key.
   * If a certificate with the same name (without implicit digest) already
   * exists, then overwrite the certificate.
   * @param certificate The certificate to add. This copies the object.
   * @throws std::invalid_argument if the name of the certificate does not
   * match the key name.
   * @return The default certificate.
   */
  ptr_lib::shared_ptr<CertificateV2>&
  setDefaultCertificate(const CertificateV2& certificate)
  {
    addCertificate(certificate);
    return setDefaultCertificate(certificate.getName());
  }

  /**
   * Get the default certificate for this Key.
   * @return A copy of the default certificate.
   * @throws Pib::Error if the default certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>&
  getDefaultCertificate();

private:
  friend class PibKey;
  // Give friend access to the tests.
  friend TestPibKeyImpl_CertificateOperation_Test;

  // Disable the copy constructor and assignment operator.
  PibKeyImpl(const PibKeyImpl& other);
  PibKeyImpl& operator=(const PibKeyImpl& other);

  Name identityName_;
  Name keyName_;
  Blob keyEncoding_;
  KeyType keyType_;

  ptr_lib::shared_ptr<CertificateV2> defaultCertificate_;

  PibCertificateContainer certificates_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
