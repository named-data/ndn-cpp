/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/certificate-storage.hpp
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

#ifndef NDN_CERTIFICATE_STORAGE_HPP
#define NDN_CERTIFICATE_STORAGE_HPP

#include "certificate-cache-v2.hpp"
#include "trust-anchor-container.hpp"

namespace ndn {

/**
 * The CertificateStorage class stores trusted anchors and has a verified
 * certificate cache, and an unverified certificate cache.
 */
class CertificateStorage {
public:
  CertificateStorage()
  : verifiedCertificateCache_(3600 * 1000.0),
    unverifiedCertificateCache_(300 * 1000.0)
  {
  }

  /**
   * Find a trusted certificate in the trust anchor container or in the
   * verified cache.
   * @param interestForCertificate The Interest for the certificate.
   * @return The found certificate, or null if not found.
   */
  ptr_lib::shared_ptr<CertificateV2>
  findTrustedCertificate(const Interest& interestForCertificate);

  /**
   * Check if the certificate with the given name prefix exists in the verified
   * cache, the unverified cache, or in the set of trust anchors.
   * @param certificatePrefix The certificate name prefix.
   * @return True if the certificate is known.
   */
  bool
  isCertificateKnown(const Name& certificatePrefix);

  /**
   * Cache the unverified certificate for a period of time (5 minutes).
   * @param certificate The certificate packet, which is copied.
   */
  void
  cacheUnverifiedCertificate(const CertificateV2& certificate)
  {
    unverifiedCertificateCache_.insert(certificate);
  }

  /**
   * Get the trust anchor container.
   * @return The trust anchor container.
   */
  const TrustAnchorContainer&
  getTrustAnchors() const { return trustAnchors_; }

  /**
   * Get the verified certificate cache.
   * @return The verified certificate cache.
   */
  const CertificateCacheV2&
  getVerifiedCertificateCache() const { return verifiedCertificateCache_; }

  /**
   * Get the unverified certificate cache.
   * @return The unverified certificate cache.
   */
  const CertificateCacheV2&
  getUnverifiedCertificateCache() const { return unverifiedCertificateCache_; }

  /**
   * Load a static trust anchor. Static trust anchors are permanently associated
   * with the validator and never expire.
   * @param groupId The certificate group id.
   * @param certificate The certificate to load as a trust anchor, which is
   * copied.
   */
  void
  loadAnchor(const std::string& groupId, const CertificateV2& certificate)
  {
    trustAnchors_.insert(groupId, certificate);
  }

  /**
   * Load dynamic trust anchors. Dynamic trust anchors are associated with the
   * validator for as long as the underlying trust anchor file (or set of files)
   * exists.
   * @param groupId The certificate group id, which must not be empty.
   * @param path The path to load the trust anchors.
   * @param refreshPeriod  The refresh time in milliseconds for the anchors
   * under path. This must be positive. The relevant trust anchors will only be
   * updated when find is called.
   * @param isDirectory (optional) If true, then path is a directory. If false
   * or omitted, it is a single file.
   * @throws std::invalid_argument If refreshPeriod is not positive.
   * @throws TrustAnchorContainer::Error a group with groupId already exists
   */
  void
  loadAnchor
    (const std::string& groupId, const std::string& path,
     Milliseconds refreshPeriod, bool isDirectory = false)
  {
    trustAnchors_.insert(groupId, path, refreshPeriod, isDirectory);
  }

  /**
   * Remove any previously loaded static or dynamic trust anchors.
   */
  void
  resetAnchors() { trustAnchors_.clear(); }

  /**
   * Cache the verified certificate a period of time (1 hour).
   * @param certificate The certificate object, which is copied.
   */
  void
  cacheVerifiedCertificate(const CertificateV2& certificate)
  {
    verifiedCertificateCache_.insert(certificate);
  }

  /**
   * Remove any cached verified certificates.
   */
  void
  resetVerifiedCertificates() { verifiedCertificateCache_.clear(); }

  /**
   * Set the offset when the cache insert() and refresh() get the current time,
   * which should only be used for testing.
   * @param nowOffsetMilliseconds The offset in milliseconds.
   */
  void
  setCacheNowOffsetMilliseconds_(Milliseconds nowOffsetMilliseconds)
  {
    verifiedCertificateCache_.setNowOffsetMilliseconds_(nowOffsetMilliseconds);
    unverifiedCertificateCache_.setNowOffsetMilliseconds_(nowOffsetMilliseconds);
  }

private:
  // Disable the copy constructor and assignment operator.
  CertificateStorage(const CertificateStorage& other);
  CertificateStorage& operator=(const CertificateStorage& other);

protected:
  TrustAnchorContainer trustAnchors_;
  CertificateCacheV2 verifiedCertificateCache_;
  CertificateCacheV2 unverifiedCertificateCache_;
};

}

#endif
