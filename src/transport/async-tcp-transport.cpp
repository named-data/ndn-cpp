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
#include "../c/transport/tcp-transport.h"
#include "../c/encoding/element-reader.h"
#include "../encoding/element-listener.hpp"
#include "../util/dynamic-uint8-vector.hpp"
#include "async-socket-transport.hpp"
#include <ndn-cpp/transport/async-tcp-transport.hpp>

using namespace std;

using boost::asio::ip::tcp;

namespace ndn {

/**
 * AsyncTcpTransport::SocketTransport simply extends
 * AsyncSocketTransport<boost::asio::ip::tcp> so that we can forward declare it
 * in the main header file without including the Boost headers for
 * AsyncSocketTransport.
 */
class AsyncTcpTransport::SocketTransport
  : public AsyncSocketTransport<boost::asio::ip::tcp> {
public:
  SocketTransport(boost::asio::io_service& ioService)
  : AsyncSocketTransport(ioService)
  {
  }
};

AsyncTcpTransport::ConnectionInfo::~ConnectionInfo()
{
}

AsyncTcpTransport::AsyncTcpTransport(boost::asio::io_service& ioService)
: ioService_(ioService), socketTransport_(new SocketTransport(ioService)),
  connectionInfo_("", 0)
{
}

bool
AsyncTcpTransport::isLocal(const Transport::ConnectionInfo& connectionInfo)
{
  const AsyncTcpTransport::ConnectionInfo& tcpConnectionInfo =
    dynamic_cast<const AsyncTcpTransport::ConnectionInfo&>(connectionInfo);

  if (connectionInfo_.getHost() == "" ||
      connectionInfo_.getHost() != tcpConnectionInfo.getHost()) {
    ndn_Error error;
    int intIsLocal;
    if ((error = ndn_TcpTransport_isLocal
         ((char *)tcpConnectionInfo.getHost().c_str(), &intIsLocal)))
      throw runtime_error(ndn_getErrorString(error));

    // Cache the result in isLocal_ and save connectionInfo_ for next time.
    connectionInfo_ = tcpConnectionInfo;
    isLocal_ = (intIsLocal != 0);
  }

  return isLocal_;
}

bool
AsyncTcpTransport::isAsync() { return true; }

void
AsyncTcpTransport::connect
  (const Transport::ConnectionInfo& connectionInfo,
   ElementListener& elementListener, const OnConnected& onConnected)
{
  const AsyncTcpTransport::ConnectionInfo& tcpConnectionInfo =
    dynamic_cast<const AsyncTcpTransport::ConnectionInfo&>(connectionInfo);

  // Boost wants the port as a string.
  stringstream portString;
  portString << tcpConnectionInfo.getPort();

  tcp::resolver::iterator endpointIterator =
    tcp::resolver(ioService_).resolve(tcp::resolver::query
      (tcpConnectionInfo.getHost(), portString.str()));

  // Assume the first entry in the iterator is good.
  // TODO: Do we need to check further entries in the iterator?
  socketTransport_->connect(*endpointIterator, elementListener, onConnected);
}

void
AsyncTcpTransport::send(const uint8_t *data, size_t dataLength)
{
  socketTransport_->send(data, dataLength);
}

bool
AsyncTcpTransport::getIsConnected()
{
  return socketTransport_->getIsConnected();
}

void
AsyncTcpTransport::processEvents()
{
}

void
AsyncTcpTransport::close()
{
  socketTransport_->close();
}

AsyncTcpTransport::~AsyncTcpTransport()
{
}

}

#endif // NDN_CPP_HAVE_BOOST_ASIO
