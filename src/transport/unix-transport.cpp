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

#include <stdexcept>
#include <stdlib.h>
#include "../c/transport/unix-transport.h"
#include "../c/encoding/element-reader.h"
#include "../encoding/element-listener.hpp"
#include "../util/dynamic-uint8-vector.hpp"
#include <ndn-cpp/transport/unix-transport.hpp>

using namespace std;

namespace ndn {

UnixTransport::ConnectionInfo::~ConnectionInfo()
{
}

UnixTransport::UnixTransport()
  : isConnected_(false), transport_(new struct ndn_UnixTransport),
    elementBuffer_(new DynamicUInt8Vector(1000))
{
  ndn_UnixTransport_initialize(transport_.get(), elementBuffer_.get());
}

bool
UnixTransport::isLocal(const Transport::ConnectionInfo& connectionInfo)
{
  return ndn_UnixTransport_isLocal() != 0;
}

void
UnixTransport::connect
  (const Transport::ConnectionInfo& connectionInfo,
   ElementListener& elementListener)
{
  const UnixTransport::ConnectionInfo& unixConnectionInfo =
    dynamic_cast<const UnixTransport::ConnectionInfo&>(connectionInfo);

  ndn_Error error;
  if ((error = ndn_UnixTransport_connect
       (transport_.get(), (char *)unixConnectionInfo.getFilePath().c_str(),
        &elementListener)))
    throw runtime_error(ndn_getErrorString(error));

  isConnected_ = true;
}

void
UnixTransport::send(const uint8_t *data, size_t dataLength)
{
  ndn_Error error;
  if ((error = ndn_UnixTransport_send(transport_.get(), data, dataLength)))
    throw runtime_error(ndn_getErrorString(error));
}

void
UnixTransport::processEvents()
{
  uint8_t buffer[MAX_NDN_PACKET_SIZE];
  ndn_Error error;
  if ((error = ndn_UnixTransport_processEvents
       (transport_.get(), buffer, sizeof(buffer))))
    throw runtime_error(ndn_getErrorString(error));
}

bool
UnixTransport::getIsConnected()
{
  return isConnected_;
}

void
UnixTransport::close()
{
  ndn_Error error;
  if ((error = ndn_UnixTransport_close(transport_.get())))
    throw runtime_error(ndn_getErrorString(error));
}

}
