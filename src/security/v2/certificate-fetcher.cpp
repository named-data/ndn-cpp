/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/certificate-fetcher.cpp
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

#include <stdexcept>
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/security/v2/certificate-fetcher.hpp>

INIT_LOGGER("ndn.CertificateFetcher");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

CertificateFetcher::~CertificateFetcher() {}

void
CertificateFetcher::setCertificateStorage(CertificateStorage& certificateStorage)
{
  certificateStorage_ = &certificateStorage;
}

void
CertificateFetcher::fetch
  (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  if (!certificateStorage_)
    throw runtime_error
      ("CertificateFetcher.fetch: You must first call setCertificateStorage");

  ptr_lib::shared_ptr<CertificateV2> certificate =
    certificateStorage_->getUnverifiedCertificateCache().find
      (certificateRequest->interest_);
  if (certificate) {
    _LOG_TRACE("Found certificate in **un**verified key cache " <<
               certificate->getName());
    continueValidation(certificate, state);
    return;
  }

  // Fetch asynchronously.
  doFetch
    (certificateRequest, state,
     bind(&CertificateFetcher::continueFetch, this, _1, _2, continueValidation));
}

}
