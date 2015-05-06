/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_SELF_VERIFY_POLICY_MANAGER_HPP
#define NDN_SELF_VERIFY_POLICY_MANAGER_HPP

#include "policy-manager.hpp"

namespace ndn {

class IdentityStorage;

/**
 * A SelfVerifyPolicyManager implements a PolicyManager to use the public key DER in the data packet's KeyLocator (if available)
 * or look in the IdentityStorage for the public key with the name in the KeyLocator (if available) and use
 * it to verify the data packet, without searching a certificate chain.  If the public key can't be found, the
 * verification fails.
 */
class SelfVerifyPolicyManager : public PolicyManager {
public:
  /**
   * Create a new SelfVerifyPolicyManager which will look up the public key in
   * the given identityStorage.
   * @param identityStorage (optional) The IdentityStorage for looking up the
   * public key.  This points to an object which must remain valid during the
   * life of this SelfVerifyPolicyManager. If omitted, then don't look for a
   * public key with the name in the KeyLocator and rely on the KeyLocator
   * having the full public key DER.
   */
  SelfVerifyPolicyManager(IdentityStorage* identityStorage = 0)
  : identityStorage_(identityStorage)
  {
  }

  /**
   * The virtual destructor.
   */
  virtual
  ~SelfVerifyPolicyManager();

  /**
   * Never skip verification.
   * @param data The received data packet.
   * @return false.
   */
  virtual bool
  skipVerifyAndTrust(const Data& data);

  /**
   * Never skip verification.
   * @param interest The received interest.
   * @return false.
   */
  virtual bool
  skipVerifyAndTrust(const Interest& interest);

  /**
   * Always return true to use the self-verification rule for the received data.
   * @param data The received data packet.
   * @return true.
   */
  virtual bool
  requireVerify(const Data& data);

  /**
   * Always return true to use the self-verification rule for the received
   * signed interest.
   * @param interest The received interest.
   * @return true.
   */
  virtual bool
  requireVerify(const Interest& interest);

  /**
   * Use the public key DER in the data packet's KeyLocator (if available) or look in the IdentityStorage for the
   * public key with the name in the KeyLocator (if available) and use it to verify the data packet.  If the public key can't
   * be found, call onVerifyFailed.
   * @param data The Data object with the signature to check.
   * @param stepCount The number of verification steps that have been done, used to track the verification progress.
   * (stepCount is ignored.)
   * @param onVerified If the signature is verified, this calls onVerified(data).
   * @param onVerifyFailed If the signature check fails or can't find the public key, this calls onVerifyFailed(data).
   * @return null for no further step for looking up a certificate chain.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Data>& data, int stepCount, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed);

  /**
   * Use wireFormat.decodeSignatureInfoAndValue to decode the last two name
   * components of the signed interest. Use the public key DER in the signed
   * interest SignatureInfo's KeyLocator (if available) or look in the
   * IdentityStorage for the public key with the name in the KeyLocator
   * (if available) and use it to verify the interest. If the public key can't
   * be found, call onVerifyFailed.
   * @param interest The interest with the signature to check.
   * @param stepCount The number of verification steps that have been done, used to track the verification progress.
   * (stepCount is ignored.)
   * @param onVerified If the signature is verified, this calls onVerified(interest).
   * @param onVerifyFailed If the signature check fails or can't find the public key, this calls onVerifyFailed(interest).
   * @return null for no further step for looking up a certificate chain.
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

private:
  /**
   * Check the type of signatureInfo to get the KeyLocator. Use the public key
   * DER in the KeyLocator (if available) or look in the IdentityStorage for the
   * public key with the name in the KeyLocator (if available) and use it to
   * verify the signedBlob. If the public key can't be found, return false.
   * (This is a generalized method which can verify both a Data packet and an
   * interest.)
   * @param signatureInfo An object of a subclass of Signature, e.g.
   * Sha256WithRsaSignature.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @return True if the signature is verified, false if failed.
   */
  bool
  verify(const Signature* signatureInfo, const SignedBlob& signedBlob);

  /**
   * Return the public key DER in the KeyLocator (if available) or look in the
   * IdentityStorage for the public key with the name in the KeyLocator
   * (if available). If the public key can't be found, return and empty Blob.
   * @param keyLocator The KeyLocator.
   * @return The public key DER or an empty Blob if not found.
   */
  Blob
  getPublicKeyDer(const KeyLocator& keyLocator);

  IdentityStorage* identityStorage_;
};

}

#endif
