/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/trust-anchor-container.hpp
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

#ifndef NDN_TRUST_ANCHOR_CONTAINER_HPP
#define NDN_TRUST_ANCHOR_CONTAINER_HPP

#include <stdexcept>
#include <map>
#include "trust-anchor-group.hpp"

namespace ndn {

/**
 * A TrustAnchorContainer represents a container for trust anchors.
 *
 * There are two kinds of anchors:
 * static anchors that are permanent for the lifetime of the container, and
 * dynamic anchors that are periodically updated.
 *
 * Trust anchors are organized in groups. Each group has a unique group id.
 * The same anchor certificate (same name without considering the implicit
 * digest) can be inserted into multiple groups, but no more than once into each.
 *
 * Dynamic groups are created using the appropriate TrustAnchorContainer.insert
 * method. Once created, the dynamic anchor group cannot be updated.
 *
 * The returned pointer to Certificate from `find` methods is only guaranteed to
 * be valid until the next invocation of `find` and may be invalidated
 * afterwards.
 */
class TrustAnchorContainer {
public:
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  TrustAnchorContainer() {}

  /**
   * Insert a static trust anchor. If the certificate (having the same name
   * without considering implicit digest) already exists in the group with
   * groupId, then do nothing.
   * @param groupId The certificate group id.
   * @param certificate The certificate to insert, which is copied.
   * @throws TrustAnchorContainer::Error If groupId is a dynamic anchor group .
   */
  void
  insert(const std::string& groupId, const CertificateV2& certificate);

  /**
   * Insert dynamic trust anchors from the path.
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
  insert
    (const std::string& groupId, const std::string& path,
     Milliseconds refreshPeriod, bool isDirectory = false);

  /**
   * Remove all static and dynamic anchors.
   */
  void
  clear()
  {
    groups_.clear();
    anchors_.clear();
  }

  /**
   * Search for a certificate across all groups (longest prefix match).
   * @param keyName The key name prefix for searching for the certificate.
   * @return The found certificate, or null if not found.
   */
  ptr_lib::shared_ptr<CertificateV2>
  find(const Name& keyName) const;

  /**
   * Find a certificate for the given interest.
   * @param interest The input interest packet.
   * @return The found certificate, or null if not found.
   * @note Interest with implicit digest is not supported.
   * @note ChildSelector is not supported.
   */
  ptr_lib::shared_ptr<CertificateV2>
  find(const Interest& interest) const;

  /**
   * Get the trust anchor group for the groupId.
   * @param groupId The group ID.
   * @return The trust anchor group.
   * @throws TrustAnchorContainer::Error if the groupId does not exist.
   */
  TrustAnchorGroup&
  getGroup(const std::string& groupId);

  /**
   * Get the number of trust anchors across all groups.
   * @return The number of trust anchors.
   */
  size_t
  size() const { return anchors_.size(); }

private:
  class AnchorContainer : public CertificateContainerInterface {
  public:
    virtual
    ~AnchorContainer();

    /**
     * Add the certificate to the container.
     * @param certificate The certificate to add, which is copied.
     */
    virtual void
    add(const CertificateV2& certificate);

    /**
     * Remove the certificate with the given name. If the name does not exist,
     * do nothing.
     * @param certificateName The name of the certificate.
     */
    virtual void
    remove(const Name& certificateName);

    /**
     * Clear all certificates.
     */
    void
    clear() { anchorsByName_.clear(); }

    /**
     * Get the number of certificates in the container.
     * @return The number of certificates.
     */
    size_t
    size() const { return anchorsByName_.size(); }

  private:
    friend class TrustAnchorContainer;

    std::map<Name, ptr_lib::shared_ptr<CertificateV2> > anchorsByName_;
  };

  void
  refresh();

  // Disable the copy constructor and assignment operator.
  TrustAnchorContainer(const TrustAnchorContainer& other);
  TrustAnchorContainer& operator=(const TrustAnchorContainer& other);

  std::map<std::string, ptr_lib::shared_ptr<TrustAnchorGroup> > groups_;
  AnchorContainer anchors_;
};

}

#endif
