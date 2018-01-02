/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validation-error.cpp
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

#include <ndn-cpp/security/v2/validation-error.hpp>

using namespace std;

namespace ndn {

ostream&
operator<<(ostream& os, const ValidationError& error)
{
  int code = error.getCode();

  if (code == ValidationError::NO_ERROR)
    os << "No error";
  else if (code == ValidationError::INVALID_SIGNATURE)
    os << "Invalid signature";
  else if (code == ValidationError::NO_SIGNATURE)
    os << "Missing signature";
  else if (code == ValidationError::CANNOT_RETRIEVE_CERTIFICATE)
    os << "Cannot retrieve certificate";
  else if (code == ValidationError::EXPIRED_CERTIFICATE)
    os << "Certificate expired";
  else if (code == ValidationError::LOOP_DETECTED)
    os << "Loop detected in certification chain";
  else if (code == ValidationError::MALFORMED_CERTIFICATE)
    os << "Malformed certificate";
  else if (code == ValidationError::EXCEEDED_DEPTH_LIMIT)
    os << "Exceeded validation depth limit";
  else if (code == ValidationError::INVALID_KEY_LOCATOR)
    os << "Key locator violates validation policy";
  else if (code == ValidationError::POLICY_ERROR)
    os << "Validation policy error";
  else if (code == ValidationError::IMPLEMENTATION_ERROR)
    os << "Internal implementation error";
  else if (code >= ValidationError::USER_MIN)
    os << "Custom error code " << code;
  else
    os << "Unrecognized error code " << code;

  if (error.getInfo().size() > 0)
    os << " (" << error.getInfo() << ")";

  return os;
}

}
