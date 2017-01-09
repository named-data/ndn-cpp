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

#include <ndn-cpp/control-response.hpp>

using namespace std;

namespace ndn {

void
ControlResponse::get(ControlResponseLite& controlResponseLite) const
{
  controlResponseLite.setStatusCode(statusCode_);
  // Store the statusText string as an ndn_Blob.
  controlResponseLite.setStatusText
    (BlobLite((const uint8_t*)statusText_.c_str(), statusText_.size()));
  if (bodyAsControlParameters_) {
    controlResponseLite.setHasBodyAsControlParameters(true);
    bodyAsControlParameters_->get(controlResponseLite.getBodyAsControlParameters());
  }
  else
    controlResponseLite.setHasBodyAsControlParameters(false);
}

void
ControlResponse::set(const ControlResponseLite& controlResponseLite)
{
  if (controlResponseLite.getHasBodyAsControlParameters()) {
    bodyAsControlParameters_.reset(new ControlParameters());
    bodyAsControlParameters_->set(controlResponseLite.getBodyAsControlParameters());
  }
  else
    bodyAsControlParameters_.reset();
  statusCode_ = controlResponseLite.getStatusCode();
  // Convert the BlobLite to the statusText_ string.
  statusText_.assign
    ((const char*)controlResponseLite.getStatusText().buf(),
     controlResponseLite.getStatusText().size());
}

}
