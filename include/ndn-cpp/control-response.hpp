/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
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

#ifndef NDN_CONTROL_RESPONSE_HPP
#define NDN_CONTROL_RESPONSE_HPP

#include "control-parameters.hpp"
#include "encoding/wire-format.hpp"
#include "lite/control-response-lite.hpp"

namespace ndn {

/**
 * A ControlResponse holds a status code, status text and other fields for a
 * ControlResponse which is used, for example, in the response from sending a
 * register prefix control command to a forwarder.
 * @see http://redmine.named-data.net/projects/nfd/wiki/ControlCommand
 */
class ControlResponse {
public:
  /**
   * Create a new ControlResponse where all values are unspecified.
   */
  ControlResponse()
  : statusCode_(-1)
  {
  }

  /**
   * Create a new ControlResponse as a deep copy of the given ControlResponse.
   * @param controlResponse The ControlResponse to copy.
   */
  ControlResponse(const ControlResponse& controlResponse)
  : statusCode_(controlResponse.statusCode_),
    statusText_(controlResponse.statusText_)
  {
    if (controlResponse.bodyAsControlParameters_)
      bodyAsControlParameters_.reset
        (new ControlParameters(*controlResponse.bodyAsControlParameters_));
  }

  /**
   * Encode this ControlResponse for a particular wire format.
   * @param wireFormat (optional) A WireFormat object used to encode this
   * ControlResponse. If omitted, use WireFormat::getDefaultWireFormat().
   * @return The encoded buffer.
   */
  Blob
  wireEncode(WireFormat& wireFormat = *WireFormat::getDefaultWireFormat()) const
  {
    return wireFormat.encodeControlResponse(*this);
  }

  /**
   * Decode the input using a particular wire format and update this
   * ControlResponse.
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const uint8_t *input, size_t inputLength,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireFormat.decodeControlResponse(*this, input, inputLength);
  }

  /**
   * Decode the input using a particular wire format and update this
   * ControlResponse.
   * @param input The input byte array to be decoded.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const std::vector<uint8_t>& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(&input[0], input.size(), wireFormat);
  }

  /**
   * Decode the input using a particular wire format and update this
   * ControlResponse.
   * @param input The input byte array to be decoded as an immutable Blob.
   * @param wireFormat (optional) A WireFormat object used to decode the input.
   * If omitted, use WireFormat::getDefaultWireFormat().
   */
  void
  wireDecode
    (const Blob& input,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    wireDecode(input.buf(), input.size(), wireFormat);
  }

  /**
   * Set controlResponseLite to point to the components in this
   * ControlResponse, without copying any memory.
   * WARNING: The resulting pointers in controlResponseLite are invalid after
   * a further use of this object which could reallocate memory.
   * @param controlResponseLite The ControlResponseLite object which
   * receives the values. where the name components arrays are already allocated.
   */
  void
  get(ControlResponseLite& controlResponseLite) const;

  /**
   * Clear this ControlResponse, and set the values by copying from
   * controlResponseLite.
   * @param controlResponseLite A ControlResponseLite object.
   */
  void
  set(const ControlResponseLite& controlResponseLite);

  /**
   * Get the status code.
   * @return The status code. If not specified, return -1.
   */
  int
  getStatusCode() const { return statusCode_; }

  /**
   * Get the status text.
   * @return The status text. If not specified, return "".
   */
  const std::string&
  getStatusText() const { return statusText_; }

  /**
   * Get the control response body as a ControlParameters.
   * @return The ControlParameters, or 0 if the body is not specified or if it
   * is not a ControlParameters.
   */
  const ControlParameters*
  getBodyAsControlParameters() const { return bodyAsControlParameters_.get(); }

  /**
   * Set the status code.
   * @param statusCode The status code. If not specified, set to -1.
   * @return This ControlResponse so that you can chain calls to update values.
   */
  ControlResponse&
  setStatusCode(int statusCode)
  {
    statusCode_ = statusCode;
    return *this;
  }

  /**
   * Set the status text.
   * @param statusText The status text. If not specified, set to "".
   * @return This ControlResponse so that you can chain calls to update values.
   */
  ControlResponse&
  setStatusText(const std::string& statusText)
  {
    statusText_ = statusText;
    return *this;
  }

  /**
   * Set the control response body as a ControlParameters.
   * @param controlParameters A pointer to the ControlParameters for the body.
   * This makes a copy of the ControlParameters. If not specified or if the body
   * is not a ControlParameters, set to 0.
   * @return This ControlResponse so that you can chain calls to update values.
   */
  ControlResponse&
  setBodyAsControlParameters(const ControlParameters* controlParameters)
  {
    if (controlParameters)
      bodyAsControlParameters_.reset
        (new ControlParameters(*controlParameters));
    else
      bodyAsControlParameters_.reset();
    return *this;
  }

private:
  int statusCode_;
  std::string statusText_;
  ptr_lib::shared_ptr<ControlParameters> bodyAsControlParameters_;
};

}

#endif
