/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validation-state.hpp
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

#ifndef NDN_VALIDATION_STATE_HPP
#define NDN_VALIDATION_STATE_HPP

#include <set>
#include "validation-error.hpp"
#include "certificate-v2.hpp"
#include "../../interest.hpp"

namespace ndn {

/**
 * A DataValidationSuccessCallback function object is used to report a
 * successful Data validation.
 */
typedef func_lib::function<void(const Data& data)> DataValidationSuccessCallback;

/**
 * A DataValidationFailureCallback function object is used to report a failed
 * Data validation.
 */
typedef func_lib::function<void
  (const Data& data, const ValidationError& error)> DataValidationFailureCallback;

/**
 * An InterestValidationSuccessCallback function object is used to report a
 * successful Interest validation.
 */
typedef func_lib::function<void
  (const Interest& interest)> InterestValidationSuccessCallback;

/**
 * An InterestValidationFailureCallback function object is used to report a
 * failed Interest validation.
 */
typedef func_lib::function<void
  (const Interest& interest, const ValidationError& error)>
  InterestValidationFailureCallback;

/**
 * ValidationState is an abstract base class for DataValidationState and
 * InterestValidationState.
 *
 * One instance of the validation state is kept for the validation of the whole
 * certificate chain.
 *
 * The state collects the certificate chain that adheres to the selected
 * validation policy to validate data or interest packets. Certificate, data,
 * and interest packet signatures are verified only after the validator
 * determines that the chain terminates with a trusted certificate (a trusted
 * anchor or a previously validated certificate). This model allows filtering
 * out invalid certificate chains without incurring (costly) cryptographic
 * signature verification overhead and mitigates some forms of denial-of-service
 * attacks.
 *
 * A validation policy and/or key fetcher may add custom information associated
 * with the validation state using tags.
 */
class ValidationState {
public:
  ValidationState()
  : hasOutcome_(false)
  {
  }

  virtual
  ~ValidationState();

  /**
   * Check if validation failed or success has been called.
   * @return True if validation failed or success has been called.
   */
  bool
  hasOutcome() { return hasOutcome_; }

  /**
   * Check if validation failed has been called.
   * @return True if validation failed has been called, false if no validation
   * callbacks have been called or validation success was called.
   */
  bool
  isOutcomeFailed() { return hasOutcome_ && outcome_ == false; }

  /**
   * Check if validation success has been called.
   * @return True if validation success has been called, false if no validation
   * callbacks have been called or validation failed was called.
   */
  bool
  isOutcomeSuccess() { return hasOutcome_ && outcome_ == true; }

  /**
   * Call the failure callback.
   */
  virtual void
  fail(const ValidationError& error) = 0;

  /**
   * Get the depth of the certificate chain.
   * @return The depth of the certificate chain.
   */
  size_t
  getDepth() const { return certificateChain_.size(); }

  /**
   * Check if certificateName has been previously seen, and record the supplied
   * name.
   * @param certificateName The certificate name, which is copied.
   * @return True if certificateName has been previously seen.
   */
  bool
  hasSeenCertificateName(const Name& certificateName)
  {
    // second is false if certificateName was already in the set.
    return !seenCertificateNames_.insert(certificateName).second;
  }

  /**
   * Add the certificate to the top of the certificate chain.
   * If the certificate chain is empty, then the certificate should be the
   * signer of the original packet. If the certificate chain is not empty, then
   * the certificate should be the signer of the front of the certificate chain.
   * @note This function does not verify the signature bits.
   * @param certificate The certificate to add, which is copied.
   */
  void
  addCertificate(const CertificateV2& certificate)
  {
    certificateChain_.insert
      (certificateChain_.begin(),
       ptr_lib::make_shared<CertificateV2>(certificate));
  }

protected:
  /**
   * Set the outcome to the given value, and set hasOutcome_ true.
   * @param outcome The outcome.
   * @throws std::runtime_error If this ValidationState already has an outcome.
   */
  void
  setOutcome(bool outcome);

private:
  friend class Validator;

  /**
   * Verify the signature of the original packet. This is only called by the
   * Validator class.
   * @param trustedCertificate The certificate that signs the original packet.
   */
  virtual void
  verifyOriginalPacket(const CertificateV2& trustedCertificate) = 0;

  /**
   * Call the success callback of the original packet without signature
   * validation. This is only called by the Validator class.
   */
  virtual void
  bypassValidation() = 0;

  /**
   * Verify signatures of certificates in the certificate chain. On return, the
   * certificate chain contains a list of certificates successfully verified by
   * trustedCertificate.
   * When the certificate chain cannot be verified, this method will call
   * fail() with the INVALID_SIGNATURE error code and the appropriate message.
   * This is only called by the Validator class.
   * @return The certificate to validate the original data packet, either the
   * last entry in the certificate chain or trustedCertificate if the
   * certificate chain is empty. However, return null if the signature of at
   * least one certificate in the chain is invalid, in which case all unverified
   * certificates have been removed from the certificate chain.
   */
  ptr_lib::shared_ptr<CertificateV2>
  verifyCertificateChain
    (const ptr_lib::shared_ptr<CertificateV2>& trustedCertificate);

  // Disable the copy constructor and assignment operator.
  ValidationState(const ValidationState& other);
  ValidationState& operator=(const ValidationState& other);

  /**
   * Each certificate in the chain signs the next certificate. The last
   * certificate signs the original packet.
   */
  std::vector<ptr_lib::shared_ptr<CertificateV2> > certificateChain_;
  std::set<Name> seenCertificateNames_;
  bool hasOutcome_;
  bool outcome_;
};

/**
 * The DataValidationState class extends ValidationState to hold the validation
 * state for a Data packet.
 */
class DataValidationState : public ValidationState {
public:
  /**
   * Create a DataValidationState for the Data packet.
   * The caller must ensure that the state instance is valid until the validation
   * finishes (i.e., until validateCertificateChain() and
   * validateOriginalPacket() have been called).
   * @param data The Data packet being validated, which is copied.
   */
  DataValidationState
    (const Data& data, const DataValidationSuccessCallback& successCallback,
     const DataValidationFailureCallback& failureCallback);

  // TODO: Checks in ~DataValidationState.

  virtual void
  fail(const ValidationError& error);

  /**
   * Get the original Data packet being validated which was given to the
   * constructor.
   * @return The original Data packet.
   */
  const Data&
  getOriginalData() const { return data_; }

private:
  virtual void
  verifyOriginalPacket(const CertificateV2& trustedCertificate);

  virtual void
  bypassValidation();

  Data data_;
  DataValidationSuccessCallback successCallback_;
  DataValidationFailureCallback failureCallback_;
};

/**
 * The InterestValidationState class extends ValidationState to hold the
 * validation state for an Interest packet.
 */
class InterestValidationState : public ValidationState {
public:
  /**
   * Create an InterestValidationState for the Data packet.
   * The caller must ensure that the state instance is valid until the validation
   * finishes (i.e., until validateCertificateChain() and
   * validateOriginalPacket() have been called).
   * @param interest The Interest packet being validated, which is copied.
   * @param successCallback This calls successCallback(interest) to report a
   * successful Interest validation.
   * @param failureCallback This calls failureCallback(interest, error) to
   * report a failed Interest validation, where error is a ValidationError.
   */
  InterestValidationState
    (const Interest& interest,
     const InterestValidationSuccessCallback& successCallback,
     const InterestValidationFailureCallback& failureCallback);

  // TODO: Checks in ~InterestValidationState.

  virtual void
  fail(const ValidationError& error);

  /**
   * Get the original Interest packet being validated which was given to the
   * constructor.
   * @return The original Interest packet.
   */
  const Interest&
  getOriginalInterest() const { return interest_; }

  void
  addSuccessCallback(const InterestValidationSuccessCallback& successCallback)
  {
    successCallbacks_.push_back(successCallback);
  }

private:
  virtual void
  verifyOriginalPacket(const CertificateV2& trustedCertificate);

  virtual void
  bypassValidation();

  Interest interest_;
  std::vector<InterestValidationSuccessCallback> successCallbacks_;
  InterestValidationFailureCallback failureCallback_;
};

}

#endif
