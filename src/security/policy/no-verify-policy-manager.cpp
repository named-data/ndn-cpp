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

#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>

using namespace std;

namespace ndn {

NoVerifyPolicyManager::~NoVerifyPolicyManager()
{
}

bool
NoVerifyPolicyManager::skipVerifyAndTrust(const Data& data)
{
  return true;
}

bool
NoVerifyPolicyManager::skipVerifyAndTrust(const Interest& interest)
{
  return true;
}

bool
NoVerifyPolicyManager::requireVerify(const Data& data)
{
  return false;
}

bool
NoVerifyPolicyManager::requireVerify(const Interest& interest)
{
  return false;
}

ptr_lib::shared_ptr<ValidationRequest>
NoVerifyPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Data>& data, int stepCount, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed)
{
  onVerified(data);
  return ptr_lib::shared_ptr<ValidationRequest>();
}

ptr_lib::shared_ptr<ValidationRequest>
NoVerifyPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat)
{
  onVerified(interest);
  return ptr_lib::shared_ptr<ValidationRequest>();
}

bool
NoVerifyPolicyManager::checkSigningPolicy(const Name& dataName, const Name& certificateName)
{
  return true;
}

Name
NoVerifyPolicyManager::inferSigningIdentity(const Name& dataName)
{
  return Name();
}

}
