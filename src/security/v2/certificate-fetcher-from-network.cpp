/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/certificate-fetcher-from-network.cpp
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
#include <ndn-cpp/security/v2/certificate-fetcher-from-network.hpp>

INIT_LOGGER("ndn.Validator");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

void
CertificateFetcherFromNetwork::Impl::doFetch
  (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  face_.expressInterest
    (certificateRequest->interest_,
     bind(&CertificateFetcherFromNetwork::Impl::onData, shared_from_this(),
          _1, _2, state, continueValidation),
     bind(&CertificateFetcherFromNetwork::Impl::onTimeout, shared_from_this(),
          _1, certificateRequest, state, continueValidation),
     bind(&CertificateFetcherFromNetwork::Impl::onNetworkNack, shared_from_this(),
          _1, _2, certificateRequest, state, continueValidation));
}

void
CertificateFetcherFromNetwork::Impl::onData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const CertificateFetcher::ValidationContinuation& continueValidation)
{
  _LOG_TRACE("Fetched certificate from network " << data->getName().toUri());

  ptr_lib::shared_ptr<CertificateV2> certificate;
  try {
    certificate.reset(new CertificateV2(*data));
  } catch (const std::exception& ex) {
    state->fail(ValidationError(ValidationError::MALFORMED_CERTIFICATE,
      "Fetched a malformed certificate `" + data->getName().toUri() +
      "` (" + ex.what() + ")"));
    return;
  }

  continueValidation(certificate, state);
}

void
CertificateFetcherFromNetwork::Impl::onNetworkNack
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<NetworkNack>& networkNack,
   const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const CertificateFetcher::ValidationContinuation& continueValidation)
{
  _LOG_TRACE("NACK (" << networkNack->getReason() <<  
    ") while fetching certificate " << 
    certificateRequest->interest_.getName().toUri());

  --certificateRequest->nRetriesLeft_;
  if (certificateRequest->nRetriesLeft_ >= 0)
    parent_.fetch(certificateRequest, state, continueValidation);
  else
    state->fail(ValidationError(ValidationError::CANNOT_RETRIEVE_CERTIFICATE,
      "Cannot fetch certificate after all retries `" +
      certificateRequest->interest_.getName().toUri() + "`"));
}

void
CertificateFetcherFromNetwork::Impl::onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const CertificateFetcher::ValidationContinuation& continueValidation)
{
  _LOG_TRACE("Timeout while fetching certificate " <<
    certificateRequest->interest_.getName().toUri() << ", retrying");

  --certificateRequest->nRetriesLeft_;
  if (certificateRequest->nRetriesLeft_ >= 0)
    parent_.fetch(certificateRequest, state, continueValidation);
  else
    state->fail(ValidationError(ValidationError::CANNOT_RETRIEVE_CERTIFICATE,
      "Cannot fetch certificate after all retries `" +
      certificateRequest->interest_.getName().toUri() + "`"));
}

}
