/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#ifndef NDN_ENCRYPT_ERROR_HPP
#define NDN_ENCRYPT_ERROR_HPP

#include "../common.hpp"

namespace ndn {

/**
 * EncryptError holds the ErrorCode enum and OnError callback definition for
 * errors from the encrypt library.
 */
class EncryptError {
public:
  enum ErrorCode {
    Timeout =                      1,
    Validation =                   2,
    UnsupportedEncryptionScheme = 32,
    InvalidEncryptedFormat =      33,
    NoDecryptKey =                34,
    EncryptionFailure =           35,
    DataRetrievalFailure =        36,
    General =                    100
  };

  /**
   * A method calls onError(errorCode, message) for an error.
   */
  typedef func_lib::function<void
    (ErrorCode errorCode, const std::string& message)> OnError;
};

}

#endif
