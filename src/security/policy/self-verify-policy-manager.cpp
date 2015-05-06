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

#include "../../c/util/crypto.h"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/identity-storage.hpp>
#include <ndn-cpp/security/policy/self-verify-policy-manager.hpp>

using namespace std;

namespace ndn {

SelfVerifyPolicyManager::~SelfVerifyPolicyManager()
{
}

bool
SelfVerifyPolicyManager::skipVerifyAndTrust(const Data& data)
{
  return false;
}

bool
SelfVerifyPolicyManager::skipVerifyAndTrust(const Interest& interest)
{
  return false;
}

bool
SelfVerifyPolicyManager::requireVerify(const Data& data)
{
  return true;
}

bool
SelfVerifyPolicyManager::requireVerify(const Interest& interest)
{
  return true;
}

ptr_lib::shared_ptr<ValidationRequest>
SelfVerifyPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Data>& data, int stepCount, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed)
{
  // wireEncode returns the cached encoding if available.
  if (verify(data->getSignature(), data->wireEncode()))
    onVerified(data);
  else
    onVerifyFailed(data);

  // No more steps, so return a null ValidationRequest.
  return ptr_lib::shared_ptr<ValidationRequest>();
}

ptr_lib::shared_ptr<ValidationRequest>
SelfVerifyPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat)
{
  // Decode the last two name components of the signed interest
  ptr_lib::shared_ptr<Signature> signature =
    wireFormat.decodeSignatureInfoAndValue
      (interest->getName().get(-2).getValue(),
       interest->getName().get(-1).getValue());

  // wireEncode returns the cached encoding if available.
  if (verify(signature.get(), interest->wireEncode()))
    onVerified(interest);
  else
    onVerifyFailed(interest);

  // No more steps, so return a null ValidationRequest.
  return ptr_lib::shared_ptr<ValidationRequest>();
}

bool
SelfVerifyPolicyManager::checkSigningPolicy(const Name& dataName, const Name& certificateName)
{
  return true;
}

Name
SelfVerifyPolicyManager::inferSigningIdentity(const Name& dataName)
{
  return Name();
}

bool
SelfVerifyPolicyManager::verify
  (const Signature* signatureInfo, const SignedBlob& signedBlob)
{
  Blob publicKeyDer;
  if (KeyLocator::canGetFromSignature(signatureInfo)) {
    publicKeyDer = getPublicKeyDer(KeyLocator::getFromSignature(signatureInfo));
    if (!publicKeyDer)
      return false;
  }

  return verifySignature(signatureInfo, signedBlob, publicKeyDer);
}

Blob
SelfVerifyPolicyManager::getPublicKeyDer(const KeyLocator& keyLocator)
{
  if (keyLocator.getType() == ndn_KeyLocatorType_KEY)
    // Use the public key DER directly.
    return keyLocator.getKeyData();
  else if (keyLocator.getType() == ndn_KeyLocatorType_KEYNAME &&
           identityStorage_)
    // Assume the key name is a certificate name.
    return identityStorage_->getKey
      (IdentityCertificate::certificateNameToPublicKeyName
       (keyLocator.getKeyName()));
  else
    // Can't find a key to verify.
    return Blob();
}

}
