/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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
  : hasName_(false), faceId_(-1), localControlFeature_(-1), origin_(-1), cost_(-1),
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

  void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(input.buf(), input.size(), wireFormat);
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

  /**
   * Check if the name is specified.
   * @return True if the name is specified, false if not.
   */
  bool
  getHasName() const { return hasName_; }

  /**
   * Get the name, if specified.
   * @return The Name. This is only meaningful if getHasName() is true.
   */
  const Name&
  getName() const { return name_; }

  int
  getFaceId() const { return faceId_; }

  const std::string&
  getUri() const { return uri_; }

  int
  getLocalControlFeature() const { return localControlFeature_; }

  int
  getOrigin() const { return origin_; }

  int
  getCost() const { return cost_; }

  ForwardingFlags&
  getForwardingFlags() { return flags_; }

  const ForwardingFlags&
  getForwardingFlags() const { return flags_; }

  Name&
  getStrategy() { return strategy_; }

  const Name&
  getStrategy() const { return strategy_; }

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

  /**
   * Set the flag for whether the name is specified. Note that setName
   * automatically calls setHasName(true).
   * @param hasName True if the name is specified, false if not.
   */
  void
  setHasName(bool hasName) { hasName_ = hasName; }

  /**
   * Set the name. This also calls setHasName(true).
   * @param name The name. This makes a copy of the name. If the name is not
   * specified, call setHasName(false).
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setName(const Name& name)
  {
    hasName_ = true;
    name_ = name;
    return *this;
  }

  /**
   * Set the Face ID.
   * @param faceId The new face ID, or -1 for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setFaceId(int faceId)
  {
    faceId_ = faceId;
    return *this;
  }

  /**
   * Set the URI.
   * @param uri The new uri, or an empty string for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setUri(const std::string& uri)
  {
    uri_ = uri;
    return *this;
  }

  /**
   * Set the local control feature value.
   * @param localControlFeature The new local control feature value, or -1 for
   * not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setLocalControlFeature(int localControlFeature)
  {
    localControlFeature_ = localControlFeature;
    return *this;
  }

  /**
   * Set the origin value.
   * @param origin The new origin value, or -1 for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setOrigin(int origin)
  {
    origin_ = origin;
    return *this;
  }

  /**
   * Set the cost value.
   * @param cost The new cost value, or -1 for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setCost(int cost)
  {
    cost_ = cost;
    return *this;
  }

  /**
   * Set the ForwardingFlags object to a copy of forwardingFlags. You can use
   * getForwardingFlags() and change the existing ForwardingFlags object.
   * @param forwardingFlags The new cost value, or null for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setForwardingFlags(const ForwardingFlags& flags)
  {
    flags_ = flags;
    return *this;
  }

  /**
   * Set the strategy to a copy of the given Name.
   * @param strategy The Name to copy, or an empty Name if not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setStrategy(const Name& strategy)
  {
    strategy_ = strategy;
    return *this;
  }

  /**
   * Set the expiration period.
   * @param expirationPeriod The expiration period in milliseconds, or
   * null for not specified.
   * @return This ControlParameters so that you can chain calls to update values.
   */
  ControlParameters&
  setExpirationPeriod(Milliseconds expirationPeriod)
  {
    expirationPeriod_ = expirationPeriod;
    return *this;
  }

private:
  bool hasName_;
  Name name_;                     /**< Only used if hasName_ */
  int faceId_;                    /**< -1 for none. */
  std::string uri_;               /**< "" for none. */
  int localControlFeature_;       /**< -1 for none. */
  int origin_;                    /**< -1 for none. */
  int cost_;                      /**< -1 for none. */
  ForwardingFlags flags_;
  Name strategy_;                 /**< empty for none. */
  Milliseconds expirationPeriod_; /**< -1 for none. */
};

}

#endif
