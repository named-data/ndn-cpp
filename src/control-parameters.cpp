/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
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
#include "c/control-parameters.h"

using namespace std;

namespace ndn {

void
ControlParameters::get
  (struct ndn_ControlParameters& controlParametersStruct) const
{
  name_.get(controlParametersStruct.name);
  controlParametersStruct.faceId = faceId_;
  // TODO: Add "Uri" string.
  controlParametersStruct.localControlFeature = localControlFeature_;
  controlParametersStruct.origin = origin_;
  controlParametersStruct.cost = cost_;
  controlParametersStruct.flags = flags_;
  // TODO: Add "Strategy" name.
  controlParametersStruct.expirationPeriod = expirationPeriod_;
}

void
ControlParameters::set
  (const struct ndn_ControlParameters& controlParametersStruct)
{
  name_.set(controlParametersStruct.name);
  faceId_ = controlParametersStruct.faceId;
  // TODO: Add "Uri" string.
  localControlFeature_ = controlParametersStruct.localControlFeature;
  origin_ = controlParametersStruct.origin;
  cost_ = controlParametersStruct.cost;
  flags_ = controlParametersStruct.flags;
  // TODO: Add "Strategy" name.
  expirationPeriod_ = controlParametersStruct.expirationPeriod;
}

}
