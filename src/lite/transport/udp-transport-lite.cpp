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


#include "../../c/transport/udp-transport.h"
#include <ndn-cpp/lite/transport/udp-transport-lite.hpp>

namespace ndn {

UdpTransportLite::UdpTransportLite()
{
  ndn_UdpTransport_initialize(this);
}

ndn_Error
UdpTransportLite::connect(char* host, unsigned short port)
{
  return ndn_UdpTransport_connect(this, host, port);
}

ndn_Error
UdpTransportLite::send(const uint8_t* data, size_t dataLength)
{
  return ndn_UdpTransport_send(this, data, dataLength);
}

ndn_Error
UdpTransportLite::receiveIsReady(bool& receiveIsReady)
{
  int isReady;
  ndn_Error error = ndn_UdpTransport_receiveIsReady(this, &isReady);
  receiveIsReady = (isReady != 0);
  return error;
}

ndn_Error
UdpTransportLite::receive(uint8_t* buffer, size_t bufferLength, size_t& nBytes)
{
  return ndn_UdpTransport_receive(this, buffer, bufferLength, &nBytes);
}

ndn_Error
UdpTransportLite::close()
{
  return ndn_UdpTransport_close(this);
}

}
