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

#ifndef NDN_VALIDATION_REQUEST_HPP
#define NDN_VALIDATION_REQUEST_HPP

#include "../key-chain.hpp"

namespace ndn {

/**
 * An OnVerified function object is used to pass a callback to verifyData to report a successful verification.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<Data>& data)> OnVerified;

/**
 * An OnVerifyFailed function object is used to pass a callback to verifyData to report a failed verification.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<Data>& data)> OnVerifyFailed;

/**
 * An OnVerifiedInterest function object is used to pass a callback to
 * verifyInterest to report a successful verification.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<Interest>& interest)>
  OnVerifiedInterest;

/**
 * An OnVerifyInterestFailed function object is used to pass a callback to
 * verifyInterest to report a failed verification.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<Interest>& interest)>
  OnVerifyInterestFailed;

class ValidationRequest {
public:
  ValidationRequest
    (const ptr_lib::shared_ptr<Interest> &interest, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed,
     int retry, int stepCount)
  : interest_(interest), onVerified_(onVerified), onVerifyFailed_(onVerifyFailed), retry_(retry), stepCount_(stepCount)
  {
  }

  virtual
  ~ValidationRequest() {}

  ptr_lib::shared_ptr<Interest> interest_; // An interest packet to fetch the requested data.
  OnVerified onVerified_;                  // A callback function if the requested certificate has been validated.
  OnVerifyFailed onVerifyFailed_;          // A callback function if the requested certificate cannot be validated.
  int retry_;                              // The number of retrials when there is an interest timeout.
  int stepCount_;
};

}

#endif
