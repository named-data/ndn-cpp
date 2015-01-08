/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#ifndef NDN_CERTIFICATE_EXTENSION_HPP
#define NDN_CERTIFICATE_EXTENSION_HPP

#include "../../common.hpp"
#include "../../util/blob.hpp"
#include "../../encoding/oid.hpp"

namespace ndn {

class DerNode;

/**
 * A CertificateExtension represents the Extension entry in a certificate.
 */
class CertificateExtension
{
public:
  /**
   * Create a new CertificateExtension.
   * @param oid The oid of subject description entry.
   * @param isCritical If true, the extension must be handled.
   * @param value The extension value.
   */
  CertificateExtension(const std::string& oid, const bool isCritical, const Blob& value)
  : extensionId_(oid), isCritical_(isCritical), extensionValue_(value)
  {
  }

  /**
   * Create a new CertificateExtension.
   * @param oid The oid of subject description entry.
   * @param isCritical If true, the extension must be handled.
   * @param value The extension value.
   */
  CertificateExtension(const OID& oid, const bool isCritical, const Blob& value)
  : extensionId_(oid), isCritical_(isCritical), extensionValue_(value)
  {
  }

  /**
   * The virtual destructor.
   */
  virtual
  ~CertificateExtension() {}

  /**
   * encode the object into DER syntax tree
   * @return the encoded DER syntax tree
   */
  ptr_lib::shared_ptr<DerNode>
  toDer() const;

  Blob
  toDerBlob() const;

  const OID&
  getOid() const { return extensionId_; }

  bool
  getIsCritical() const { return isCritical_; }

  const Blob&
  getValue() const { return extensionValue_; }

protected:
  OID extensionId_;
  bool isCritical_;
  Blob extensionValue_;
};

}

#endif
