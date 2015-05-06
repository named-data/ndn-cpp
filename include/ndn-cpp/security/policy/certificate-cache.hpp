/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN certificate_cache.py by Adeola Bannis.
 * Originally from Yingdi Yu <http://irl.cs.ucla.edu/~yingdi/>.
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

#ifndef NDN_CERTIFICATE_CACHE_HPP
#define NDN_CERTIFICATE_CACHE_HPP

#include <map>
#include "../certificate/identity-certificate.hpp"

namespace ndn {

/**
 * A CertificateCache is used to save other users' certificate during
 * verification.
 */
class CertificateCache {
public:
  /**
   * Insert the certificate into the cache. Assumes the timestamp is not yet
   * removed from the name.
   * @param certificate The certificate to copy and insert.
   */
  void
  insertCertificate(const IdentityCertificate& certificate)
  {
    Name certName = certificate.getName().getPrefix(-1);
    cache_[certName.toUri()] = certificate.wireEncode();
  }

  /**
   * Remove a certificate from the cache. This does nothing if it is not present.
   * @param certificateName The name of the certificate to remove. This assumes
   * there is no timestamp in the name.
   */
  void
  deleteCertificate(const Name& certificateName)
  {
    std::map<std::string, Blob>::iterator entry
      (cache_.find(certificateName.toUri()));
    if (entry != cache_.end())
      cache_.erase(entry);
  }

  /**
   * Fetch a certificate from the cache.
   * @param certificateName The name of the certificate to remove. Assumes there
   * is no timestamp in the name.
   * @return A new copy of the IdentityCertificate, or a null shared_ptr if not
   * found.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getCertificate(const Name& certificateName) const
  {
    std::map<std::string, Blob>::const_iterator entry
      (cache_.find(certificateName.toUri()));
    if (entry == cache_.end())
      return ptr_lib::shared_ptr<IdentityCertificate>();

    ptr_lib::shared_ptr<IdentityCertificate> cert(new IdentityCertificate());
    Blob certData = entry->second;
    cert->wireDecode(certData);
    return cert;
  }

  /**
   * Clear all certificates from the store.
   */
  void
  reset()
  {
    cache_.clear();
  }

private:
  // The key is the certificate name URI. The value is the wire encoding.
  std::map<std::string, Blob> cache_;
};

}

#endif
