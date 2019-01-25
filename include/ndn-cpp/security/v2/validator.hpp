/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validator.hpp
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

#ifndef NDN_VALIDATOR_HPP
#define NDN_VALIDATOR_HPP

#include "certificate-fetcher-offline.hpp"
#include "validation-policy.hpp"

namespace ndn {

/**
 * The Validator class provides an interface for validating data and interest
 * packets.
 *
 * Every time a validation process is initiated, it creates a ValidationState
 * that exists until the validation finishes with either success or failure.
 * This state serves several purposes:
 * to record the Interest or Data packet being validated,
 * to record the failure callback,
 * to record certificates in the certification chain for the Interest or Data
 * packet being validated,
 * to record the names of the requested certificates in order to detect loops in
 * the certificate chain,
 * and to keep track of the validation chain size (also known as the validation
 * "depth").
 *
 * During validation, the policy and/or key fetcher can augment the validation
 * state with policy- and fetcher-specific information using tags.
 *
 * A Validator has a trust anchor cache to save static and dynamic trust
 * anchors, a verified certificate cache for saving certificates that are
 * already verified, and an unverified certificate cache for saving pre-fetched
 * but not yet verified certificates.
 */
class Validator : public CertificateStorage {
public:
  /**
   * Create a Validator with the policy and fetcher.
   * @param policy The validation policy to be associated with this validator.
   * @param certificateFetcher (optional) The certificate fetcher implementation.
   * If omitted, use a CertificateFetcherOffline (assuming that the validation
   * policy doesn't need to fetch certificates).
   */
  Validator
    (const ptr_lib::shared_ptr<ValidationPolicy>& policy,
     const ptr_lib::shared_ptr<CertificateFetcher>& certificateFetcher =
       ptr_lib::make_shared<CertificateFetcherOffline>());

  /**
   * Get the ValidationPolicy given to the constructor.
   * @return The ValidationPolicy.
   */
  ValidationPolicy&
  getPolicy() { return *policy_; }

  /**
   * Get the CertificateFetcher given to (or created in) the constructor.
   * @return The CertificateFetcher.
   */
  CertificateFetcher&
  getFetcher() { return *certificateFetcher_; }

  /**
   * Set the maximum depth of the certificate chain.
   * @param maxDepth The maximum depth.
   */
  void
  setMaxDepth(size_t maxDepth) { maxDepth_ = maxDepth; }

  /**
   * Get the maximum depth of the certificate chain.
   * @return The maximum depth.
   */
  size_t
  getMaxDepth() const { return maxDepth_; }

  /**
   * Asynchronously validate the Data packet.
   * @param data The Data packet to validate, which is copied.
   * @param successCallback On validation success, this calls
   * successCallback(data).
   * @param failureCallback On validation failure, this calls
   * failureCallback(data, error) where error is a ValidationError.
   */
  void
  validate
    (const Data& data, const DataValidationSuccessCallback& successCallback,
     const DataValidationFailureCallback& failureCallback);

  /**
   * Asynchronously validate the Interest.
   * @param interest The Interest to validate, which is copied.
   * @param successCallback On validation success, this calls
   * successCallback(interest).
   * @param failureCallback On validation failure, this calls
   * failureCallback(interest, error) where error is a ValidationError.
   */
  void
  validate
    (const Interest& interest,
     const InterestValidationSuccessCallback& successCallback,
     const InterestValidationFailureCallback& failureCallback);

private:
  /**
   * Recursively validate the certificates in the certification chain.
   * @param certificate The certificate to check.
   * @param state The current validation state.
   */
  void
  validateCertificate
    (const ptr_lib::shared_ptr<CertificateV2>& certificate,
     const ptr_lib::shared_ptr<ValidationState>& state);

  /**
   * Request a certificate for further validation.
   * @param certificateRequest The certificate request.
   * @param state The current validation state.
   */
  void
  requestCertificate
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state);

  /**
   * This is the continuation for validateCertificate.
   */
  void
  continueValidateCertificate
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state,
     const ptr_lib::shared_ptr<CertificateV2>& certificate)
  {
    if (!certificateRequest)
      state->fail(ValidationError
        (ValidationError::POLICY_ERROR,
         "Validation policy is not allowed to designate `" +
         certificate->getName().toUri() + "` as a trust anchor"));
    else {
      // We need to fetch the key and validate it.
      state->addCertificate(*certificate);
      requestCertificate(certificateRequest, state);
    }
  }

  /**
   * This is the continuation for validate.
   */
  void
  continueValidate
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state)
  {
    if (!certificateRequest)
      state->bypassValidation();
    else
      // We need to fetch the key and validate it.
      requestCertificate(certificateRequest, state);
  }

  ptr_lib::shared_ptr<ValidationPolicy> policy_;
  ptr_lib::shared_ptr<CertificateFetcher> certificateFetcher_;
  size_t maxDepth_;
};

}

#endif
