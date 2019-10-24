/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/certificate-container.hpp
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

#ifndef NDN_PIB_CERTIFICATE_CONTAINER_HPP
#define NDN_PIB_CERTIFICATE_CONTAINER_HPP

#include <set>
#include <map>
#include "../v2/certificate-v2.hpp"

// Give friend access to the tests.
class TestPibCertificateContainer_Basic_Test;
class TestPibCertificateContainer_Errors_Test;
class TestKeyChain_Management_Test;

namespace ndn {

class PibImpl;
class PibKey;

/**
 * A PibCertificateContainer is used to search/enumerate the certificates of a
 * key. (A PibCertificateContainer object can only be created by PibKey.)
 */
class PibCertificateContainer {
public:
  /**
   * Get the number of certificates in the container.
   * @return The number of certificates.
   */
  size_t
  size() const { return certificateNames_.size(); }

  /**
   * Add certificate into the container. If the certificate already exists,
   * this replaces it.
   * @param certificate The certificate to add. This copies the object.
   * @throws std::invalid_argument if the name of the certificate does not
   * match the key name.
   */
  void
  add(const CertificateV2& certificate);

  /**
   * Remove the certificate with name certificateName from the container. If the
   * certificate does not exist, do nothing.
   * @param certificateName The name of the certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name.
   */
  void
  remove(const Name& certificateName);

  /**
   * Get the certificate with certificateName from the container.
   * @param certificateName The name of the certificate.
   * @return A copy of the certificate.
   * @throws std::invalid_argument if certificateName does not match the key
   * name
   * @throws Pib::Error if the certificate does not exist.
   */
  ptr_lib::shared_ptr<CertificateV2>
  get(const Name& certificateName);

  /**
   * Check if the container is consistent with the backend storage.
   * @return True if the container is consistent, false otherwise.
   * @note This method is heavy-weight and should be used in a debugging mode
   * only.
   */
  bool
  isConsistent() const;

private:
  friend class PibKeyImpl;
  // Give friend access to the tests.
  friend class ::TestPibCertificateContainer_Basic_Test;
  friend class ::TestPibCertificateContainer_Errors_Test;
  friend class ::TestKeyChain_Management_Test;

  /**
   * Create a PibCertificateContainer for a key with keyName. This constructor
   * should only be called by PibKeyImpl.
   * @param keyName The name of the key, which is copied.
   * @param pibImpl The PIB backend implementation.
   */
  PibCertificateContainer
    (const Name& keyName, const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  // Disable the copy constructor and assignment operator.
  PibCertificateContainer(const PibCertificateContainer& other);
  PibCertificateContainer& operator=(const PibCertificateContainer& other);

  Name keyName_;
  std::set<Name> certificateNames_;
  // Cache of loaded certificates.
  std::map<Name, ptr_lib::shared_ptr<CertificateV2>> certificates_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
