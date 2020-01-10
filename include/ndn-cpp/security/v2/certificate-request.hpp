/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/certificate-request.hpp
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

#ifndef NDN_CERTIFICATE_REQUEST_HPP
#define NDN_CERTIFICATE_REQUEST_HPP

#include "../../interest.hpp"

namespace ndn {

/**
 * A CertificateRequest represents a request for a certificate, associated with
 * the number of retries left. The interest_ and nRetriesLeft_ fields are
 * public so that you can modify them.
 */
class CertificateRequest {
public:
  /**
   * Create a CertificateRequest with a default Interest and 0 retries left.
   */
  CertificateRequest()
  : nRetriesLeft_(0)
  {
  }

  /**
   * Create  a CertificateRequest for the Interest and 3 retries left.
   * @param interest The Interest which is copied.
   */
  CertificateRequest(const Interest& interest)
    // Copy the Interest.
  : interest_(interest),
    nRetriesLeft_(3)
  {
  }

  /** The Interest for the requested Data packet or Certificate.
   */
  Interest interest_;
  /** The number of remaining retries after time out or NACK.
   */
  int nRetriesLeft_;

private:
  // Disable the copy constructor and assignment operator.
  CertificateRequest(const CertificateRequest& other);
  CertificateRequest& operator=(const CertificateRequest& other);
};

}

#endif
