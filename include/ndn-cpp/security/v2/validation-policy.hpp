/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validation-policy.hpp
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

#ifndef NDN_VALIDATION_POLICY_HPP
#define NDN_VALIDATION_POLICY_HPP

#include "validation-state.hpp"
#include "certificate-request.hpp"

namespace ndn {

class Validator;

/**
 * ValidationPolicy is an abstract base class that implements a validation
 * policy for Data and Interest packets.
 */
class ValidationPolicy {
public:
  typedef func_lib::function<void
    (const ptr_lib::shared_ptr<CertificateRequest>& certificateRequest,
     const ptr_lib::shared_ptr<ValidationState>& state)> ValidationContinuation;

  ValidationPolicy()
  : validator_(0)
  {
  }

  virtual
  ~ValidationPolicy();

  /**
   * Set the inner policy.
   * Multiple assignments of the inner policy will create a "chain" of linked
   * policies.The inner policy from the latest invocation of setInnerPolicy will
   * be at the bottom of the policy list.
   * For example, the sequence `this->setInnerPolicy(policy1)` and
   * `this->setInnerPolicy(policy2)`, will result in
   * `this->m_innerPolicy == policy1`,
   * this->m_innerPolicy->m_innerPolicy == policy2', and
   * `this->m_innerPolicy->m_innerPolicy->m_innerPolicy == null`.
   * @throw invalid_argument exception, if the innerPolicy is null.
   */
  void
  setInnerPolicy(ptr_lib::shared_ptr<ValidationPolicy> innerPolicy);

  /**
   * Check if the inner policy is set.
   * @return True if the inner policy is set.
   */
  bool
  hasInnerPolicy() const { return !!innerPolicy_; }

  /**
   * Get the inner policy. If the inner policy was not set, the behavior is
   * undefined.
   * @return The inner policy.
   */
  ValidationPolicy&
  getInnerPolicy() { return *innerPolicy_; }

  /**
   * Set the validator to which this policy is associated. This replaces any
   * previous validator.
   * @param validator The validator.
   */
  void
  setValidator(Validator& validator)
  {
    validator_ = &validator;
    if (innerPolicy_)
      innerPolicy_->setValidator(validator);
  }

  /**
   * Check the Data packet against the policy.
   * Your derived class must implement this.
   * Depending on the implementation of the policy, this check can be done
   * synchronously or asynchronously.
   * The semantics of checkPolicy are as follows:
   * If the packet violates the policy, then the policy should call
   * state.fail() with an appropriate error code and error description.
   * If the packet conforms to the policy and no further key retrievals are
   * necessary, then the policy should call continueValidation(state, nullptr).
   * If the packet conforms to the policy and a key needs to be fetched, then
   * the policy should call
   * continueValidation(state, <appropriate-key-request-instance>).
   */
  virtual void
  checkPolicy
    (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation) = 0;

  /**
   * Check the Interest against the policy.
   * Your derived class must implement this.
   * Depending on implementation of the policy, this check can be done 
   * synchronously or asynchronously.
   * See the checkPolicy(Data) documentation for the semantics.
   */
  virtual void
  checkPolicy
    (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation) = 0;

  /**
   * Check the certificate against the policy.
   * This base class implementation just calls checkPolicy(Data, ...). Your
   * derived class may override.
   * Depending on implementation of the policy, this check can be done
   * synchronously or asynchronously.
   * See the checkPolicy(Data) documentation for the semantics.
   */
  virtual void
  checkCertificatePolicy
    (const CertificateV2& certificate,
     const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

private:
  // Disable the copy constructor and assignment operator.
  ValidationPolicy(const ValidationPolicy& other);
  ValidationPolicy& operator=(const ValidationPolicy& other);

protected:
  Validator* validator_;
  ptr_lib::shared_ptr<ValidationPolicy> innerPolicy_;
};

}

#endif
