/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
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

#include <stdexcept>
#include <ndn-cpp/security/v2/validator.hpp>
#include <ndn-cpp/security/v2/validation-policy-from-pib.hpp>

#include "ndn-cpp/security/v2/certificate-storage.hpp"

using namespace std;

namespace ndn {

void
ValidationPolicyFromPib::checkPolicy
  (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  Name keyName = getKeyLocatorName(data, *state);
  if (state->isOutcomeFailed())
    // Already called state.fail() .
    return;
  
  checkPolicyHelper(keyName, state, continueValidation);
}

void
ValidationPolicyFromPib::checkPolicy
  (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  Name keyName = getKeyLocatorName(interest, *state);
  if (state->isOutcomeFailed())
    // Already called state.fail() .
    return;

  checkPolicyHelper(keyName, state, continueValidation);
}

void
ValidationPolicyFromPib::checkPolicyHelper
  (const Name& keyName, const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  ptr_lib::shared_ptr<PibIdentity> identity;
  try {
    identity = pib_.getIdentity(PibKey::extractIdentityFromKeyName(keyName));
  } catch (const std::exception& ex) {
    state->fail(ValidationError
      (ValidationError::CANNOT_RETRIEVE_CERTIFICATE,
       "Cannot get the PIB identity for key " + keyName.toUri() + ": " + ex.what()));
    return;
  }

  ptr_lib::shared_ptr<PibKey> key;
  try {
    key = identity->getKey(keyName);
  } catch (const std::exception& ex) {
    state->fail(ValidationError
      (ValidationError::CANNOT_RETRIEVE_CERTIFICATE,
       "Cannot get the PIB key " + keyName.toUri() + ": " + ex.what()));
    return;
  }

  ptr_lib::shared_ptr<CertificateV2> certificate;
  try {
    certificate = key->getDefaultCertificate();
  } catch (const std::exception& ex) {
    state->fail(ValidationError
      (ValidationError::CANNOT_RETRIEVE_CERTIFICATE,
       "Cannot get the default certificate for key " + keyName.toUri() + ": " +
       ex.what()));
    return;
  }

  // Add the certificate as the temporary trust anchor.
  validator_->resetAnchors();
  validator_->loadAnchor("", *certificate);
  continueValidation
    (ptr_lib::make_shared<CertificateRequest>(Interest(keyName)), state);
  // Clear the temporary trust anchor.
  validator_->resetAnchors();
}

}
