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

#ifndef NDN_ASYNC_UNIX_TRANSPORT_HPP
#define NDN_ASYNC_UNIX_TRANSPORT_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_ASIO.
#include "../ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_BOOST_ASIO

#include <string>
#include <boost/asio.hpp>
#include "../common.hpp"
#include "transport.hpp"

namespace ndn {

template<class AsioProtocol> class AsyncSocketTransport;

/**
 * AsyncUnixTransport extends Transport for async communication over a Unix
 * socket using Boost's asio io_service. This only uses asio for communication.
 * To make this thread-safe, you must dispatch calls to send(), etc. to the
 * io_service, as is done by ThreadsafeFace. To use this, you do not need to
 * call processEvents.
 */
class AsyncUnixTransport : public Transport {
public:
  /**
   * An AsyncUnixTransport::ConnectionInfo extends Transport::ConnectionInfo to
   * hold the file path of the Unix socket.
   */
  class ConnectionInfo : public Transport::ConnectionInfo {
  public:
    /**
     * Create a ConnectionInfo with the given filePath.
     * @param filePath The file path of the Unix socket to connect to.
     */
    ConnectionInfo(const char *filePath)
    : filePath_(filePath)
    {
    }

    /**
     * Get the file path given to the constructor.
     * @return A string reference for the file path.
     */
    const std::string&
    getFilePath() const { return filePath_; }

    virtual
    ~ConnectionInfo();

  private:
    std::string filePath_;
  };

  /**
   * Create an AsyncUnixTransport in the unconnected state. This will use the
   * ioService to create the connection and communicate asynchronously.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   */
  AsyncUnixTransport(boost::asio::io_service& ioService);

  /**
   * Determine whether this transport connecting according to connectionInfo is
   * to a node on the current machine. Unix transports are always local.
   * @param connectionInfo This is ignored.
   * @return True because Unix transports are always local.
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

  virtual ~AsyncUnixTransport();

private:
  ptr_lib::shared_ptr<AsyncSocketTransport<boost::asio::local::stream_protocol> >
    socketTransport_;
};

}

#endif // NDN_CPP_HAVE_BOOST_ASIO

#endif
