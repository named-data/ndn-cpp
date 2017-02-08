/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

#ifndef NDN_EXPONENTIAL_RE_EXPRESS_HPP
#define NDN_EXPONENTIAL_RE_EXPRESS_HPP

#include "../face.hpp"

namespace ndn {

/**
 * An ExponentialReExpress uses an internal onTimeout to express the interest
 * again with double the interestLifetime. See 
 * ExponentialReExpress.makeOnTimeout.
 */
class ExponentialReExpress : public ptr_lib::enable_shared_from_this<ExponentialReExpress> {
public:
  static OnTimeout
  makeOnTimeout
    (Face* face, const OnData& onData, const OnTimeout& onTimeout,
     Milliseconds maxInterestLifetime = 16000.0);

private:
  /**
   * Create a new ExponentialReExpress where onTimeout expresses the interest
   * again with double the interestLifetime. If the interesLifetime goes over
   * maxInterestLifetime, then call the given onTimeout. If this internally
   * gets onData, just call the given onData.
   */
  ExponentialReExpress
    (Face* face, const OnData& onData, const OnTimeout& onTimeout,
     Milliseconds maxInterestLifetime)
  : face_(face), callerOnData_(onData), callerOnTimeout_(onTimeout),
    maxInterestLifetime_(maxInterestLifetime)
  {}

  void
  onTimeout(const ptr_lib::shared_ptr<const Interest>& interest);

  Face* face_;
  OnData callerOnData_;
  OnTimeout callerOnTimeout_;
  Milliseconds maxInterestLifetime_;
};

}

#endif
