/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validator.cpp
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
#include <ndn-cpp/security/v2/validator.hpp>

INIT_LOGGER("ndn.Validator");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

Validator::Validator
  (const ptr_lib::shared_ptr<ValidationPolicy>& policy,
   const ptr_lib::shared_ptr<CertificateFetcher>& certificateFetcher)
: policy_(policy),
  certificateFetcher_(certificateFetcher),
  maxDepth_(25)
{
  if (!policy_)
    throw invalid_argument("The policy is null");
  if (!certificateFetcher_)
    throw invalid_argument("The certificateFetcher is null");

  policy_->setValidator(*this);
  certificateFetcher_->setCertificateStorage(*this);
}

void
Validator::validate
  (const Data& data,
   const DataValidationSuccessCallback& successCallback,
   const DataValidationFailureCallback& failureCallback)
{
  ptr_lib::shared_ptr<DataValidationState> state
    (new DataValidationState(data, successCallback, failureCallback));
  _LOG_TRACE("Start validating data " << data.getName());

  policy_->checkPolicy
    (data, state, bind(&Validator::continueValidate, this, _1, _2));
}

void
Validator::validate
  (const Interest& interest,
   const InterestValidationSuccessCallback& successCallback,
   const InterestValidationFailureCallback& failureCallback)
{
  ptr_lib::shared_ptr<InterestValidationState> state
    (new InterestValidationState(interest, successCallback, failureCallback));
  _LOG_TRACE("Start validating interest " << interest.getName());

  policy_->checkPolicy
    (interest, state, bind(&Validator::continueValidate, this, _1, _2));
}

void
Validator::validateCertificate
  (const ptr_lib::shared_ptr<CertificateV2>& certificate,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  _LOG_TRACE("Start validating certificate " << certificate->getName());

  if (!certificate->isValid()) {
    state->fail(ValidationError
      (ValidationError::EXPIRED_CERTIFICATE,
       "Retrieved certificate is not yet valid or expired `" +
       certificate->getName().toUri() + "`"));
    return;
  }

  policy_->checkCertificatePolicy
    (*certificate, state,
     bind(&Validator::continueValidateCertificate, this, _1, _2, certificate));
}

void
Validator::requestCertificate
  (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  if (state->getDepth() >= maxDepth_) {
    state->fail(ValidationError
      (ValidationError::EXCEEDED_DEPTH_LIMIT, "Exceeded validation depth limit"));
    return;
  }

  if (state->hasSeenCertificateName(certificateRequest->interest_.getName())) {
    state->fail(ValidationError
      (ValidationError::LOOP_DETECTED,
       "Validation loop detected for certificate `" + 
         certificateRequest->interest_.getName().toUri() + "`"));
    return;
  }

  _LOG_TRACE("Retrieving " << certificateRequest->interest_.getName());

  ptr_lib::shared_ptr<CertificateV2> certificate = findTrustedCertificate
    (certificateRequest->interest_);
  if (certificate) {
    _LOG_TRACE("Found trusted certificate " << certificate->getName());

    certificate = state->verifyCertificateChain(certificate);
    if (certificate)
      state->verifyOriginalPacket(*certificate);

    for (size_t i = 0; i < state->certificateChain_.size(); ++i)
      cacheVerifiedCertificate(*state->certificateChain_[i]);

    return;
  }

  certificateFetcher_->fetch
    (certificateRequest, state,
     bind(&Validator::validateCertificate, this, _1, _2));
}

}
