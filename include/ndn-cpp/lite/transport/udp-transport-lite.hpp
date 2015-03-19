/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
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

#ifndef NDN_UDP_TRANSPORT_LITE_HPP
#define NDN_UDP_TRANSPORT_LITE_HPP

#include "../../c/errors.h"
#include "../../c/transport/transport-types.h"

namespace ndn {

class UdpTransportLite : private ndn_UdpTransport {
public:
  /**
   * Create a UdpTransportLite with default values for no connection yet.
   */
  UdpTransportLite();

  /**
   * Connect with UDP to the host:port.
   * @param host The host to connect to.
   * @param port The port to connect to.
   * @return 0 for success, else an error code.
   */
  ndn_Error
  connect(char* host, unsigned short port);

  /**
   * Send data to the socket.
   * @param data A pointer to the buffer of data to send.
   * @param dataLength The number of bytes in data.
   * @return 0 for success, else an error code.
   */
  ndn_Error
  send(const uint8_t* data, size_t dataLength);

  /**
   * Check if there is data ready on the socket to be received with receive.
   * This does not block, and returns immediately.
   * @param receiveIsReady This will be set to true if data is ready, false if not.
   * @return 0 for success, else an error code.
   */
  ndn_Error
  receiveIsReady(bool& receiveIsReady);

  /**
   * Receive data from the socket.  NOTE: This is a blocking call.
   * You should first call receiveIsReady to make sure there is data ready to
   * receive.
   * @param buffer A pointer to the buffer to receive the data.
   * @param bufferLength The maximum length of buffer.
   * @param nBytes Return the number of bytes received into buffer.
   * @return 0 for success, else an error code.
   */
  ndn_Error
  receive(uint8_t* buffer, size_t bufferLength, size_t& nBytes);

  /**
   * Close the socket.
   * @return 0 for success, else an error code.
   */
  ndn_Error
  close();

  /**
   * Upcast the reference to the ndn_UdpTransport struct to a UdpTransportLite.
   * @param transport A reference to the ndn_UdpTransport struct.
   * @return The same reference as UdpTransportLite.
   */
  static UdpTransportLite&
  upCast(ndn_UdpTransport& transport) { return *(UdpTransportLite*)&transport; }

  static const UdpTransportLite&
  upCast(const ndn_UdpTransport& transport) { return *(UdpTransportLite*)&transport; }
};

}

#endif
