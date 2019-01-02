/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2019 Regents of the University of California.
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

#ifndef NDN_ASYNC_TCP_TRANSPORT_HPP
#define NDN_ASYNC_TCP_TRANSPORT_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_ASIO.
#include "../ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_BOOST_ASIO

#include <string>
#include <boost/asio.hpp>
#include "../common.hpp"
#include "../c/encoding/element-reader-types.h"
#include "transport.hpp"

namespace ndn {

/**
 * AsyncTcpTransport extends Transport for async communication over TCP using
 * Boost's asio io_service. This only uses asio for communication. To make this
 * thread-safe, you must dispatch calls to send(), etc. to the io_service, as is
 * done by ThreadsafeFace. To use this, you do not need to call processEvents.
 */
class AsyncTcpTransport : public Transport {
public:
  /**
   * An AsyncTcpTransport::ConnectionInfo extends Transport::ConnectionInfo to
   * hold the host and port info for the TCP connection.
   */
  class ConnectionInfo : public Transport::ConnectionInfo {
  public:
    /**
     * Create a ConnectionInfo with the given host and port.
     * @param host The host for the connection.
     * @param port The port number for the connection. If omitted, use 6363.
     */
    ConnectionInfo(const char *host, unsigned short port = 6363)
    : host_(host), port_(port)
    {
    }

    /**
     * Get the host given to the constructor.
     * @return A string reference for the host.
     */
    const std::string&
    getHost() const { return host_; }

    /**
     * Get the port given to the constructor.
     * @return The port number.
     */
    unsigned short
    getPort() const { return port_; }

    virtual
    ~ConnectionInfo();

  private:
    std::string host_;
    unsigned short port_;
  };

  /**
   * Create an AsyncTcpTransport in the unconnected state. This will use the
   * ioService to create the connection and communicate asynchronously.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   */
  AsyncTcpTransport(boost::asio::io_service& ioService);

  /**
   * Determine whether this transport connecting according to connectionInfo is
   * to a node on the current machine; results are cached. According to
   * http://redmine.named-data.net/projects/nfd/wiki/ScopeControl#local-face,
   * TCP transports with a loopback address are local. If connectionInfo
   * contains a host name, this will do a blocking DNS lookup; otherwise
   * this will parse the IP address and examine the first octet to determine if
   * it is a loopback address (e.g. the first IPv4 octet is 127 or IPv6 is "::1").
   * @param connectionInfo A TcpTransport.ConnectionInfo with the host to check.
   * @return True if the host is local, false if not.
   */
  virtual bool
  isLocal(const Transport::ConnectionInfo& connectionInfo);

  /**
   * Override to return true since connect needs to use the onConnected callback.
   * @return True.
   */
  virtual bool
  isAsync();

  /**
   * Connect according to the info in connectionInfo, and use elementListener.
   * To be thread-safe, this must be called from a dispatch to the ioService
   * which was given to the constructor, as is done by ThreadsafeFace.
   * @param connectionInfo A reference to an AsyncTcpTransport::ConnectionInfo.
   * @param elementListener Not a shared_ptr because we assume that it will
   * remain valid during the life of this object.
   * @param onConnected This calls onConnected() when the connection is
   * established.
   */
  virtual void
  connect
    (const Transport::ConnectionInfo& connectionInfo,
     ElementListener& elementListener, const OnConnected& onConnected);

  /**
   * Send data to the host. To be thread-safe, this must be called from a
   * dispatch to the ioService which was given to the constructor, as is done by
   * ThreadsafeFace.
   * @param data A pointer to the buffer of data to send.
   * @param dataLength The number of bytes in data.
   */
  virtual void
  send(const uint8_t *data, size_t dataLength);

  /**
   * Do nothing since the asio io_service reads the socket.
   */
  virtual void
  processEvents();

  virtual bool
  getIsConnected();

  /**
   * Close the connection to the host.
   */
  virtual void
  close();

  virtual ~AsyncTcpTransport();

private:
  boost::asio::io_service& ioService_;
  // We define SocketTransport in the source file so that we don't have to
  // include the Boost headers for boost::asio::ip::tcp in this header file.
  // (We cannot forward declare ip::tcp itself because it is an inner class.)
  class SocketTransport;
  ptr_lib::shared_ptr<SocketTransport> socketTransport_;
  ConnectionInfo connectionInfo_;
  bool isLocal_;
};

}

#endif // NDN_CPP_HAVE_BOOST_ASIO

#endif
