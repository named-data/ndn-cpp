/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
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
