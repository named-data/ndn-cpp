/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/trust-anchor-group.hpp
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

#ifndef NDN_TRUST_ANCHOR_GROUP_HPP
#define NDN_TRUST_ANCHOR_GROUP_HPP

#include <set>
#include "certificate-v2.hpp"

namespace ndn {

class CertificateContainerInterface
{
public:
  virtual
  ~CertificateContainerInterface();

  /**
   * Add the certificate to the container.
   * @param certificate The certificate to add, which is copied.
   */
  virtual void
  add(const CertificateV2& certificate) = 0;

  /**
   * Remove the certificate with the given name. If the name does not exist,
   * do nothing.
   * @param certificateName The name of the certificate.
   */
  virtual void
  remove(const Name& certificateName) = 0;
};

/**
 * TrustAnchorGroup represents a group of trust anchors which implement the
 * CertificateContainerInterface.
 */
class TrustAnchorGroup
{
public:
  /**
   * Create a TrustAnchorGroup to use an existing container.
   * @param certificateContainer The existing certificate container.
   * @param id The group ID.
   */
  TrustAnchorGroup
    (CertificateContainerInterface& certificateContainer, const std::string& id)
  : certificates_(certificateContainer),
    id_(id)
  {
  }

  virtual
  ~TrustAnchorGroup();

  /**
   * Get the group id given to the constructor.
   * @return The group id.
   */
  const std::string&
  getId() const { return id_; }

  /**
   * Get the number of certificates in the group.
   * @return The number of certificates.
   */
  size_t
  size() const { return anchorNames_.size(); }

  /**
   * Request a certificate refresh. The base method does nothing.
   */
  virtual void
  refresh();

  /**
   * Read a base-64-encoded certificate from a file.
   * @param filePath The certificate file path.
   * @return The decoded certificate, or null if there is an error.
   */
  static ptr_lib::shared_ptr<CertificateV2>
  readCertificate(const std::string& filePath);

protected:
  CertificateContainerInterface& certificates_;
  std::set<Name> anchorNames_;

private:
  // Disable the copy constructor and assignment operator.
  TrustAnchorGroup(const TrustAnchorGroup& other);
  TrustAnchorGroup& operator=(const TrustAnchorGroup& other);

  std::string id_;
};

/**
 * The StaticTrustAnchorGroup class extends TrustAnchorGroup to implement a
 * static trust anchor group.
 */
class StaticTrustAnchorGroup : public TrustAnchorGroup {
public:
  StaticTrustAnchorGroup
    (CertificateContainerInterface& certificateContainer, const std::string& id)
  : TrustAnchorGroup(certificateContainer, id)
  {
  }

  /**
   * Load the static anchor certificate. If a certificate with the name is
   * already added, do nothing.
   * @param certificate The certificate to add, which is copied.
   */
  void
  add(const CertificateV2& certificate);

  /**
   * Remove the static anchor with the certificate name.
   * @param certificateName The certificate name.
   */
  void
  remove(const Name& certificateName);
};

/**
 * The DynamicTrustAnchorGroup class extends TrustAnchorGroup to implement a
 * dynamic trust anchor group.
 */
class DynamicTrustAnchorGroup : public TrustAnchorGroup {
public:
  /**
   * Create a dynamic trust anchor group.
   *
   * This loads all the certificates from the path and will refresh certificates
   * every refreshPeriod milliseconds.
   *
   * Note that refresh is not scheduled, but is performed upon each "find"
   * operations.
   *
   * When isDirectory is false and the path doesn't point to a valid certificate
   * (the file doesn't exist or the content is not a valid certificate), then
   * the dynamic anchor group will be empty until the file gets created. If the
   * file disappears or gets corrupted, the anchor group becomes empty.
   *
   * When isDirectory is true and the path doesn't point to a valid folder, the
   * folder is empty, or it doesn't contain valid certificates, then the group
   * will be empty until certificate files are placed in the folder. If the
   * folder is removed, becomes empty, or no longer contains valid certificates,
   * then the anchor group becomes empty.
   *
   * Upon refresh, the existing certificates are not changed.
   *
   * @param certificateContainer A certificate container into which trust
   * anchors from the group will be added.
   * @param id The group id.
   * @param path The file path for trust anchor(s), which could be a directory
   * or a file. If it is a directory, all the certificates in the directory will
   * be loaded.
   * @param refreshPeriod  The refresh time in milliseconds for the anchors
   * under path. This must be positive.
   * @param isDirectory (optional) If true, then path is a directory. If false
   * or omitted, it is a single file.
   * @throws std::invalid_argument If refreshPeriod is not positive.
   */
  DynamicTrustAnchorGroup
    (CertificateContainerInterface& certificateContainer, const std::string& id,
     const std::string& path, Milliseconds refreshPeriod,
     bool isDirectory = false);

  /**
   * Request a certificate refresh.
   */
  virtual void
  refresh();

private:
  void
  loadCertificate(const std::string& file, std::set<Name>& oldAnchorNames);

  bool isDirectory_;
  std::string path_;
  Milliseconds refreshPeriod_;
  Milliseconds expireTime_;
};

}

#endif
