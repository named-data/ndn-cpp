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

#ifndef NDN_POLICY_MANAGER_HPP
#define NDN_POLICY_MANAGER_HPP

#include "../../data.hpp"
#include "../key-chain.hpp"

namespace ndn {

class ValidationRequest;

/**
 * A PolicyManager is an abstract base class to represent the policy for verifying data packets.
 * You must create an object of a subclass.
 */
class PolicyManager {
public:
  /**
   * The virtual destructor.
   */
  virtual
  ~PolicyManager() {}

  /**
   * Check if the received data packet can escape from verification and be trusted as valid.
   * @param data The received data packet.
   * @return true if the data does not need to be verified to be trusted as valid, otherwise false.
   */
  virtual bool
  skipVerifyAndTrust(const Data& data) = 0;

  /**
   * Check if the received signed interest can escape from verification and be
   * trusted as valid.
   * @param interest The received interest.
   * @return true if the interest does not need to be verified to be trusted as valid, otherwise false.
   */
  virtual bool
  skipVerifyAndTrust(const Interest& interest) = 0;

  /**
   * Check if this PolicyManager has a verification rule for the received data.
   * @param data The received data packet.
   * @return true if the data must be verified, otherwise false.
   */
  virtual bool
  requireVerify(const Data& data) = 0;

  /**
   * Check if this PolicyManager has a verification rule for the received signed
   * interest.
   * @param interest The received interest.
   * @return true if the interest must be verified, otherwise false.
   */
  virtual bool
  requireVerify(const Interest& interest) = 0;

  /**
   * Check whether the received data packet complies with the verification policy, and get the indication of the next verification step.
   * @param data The Data object with the signature to check.
   * @param stepCount The number of verification steps that have been done, used to track the verification progress.
   * @param onVerified If the signature is verified, this calls onVerified(data).
   * @param onVerifyFailed If the signature check fails, this calls onVerifyFailed(data).
   * @return the indication of next verification step, null if there is no further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Data>& data, int stepCount, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed) = 0;

  /**
   * Check whether the received signed interest complies with the verification
   * policy, and get the indication of the next verification step.
   * @param interest The interest with the signature to check.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param onVerified If the signature is verified, this calls onVerified(interest).
   * @param onVerifyFailed If the signature check fails, this calls
   * onVerifyFailed(interest).
   * @return the indication of next verification step, null if there is no
   * further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat) = 0;

  /**
   * Check if the signing certificate name and data name satisfy the signing policy.
   * @param dataName The name of data to be signed.
   * @param certificateName The name of signing certificate.
   * @return true if the signing certificate can be used to sign the data, otherwise false.
   */
  virtual bool
  checkSigningPolicy(const Name& dataName, const Name& certificateName) = 0;

  /**
   * Infer the signing identity name according to the policy. If the signing identity cannot be inferred, return an empty name.
   * @param dataName The name of data to be signed.
   * @return The signing identity or an empty name if cannot infer.
   */
  virtual Name
  inferSigningIdentity(const Name& dataName) = 0;

protected:
  /**
   * Check the type of signature and use the publicKeyDer to verify the
   * signedBlob using the appropriate signature algorithm.
   * @param signature An object of a subclass of Signature, e.g.
   * Sha256WithRsaSignature.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @param publicKeyDer The DER-encoded public key used to verify the signature.
   * This may be null if the signature type does not require a public key.
   * @return True if the signature is verified, false if failed.
   * @throws SecurityException if the signature type is not recognized or if
   * publicKeyDer can't be decoded.
   */
  static bool
  verifySignature
    (const Signature* signature, const SignedBlob& signedBlob,
     const Blob& publicKeyDer);

  /**
   * Verify the ECDSA signature on the SignedBlob using the given public key.
   * @param signature The signature bits.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @param publicKeyDer The DER-encoded public key used to verify the signature.
   * @return true if the signature verifies, false if not.
   * @throws UnrecognizedKeyFormatException if publicKeyDer can't be decoded as
   * an ECDSA public key.
   */
  static bool
  verifySha256WithEcdsaSignature
    (const Blob& signature, const SignedBlob& signedBlob,
     const Blob& publicKeyDer);

  /**
   * Verify the RSA signature on the SignedBlob using the given public key.
   * @param signature The signature bits.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @param publicKeyDer The DER-encoded public key used to verify the signature.
   * @return true if the signature verifies, false if not.
   * @throws UnrecognizedKeyFormatException if publicKeyDer can't be decoded as
   * an RSA public key.
   */
  static bool
  verifySha256WithRsaSignature
    (const Blob& signature, const SignedBlob& signedBlob,
     const Blob& publicKeyDer);

  /**
   * Verify the DigestSha256 signature on the SignedBlob by verifying that the
   * digest of SignedBlob equals the signature.
   * @param signature The signature bits.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @return true if the signature verifies, false if not.
   */
  static bool
  verifyDigestSha256Signature
    (const Blob& signature, const SignedBlob& signedBlob);
};

}

#endif
