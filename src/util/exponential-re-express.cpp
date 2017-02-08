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

#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/util/exponential-re-express.hpp>

INIT_LOGGER("ndn.ExponentialReExpress");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

OnTimeout
ExponentialReExpress::makeOnTimeout
  (Face* face, const OnData& onData, const OnTimeout& onTimeout,
   Milliseconds maxInterestLifetime)
{
  ptr_lib::shared_ptr<ExponentialReExpress> reExpress
    (new ExponentialReExpress(face, onData, onTimeout, maxInterestLifetime));
  return bind(&ExponentialReExpress::onTimeout, reExpress, _1);
}

void
ExponentialReExpress::onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest)
{
  Milliseconds interestLifetime = interest->getInterestLifetimeMilliseconds();
  if (interestLifetime < 0) {
    // Can't re-express.
    if (callerOnTimeout_) {
      try {
        callerOnTimeout_(interest);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ExponentialReExpress::onTimeout: Error in onTimeout: " <<
                   ex.what());
      } catch (...) {
        _LOG_ERROR("ExponentialReExpress::onTimeout: Error in onTimeout.");
      }
    }
    return;
  }

  Milliseconds nextInterestLifetime = interestLifetime * 2;
  if (nextInterestLifetime > maxInterestLifetime_) {
    if (callerOnTimeout_) {
      try {
        callerOnTimeout_(interest);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ExponentialReExpress::onTimeout: Error in onTimeout: " <<
                   ex.what());
      } catch (...) {
        _LOG_ERROR("ExponentialReExpress::onTimeout: Error in onTimeout.");
      }
    }
    return;
  }

  Interest nextInterest(*interest);
  nextInterest.setInterestLifetimeMilliseconds(nextInterestLifetime);
  face_->expressInterest
    (nextInterest, callerOnData_, 
     bind(&ExponentialReExpress::onTimeout, shared_from_this(), _1));
}

}
