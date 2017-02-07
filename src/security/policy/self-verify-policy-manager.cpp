/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/security/policy/self-verify-policy-manager.hpp>

using namespace std;

INIT_LOGGER("ndn.SelfVerifyPolicyManager");

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
  (const ptr_lib::shared_ptr<Data>& data, int stepCount, 
   const OnVerified& onVerified,
   const OnDataValidationFailed& onValidationFailed)
{
  string failureReason = "unknown";
  // wireEncode returns the cached encoding if available.
  if (verify(data->getSignature(), data->wireEncode(), failureReason)) {
    try {
      onVerified(data);
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onVerified: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onVerified.");
    }
  }
  else {
    try {
      onValidationFailed(data, failureReason);
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onDataValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onDataValidationFailed.");
    }
  }

  // No more steps, so return a null ValidationRequest.
  return ptr_lib::shared_ptr<ValidationRequest>();
}

ptr_lib::shared_ptr<ValidationRequest>
SelfVerifyPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnInterestValidationFailed& onValidationFailed,
   WireFormat& wireFormat)
{
  if (interest->getName().size() < 2) {
    try {
      onValidationFailed
        (interest,
         "The signed interest has less than 2 components: " +
         interest->getName().toUri());
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  string failureReason = "unknown";
  // Decode the last two name components of the signed interest
  ptr_lib::shared_ptr<Signature> signature;
  try {
    signature =
      wireFormat.decodeSignatureInfoAndValue
        (interest->getName().get(-2).getValue(),
         interest->getName().get(-1).getValue());
  } catch (const std::exception& ex) {
    try {
      onValidationFailed
        (interest, 
         string("Error decoding the signed interest signature: ") + ex.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
  }

  // wireEncode returns the cached encoding if available.
  if (verify(signature.get(), interest->wireEncode(), failureReason)) {
    try {
      onVerified(interest);
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onVerified: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onVerified.");
    }
  }
  else {
    try {
      onValidationFailed(interest, failureReason);
    } catch (const std::exception& ex) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("SelfVerifyPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
  }

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
  (const Signature* signatureInfo, const SignedBlob& signedBlob,
   string& failureReason)
{
  Blob publicKeyDer;
  if (KeyLocator::canGetFromSignature(signatureInfo)) {
    publicKeyDer = getPublicKeyDer
      (KeyLocator::getFromSignature(signatureInfo), failureReason);
    if (!publicKeyDer)
      return false;
  }

  if (verifySignature(signatureInfo, signedBlob, publicKeyDer))
    return true;
  else {
    failureReason = "The signature did not verify with the given public key";
    return false;
  }
}

Blob
SelfVerifyPolicyManager::getPublicKeyDer
  (const KeyLocator& keyLocator, string& failureReason)
{
  if (keyLocator.getType() == ndn_KeyLocatorType_KEYNAME && identityStorage_) {
    Name keyName;
    try {
      // Assume the key name is a certificate name.
      keyName = IdentityCertificate::certificateNameToPublicKeyName
         (keyLocator.getKeyName());
    } catch (const std::exception&) {
      failureReason = "Cannot get a public key name from the certificate named: " +
        keyLocator.getKeyName().toUri();
      return Blob();
    }
    try {
      return identityStorage_->getKey(keyName);
    } catch (SecurityException&) {
      failureReason = "The identityStorage doesn't have the key named " +
        keyName.toUri();
      return Blob();
    }
  }
  else {
    // Can't find a key to verify.
    failureReason = "The signature KeyLocator doesn't have a key name";
    return Blob();
  }
}

}
