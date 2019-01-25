/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validation-error.hpp
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

#ifndef NDN_VALIDATION_ERROR_HPP
#define NDN_VALIDATION_ERROR_HPP

#include <string>
#include "../../common.hpp"

namespace ndn {

/**
 * A ValidationError holds an error code and an optional detailed error message.
 */
class ValidationError {
public:
  static const int NO_ERROR =                    0;
  static const int INVALID_SIGNATURE =           1;
  static const int NO_SIGNATURE =                2;
  static const int CANNOT_RETRIEVE_CERTIFICATE = 3;
  static const int EXPIRED_CERTIFICATE =         4;
  static const int LOOP_DETECTED =               5;
  static const int MALFORMED_CERTIFICATE =       6;
  static const int EXCEEDED_DEPTH_LIMIT =        7;
  static const int INVALID_KEY_LOCATOR =         8;
  static const int POLICY_ERROR =                9;
  static const int IMPLEMENTATION_ERROR =        255;
  // Custom error codes should use >= USER_MIN.
  static const int USER_MIN =                    256;

  /**
   * Create a new ValidationError for the given code.
   * @param code The code which is one of the standard error codes such as
   * INVALID_SIGNATURE, or a custom code if greater than or equal to USER_MIN.
   * @param info (optional) The optional error message.
   */
  ValidationError(int code, const std::string& info = "")
  : code_(code), info_(info)
  {
  }

  /**
   * Get the error code given to the constructor.
   * @return The error code which is one of the standard error codes such as
   * INVALID_SIGNATURE, or a custom code if greater than or equal to USER_MIN.
   */
  int
  getCode() const { return code_; }

  /**
   * Get the error message given to the constructor.
   * @return The error message, or "" if none.
   */
  const std::string&
  getInfo() const { return info_; }

  /**
   * Get a string representation of this ValidationError.
   * @return The string representation.
   */
  std::string
  toString() const;

private:
  int code_;
  std::string info_;
};

std::ostream&
operator<<(std::ostream& os, const ValidationError& error);

}

#endif
