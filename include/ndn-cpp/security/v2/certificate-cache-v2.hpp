/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/certificate-cache.hpp
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

#ifndef NDN_CERTIFICATE_CACHE_V2_HPP
#define NDN_CERTIFICATE_CACHE_V2_HPP

#include <map>
#include "../../interest.hpp"
#include "certificate-v2.hpp"

namespace ndn {

/**
 * A CertificateCacheV2 holds other user's verified certificates in security v2
 * format CertificateV2. A certificate is removed no later than its NotAfter
 * time, or maxLifetime after it has been added to the cache.
 */
class CertificateCacheV2 {
public:
  /**
   * Create a CertificateCacheV2.
   * @param maxLifetimeMilliseconds (optional) The maximum time that
   * certificates can live inside the cache, in milliseconds. If omitted, use
   * getDefaultLifetime().
   */
  CertificateCacheV2(Milliseconds maxLifetimeMilliseconds = getDefaultLifetime());

  /**
   * Insert the certificate into the cache. The inserted certificate will be
   * removed no later than its NotAfter time, or maxLifetimeMilliseconds given
   * to the constructor.
   * @param certificate The certificate object, which is copied.
   */
  void
  insert(const CertificateV2& certificate);

  /**
   * Find the certificate by the given key name.
   * @param certificatePrefix The certificate prefix for searching for the
   * certificate.
   * @return The found certificate, or null if not found. You must not modify
   * the returned object. If you need to modify it, then make a copy.
   */
  ptr_lib::shared_ptr<CertificateV2>
  find(const Name& certificatePrefix) const;

  /**
   * Find the certificate by the given interest.
   * @param interest The input interest object.
   * @return The found certificate which matches the interest, or null if not
   * found. You must not modify the returned object. If you need to modify it,
   * then make a copy.
   * @note ChildSelector is not supported.
   */
  ptr_lib::shared_ptr<CertificateV2>
  find(const Interest& interest) const;

  /**
   * Remove the certificate whose name equals the given name. If no such
   * certificate is in the cache, do nothing.
   * @param certificateName The name of the certificate.
   */
  void
  deleteCertificate(const Name& certificateName);

  /**
   * Clear all certificates from the cache.
   */
  void
  clear()
  {
    certificatesByName_.clear();
    // TODO: certificatesByTime_.clear();
  }

  /**
   * Get the default maximum lifetime (1 hour).
   * @return The lifetime in milliseconds.
   */
  static Milliseconds
  getDefaultLifetime() { return 3600.0 * 1000; }

private:
  /**
   * Remove all outdated certificate entries.
   */
  void
  refresh();

  // Disable the copy constructor and assignment operator.
  CertificateCacheV2(const CertificateCacheV2& other);
  CertificateCacheV2& operator=(const CertificateCacheV2& other);

  std::map<Name, ptr_lib::shared_ptr<CertificateV2> > certificatesByName_;
  Milliseconds maxLifetimeMilliseconds_;
};

}

#endif
