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

#ifndef NDN_CERTIFICATE_SUBJECT_DESCRIPTION_HPP
#define NDN_CERTIFICATE_SUBJECT_DESCRIPTION_HPP

#include "../../common.hpp"
#include "../../encoding/oid.hpp"

namespace ndn {

class DerNode;

/**
 * A CertificateSubjectDescription represents the SubjectDescription entry in a Certificate.
 */
class CertificateSubjectDescription {
public:
  /**
   * Create a new CertificateSubjectDescription.
   * @param oid The oid of the subject description entry.
   * @param value The value of the subject description entry.
   */
  CertificateSubjectDescription(std::string oid, std::string value)
  : oid_(oid), value_(value)
  {
  }

  /**
   * Create a new CertificateSubjectDescription.
   * @param oid The oid of the subject description entry.
   * @param value The value of the subject description entry.
   */
  CertificateSubjectDescription(OID oid, std::string value)
  : oid_(oid), value_(value)
  {
  }

  /**
   * Encode the object into a DER syntax tree.
   * @return The encoded DER syntax tree.
   */
  ptr_lib::shared_ptr<DerNode>
  toDer() const;

  std::string
  getOidString() const
  {
    return oid_.toString();
  }

  const std::string &
  getValue() const
  {
    return value_;
  }

private:
  OID oid_;
  std::string value_;
};

}

#endif
