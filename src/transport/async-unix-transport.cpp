/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2017 Regents of the University of California.
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


// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_ASIO.
#include <ndn-cpp/ndn-cpp-config.h>
#ifdef NDN_CPP_HAVE_BOOST_ASIO

#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <stdlib.h>
#include "../c/transport/unix-transport.h"
#include "async-socket-transport.hpp"
#include <ndn-cpp/transport/async-unix-transport.hpp>

using namespace std;

using boost::asio::local::stream_protocol;

namespace ndn {

AsyncUnixTransport::ConnectionInfo::~ConnectionInfo()
{
}

AsyncUnixTransport::AsyncUnixTransport(boost::asio::io_service& ioService)
: socketTransport_(new AsyncSocketTransport<boost::asio::local::stream_protocol>
                   (ioService))
{
}

bool
AsyncUnixTransport::isLocal(const Transport::ConnectionInfo& connectionInfo)
{
  return ndn_UnixTransport_isLocal() != 0;
}

bool
AsyncUnixTransport::isAsync() { return true; }

void
AsyncUnixTransport::connect
  (const Transport::ConnectionInfo& connectionInfo,
   ElementListener& elementListener, const OnConnected& onConnected)
{
  const AsyncUnixTransport::ConnectionInfo& unixConnectionInfo =
    dynamic_cast<const AsyncUnixTransport::ConnectionInfo&>(connectionInfo);

  socketTransport_->connect
    (stream_protocol::endpoint(unixConnectionInfo.getFilePath()),
     elementListener, onConnected);
}

void
AsyncUnixTransport::send(const uint8_t *data, size_t dataLength)
{
  socketTransport_->send(data, dataLength);
}

bool
AsyncUnixTransport::getIsConnected()
{
  return socketTransport_->getIsConnected();
}

void
AsyncUnixTransport::processEvents()
{
}

void
AsyncUnixTransport::close()
{
  socketTransport_->close();
}

AsyncUnixTransport::~AsyncUnixTransport()
{
}

}

#endif // NDN_CPP_HAVE_BOOST_ASIO
