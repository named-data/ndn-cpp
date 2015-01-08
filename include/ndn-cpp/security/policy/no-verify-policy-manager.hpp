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

#ifndef NDN_NO_VERIFY_POLICY_MANAGER_HPP
#define NDN_NO_VERIFY_POLICY_MANAGER_HPP

#include "policy-manager.hpp"

namespace ndn {

class NoVerifyPolicyManager : public PolicyManager {
public:
  /**
   * The virtual destructor.
   */
  virtual
  ~NoVerifyPolicyManager();

  /**
   * Override to always skip verification and trust as valid.
   * @param data The received data packet.
   * @return true.
   */
  virtual bool
  skipVerifyAndTrust(const Data& data);

  /**
   * Override to always skip verification and trust as valid.
   * @param interest The received interest.
   * @return true.
   */
  virtual bool
  skipVerifyAndTrust(const Interest& interest);

  /**
   * Override to return false for no verification rule for the received data.
   * @param data The received data packet.
   * @return false.
   */
  virtual bool
  requireVerify(const Data& data);

  /**
   * Override to return false for no verification rule for the received interest.
   * @param interest The received interest.
   * @return false.
   */
  virtual bool
  requireVerify(const Interest& interest);

  /**
   * Override to call onVerified(data) and to indicate no further verification step.
   * @param data The Data object with the signature (to ignore).
   * @param stepCount The number of verification steps that have been done, used to track the verification progress.
   * @param onVerified This does override to call onVerified(data).
   * @param onVerifyFailed Override to ignore this.
   * @return null for no further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Data>& data, int stepCount, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed);

  /**
   * Override to call onVerified(interest) and to indicate no further
   * verification step.
   * @param interest The interest with the signature (to ignore).
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param onVerified This does override to call onVerified(interest).
   * @param onVerifyFailed Override to ignore this.
   * @return null for no further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat);

  /**
   * Override to always indicate that the signing certificate name and data name satisfy the signing policy.
   * @param dataName The name of data to be signed.
   * @param certificateName The name of signing certificate.
   * @return true to indicate that the signing certificate can be used to sign the data.
   */
  virtual bool
  checkSigningPolicy(const Name& dataName, const Name& certificateName);

  /**
   * Override to indicate that the signing identity cannot be inferred.
   * @param dataName The name of data to be signed.
   * @return An empty name because cannot infer.
   */
  virtual Name
  inferSigningIdentity(const Name& dataName);
};

}

#endif
