/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_CONTROL_PARAMETERS_HPP
#define NDN_CONTROL_PARAMETERS_HPP

#include "name.hpp"
#include "forwarding-flags.hpp"
#include "encoding/wire-format.hpp"

struct ndn_ControlParameters;

namespace ndn {

/**
 * A ControlParameters holds a Name and other fields for a 
 * ControlParameters which is used, for example, in the command interest to 
 * register a prefix with a forwarder.
 */
class ControlParameters {
public:    
  ControlParameters()
  : faceId_(-1), localControlFeature_(-1), origin_(-1), cost_(-1), 
    expirationPeriod_(-1.0)
  {
  }
  
  Blob 
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const 
  {
    return wireFormat.encodeControlParameters(*this);
  }
  
  void 
  wireDecode(const uint8_t *input, size_t inputLength, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) 
  {
    wireFormat.decodeControlParameters(*this, input, inputLength);
  }
  
  void 
  wireDecode(const std::vector<uint8_t>& input, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) 
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }
  
  /**
   * Set the controlParametersStruct to point to the components in this 
   * ControlParameters, without copying any memory.
   * WARNING: The resulting pointers in controlParametersStruct are 
   * invalid after a further use of this object which could reallocate memory.
   * @param controlParametersStruct a C ndn_ControlParameters 
   * struct where the name components array is already allocated.
   */
  void 
  get(struct ndn_ControlParameters& controlParametersStruct) const;

  Name& 
  getName() { return name_; }
  
  const Name& 
  getName() const { return name_; }
  
  int 
  getFaceId() const { return faceId_; }
  
  int 
  getLocalControlFeature() const { return localControlFeature_; }
  
  int 
  getOrigin() const { return origin_; }
  
  int 
  getCost() const { return cost_; }

  const ForwardingFlags& 
  getFlags() const { return flags_; }

  Milliseconds 
  getExpirationPeriod() const { return expirationPeriod_; }
  
  /**
   * Clear this ControlParameters, and set the values by copying from 
   * controlParametersStruct.
   * @param controlParametersStruct a C ndn_ControlParameters 
   * struct.
   */
  void 
  set(const struct ndn_ControlParameters& controlParametersStruct);
  
  ControlParameters& 
  setName(const Name& name) 
  { 
    name_ = name; 
    return *this;
  }
  
  ControlParameters& 
  setFaceId(int faceId) 
  { 
    faceId_ = faceId; 
    return *this;
  }
      
  ControlParameters& 
  setLocalControlFeature(int localControlFeature) 
  { 
    localControlFeature_ = localControlFeature; 
    return *this;
  }
      
  ControlParameters& 
  setOrigin(int origin) 
  { 
    origin_ = origin; 
    return *this;
  }
      
  ControlParameters& 
  setCost(int cost) 
  { 
    cost_ = cost; 
    return *this;
  }
      
  ControlParameters& 
  setFlags(const ForwardingFlags& flags) 
  { 
    flags_ = flags; 
    return *this;
  }
      
  ControlParameters& 
  setExpirationPeriod(Milliseconds expirationPeriod) 
  { 
    expirationPeriod_ = expirationPeriod; 
    return *this;
  }
      
private:
  Name name_;
  int faceId_;                    /**< -1 for none. */
  // TODO: Add "Uri" string.
  int localControlFeature_;       /**< -1 for none. */
  int origin_;                    /**< -1 for none. */
  int cost_;                      /**< -1 for none. */
  ForwardingFlags flags_;
  // TODO: Add "Strategy" name.
  Milliseconds expirationPeriod_; /**< -1 for none. */
};

}

#endif
