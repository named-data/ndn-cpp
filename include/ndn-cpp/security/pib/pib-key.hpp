/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/key.hpp
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

#ifndef NDN_PIB_KEY_HPP
#define NDN_PIB_KEY_HPP

#include "../security-common.hpp"
#include "pib-certificate-container.hpp"

// Give friend access to the tests.
class TestKeyChain_Management_Test;

namespace ndn {

class PibKeyImpl;

/**
 * The PibKey class provides access to a key at the second level in the PIB's
 * Identity-Key-Certificate hierarchy. A PibKey object has a Name
 * (identity + "KEY" + keyId), and contains one or more CertificateV2
 * objects, one of which is set as the default certificate of this key.
 * A certificate can be directly accessed by getting a CertificateV2 object.
 */
class PibKey {
public:
  /*
   * Get the key name.
   * @return The key name.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  const Name&
  getName();

  /**
   * Get the name of the identity this key belongs to.
   * @return The name of the identity.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  const Name&
  getIdentityName();

  /**
   * Get the key type.
   * @return The key type.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  KeyType
  getKeyType();

  /**
   * Get the public key encoding.
   * @return The public key encoding.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  const Blob&
  getPublicKey();

  /**
   * Get the certificate with name certificateName.
   * @param certificateName The name of the certificate.
   * @return A copy of the CertificateV2 object.
   * @throws std::domain_error if the backend implementation instance is invalid.
   * @throws std::invalid_argument if certificateName does not match the key name.
   * @throws Pib::Error if the certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>
  getCertificate(const Name& certificateName);

  /**
   * Get the default certificate for this Key.
   * @return A copy of the default certificate.
   * @throws std::domain_error if the backend implementation instance is invalid.
   * @throws Pib::Error if the default certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>&
  getDefaultCertificate();

  /**
   * Construct a key name based on the appropriate naming conventions.
   * @param identityName The name of the identity.
   * @param keyId The key ID name component.
   * @return The constructed name as a new Name.
   */
  static Name
  constructKeyName(const Name& identityName, const Name::Component& keyId);

  /**
   * Check if keyName follows the naming conventions for a key name.
   * @param keyName The name of the key.
   * @return True if keyName follows the naming conventions, otherwise false.
   */
  static bool
  isValidKeyName(const Name& keyName);

  /**
   * Extract the identity namespace from keyName.
   * @param keyName The name of the key.
   * @return The identity name as a new Name.
   */
  static Name
  extractIdentityFromKeyName(const Name& keyName);

private:
  friend class PibKeyContainer;
  friend class KeyChain;
  // Give friend access to the tests.
  friend class ::TestKeyChain_Management_Test;

  /**
   * Create a PibKey which uses the impl backend implementation. This
   * constructor should only be called by PibKeyContainer.
   */
  PibKey(ptr_lib::weak_ptr<PibKeyImpl> impl);

  /**
   * Add the certificate. If a certificate with the same name (without implicit
   * digest) already exists, then overwrite the certificate. If no default
   * certificate for the key has been set, then set the added certificate as
   * default for the key.
   * This should only be called by KeyChain.
   * @param certificate The certificate to add. This copies the object.
   * @throws std::invalid_argument if the name of the certificate does not
   * match the key name.
   */
  void
  addCertificate(const CertificateV2& certificate);

  /**
   * Remove the certificate with name certificateName. If the certificate does
   * not exist, do nothing.
   * This should only be called by KeyChain.
   * @param certificateName The name of the certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name.
   */
  void
  removeCertificate(const Name& certificateName);

  /**
   * Set the existing certificate with name certificateName as the default
   * certificate.
   * This should only be called by KeyChain.
   * @param certificateName The name of the certificate.
   * @return The default certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name
   * @throws Pib::Error if the certificate does not exist.
   */
  const ptr_lib::shared_ptr<CertificateV2>&
  setDefaultCertificate(const Name& certificateName);

  /**
   * Add the certificate and set it as the default certificate of the key.
   * If a certificate with the same name (without implicit digest) already
   * exists, then overwrite the certificate.
   * This should only be called by KeyChain.
   * @param certificate The certificate to add. This copies the object.
   * @throws std::invalid_argument if the name of the certificate does not
   * match the key name.
   * @return The default certificate.
   */
  const ptr_lib::shared_ptr<CertificateV2>&
  setDefaultCertificate(const CertificateV2& certificate);

  /**
   * Get the PibCertificateContainer in the PibKeyImpl. This should only be
   * called by KeyChain.
   */
  PibCertificateContainer&
  getCertificates();

  const PibCertificateContainer&
  getCertificates() const { return const_cast<PibKey*>(this)->getCertificates(); }

  /**
   * Check the validity of the impl_ instance.
   * @return A shared_ptr when the instance is valid.
   * @throws std::domain_error if the backend implementation instance is invalid.
   */
  ptr_lib::shared_ptr<PibKeyImpl>
  lock() const;

  // Disable the copy constructor and assignment operator.
  PibKey(const PibKey& other);
  PibKey& operator=(const PibKey& other);

  ptr_lib::weak_ptr<PibKeyImpl> impl_;
};

}

#endif
