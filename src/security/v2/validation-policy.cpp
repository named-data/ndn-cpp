/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validation-policy.cpp
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
#include <ndn-cpp/security/v2/validation-policy.hpp>

using namespace std;

namespace ndn {

ValidationPolicy::~ValidationPolicy() {}

void
ValidationPolicy::setInnerPolicy
  (ptr_lib::shared_ptr<ValidationPolicy> innerPolicy)
{
  if (!innerPolicy)
    throw invalid_argument("The innerPolicy argument cannot be null");

  if (validator_)
    innerPolicy->setValidator(*validator_);

  if (!innerPolicy_)
    innerPolicy_ = innerPolicy;
  else
    innerPolicy_->setInnerPolicy(innerPolicy);
}

void
ValidationPolicy::checkPolicy
  (const CertificateV2& certificate,
   const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  checkPolicy(static_cast<const Data&>(certificate), state, continueValidation);
}

}
