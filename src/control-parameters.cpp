/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
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
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/control-parameters.hpp>

using namespace std;

namespace ndn {

void
ControlParameters::get
  (ControlParametersLite& controlParametersLite) const
{
  controlParametersLite.setHasName(hasName_);
  name_.get(controlParametersLite.getName());
  controlParametersLite.setFaceId(faceId_);
  // Store the uri_ string as an ndn_Blob.
  controlParametersLite.setUri
    (BlobLite((const uint8_t*)uri_.c_str(), uri_.size()));
  controlParametersLite.setLocalControlFeature(localControlFeature_);
  controlParametersLite.setOrigin(origin_);
  controlParametersLite.setCost(cost_);
  flags_.get(controlParametersLite.getForwardingFlags());
  strategy_.get(controlParametersLite.getStrategy());
  controlParametersLite.setExpirationPeriod(expirationPeriod_);
}

void
ControlParameters::set(const ControlParametersLite& controlParametersLite)
{
  hasName_ = controlParametersLite.getHasName();
  name_.set(controlParametersLite.getName());
  faceId_ = controlParametersLite.getFaceId();
  // Convert the BlobLite to the uri_ string.
  uri_.assign
    ((const char*)controlParametersLite.getUri().buf(),
     controlParametersLite.getUri().size());
  localControlFeature_ = controlParametersLite.getLocalControlFeature();
  origin_ = controlParametersLite.getOrigin();
  cost_ = controlParametersLite.getCost();
  flags_.set(controlParametersLite.getForwardingFlags());
  strategy_.set(controlParametersLite.getStrategy());
  expirationPeriod_ = controlParametersLite.getExpirationPeriod();
}

}
