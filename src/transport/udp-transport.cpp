/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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

#include <stdexcept>
#include <stdlib.h>
#include "../c/transport/udp-transport.h"
#include "../c/encoding/element-reader.h"
#include "../encoding/element-listener.hpp"
#include "../c/util/ndn_realloc.h"
#include <ndn-cpp/transport/udp-transport.hpp>

using namespace std;

namespace ndn {

UdpTransport::ConnectionInfo::~ConnectionInfo()
{  
}

UdpTransport::UdpTransport() 
  : isConnected_(false), transport_(new struct ndn_UdpTransport), elementReader_(new struct ndn_ElementReader)
{
  ndn_UdpTransport_initialize(transport_.get());
  elementReader_->partialData.array = 0;
}

void 
UdpTransport::connect(const Transport::ConnectionInfo& connectionInfo, ElementListener& elementListener)
{
  const UdpTransport::ConnectionInfo& udpConnectionInfo = dynamic_cast<const UdpTransport::ConnectionInfo&>(connectionInfo);
  
  ndn_Error error;
  if ((error = ndn_UdpTransport_connect(transport_.get(), (char *)udpConnectionInfo.getHost().c_str(), udpConnectionInfo.getPort())))
    throw runtime_error(ndn_getErrorString(error)); 

  // TODO: This belongs in the socket listener.
  const size_t initialLength = 1000;
  // Automatically cast elementReader_ to (struct ndn_ElementListener *)
  ndn_ElementReader_initialize
    (elementReader_.get(), &elementListener, (uint8_t *)malloc(initialLength), initialLength, ndn_realloc);
  
  isConnected_ = true;
}

void 
UdpTransport::send(const uint8_t *data, size_t dataLength)
{
  ndn_Error error;
  if ((error = ndn_UdpTransport_send(transport_.get(), data, dataLength)))
    throw runtime_error(ndn_getErrorString(error));  
}

void 
UdpTransport::processEvents()
{
  // Loop until there is no more data in the receive buffer.
  while(true) {
    int receiveIsReady;
    ndn_Error error;
    if ((error = ndn_UdpTransport_receiveIsReady
         (transport_.get(), &receiveIsReady)))
      throw runtime_error(ndn_getErrorString(error));  
    if (!receiveIsReady)
      return;

    uint8_t buffer[8000];
    size_t nBytes;
    if ((error = ndn_UdpTransport_receive
         (transport_.get(), buffer, sizeof(buffer), &nBytes)))
      throw runtime_error(ndn_getErrorString(error));  
    if (nBytes == 0)
      return;

    ndn_ElementReader_onReceivedData(elementReader_.get(), buffer, nBytes);
  }
}

bool 
UdpTransport::getIsConnected()
{
  return isConnected_;
}

void 
UdpTransport::close()
{
  ndn_Error error;
  if ((error = ndn_UdpTransport_close(transport_.get())))
    throw runtime_error(ndn_getErrorString(error));  
}

UdpTransport::~UdpTransport()
{
  if (elementReader_->partialData.array)
    // Free the memory allocated in connect.
    free(elementReader_->partialData.array);
}

}
