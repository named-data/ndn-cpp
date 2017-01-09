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

#ifndef NDN_ASYNC_SOCKET_TRANSPORT_HPP
#define NDN_ASYNC_SOCKET_TRANSPORT_HPP

#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <ndn-cpp/transport/transport.hpp>
#include "../c/encoding/element-reader.h"
#include "../encoding/element-listener.hpp"
#include "../util/dynamic-uint8-vector.hpp"

namespace ndn {

/**
 * AsyncSocketTransport is a helper template class for AsyncTcpTransport and
 * AsyncUnixTransport to implement common socket communication tasks using
 * Boost's asio io_service. The template class AsioProtocol should be
 * boost::asio::ip::tcp or boost::asio::local::stream_protocol (for a Unix
 * socket). Boost uses template classes instead of base classes with override,
 * so we implement all methods here in the header file.
 */
template<class AsioProtocol> class AsyncSocketTransport {
public:
  /**
   * Create an AsyncSocketTransport in the unconnected state. This will use the
   * ioService to create the connection and communicate asynchronously.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   */
  AsyncSocketTransport(boost::asio::io_service& ioService)
  : ioService_(ioService), socket_(new typename AsioProtocol::socket(ioService)),
    elementBuffer_(new DynamicUInt8Vector(1000)), isConnected_(false)
  {
    ndn_ElementReader_initialize(&elementReader_, 0, elementBuffer_.get());
  }

  /**
   * Connect according to the info in connectionInfo, and use elementListener.
   * To be thread-safe, this must be called from a dispatch to the ioService
   * which was given to the constructor, as is done by ThreadsafeFace.
   * @param endPoint The asio endpoint for the protocol containing the
   * connection info.
   * @param elementListener Not a shared_ptr because we assume that it will
   * remain valid during the life of this object.
   * @param onConnected This calls onConnected() when the connection is
   * established.
   */
  void
  connect
    (const typename AsioProtocol::endpoint& endPoint,
     ElementListener& elementListener, const Transport::OnConnected& onConnected)
  {
    close();

    ndn_ElementReader_reset(&elementReader_, &elementListener);

    socket_->async_connect
      (endPoint,
       boost::bind(&AsyncSocketTransport::connectHandler, this, _1, onConnected));
  }

  /**
   * Set data to the host. To be thread-safe, this must be called from a
   * dispatch to the ioService which was given to the constructor, as is done by
   * ThreadsafeFace.
   * @param data A pointer to the buffer of data to send.
   * @param dataLength The number of bytes in data.
   */
  void
  send(const uint8_t *data, size_t dataLength)
  {
    if (!isConnected_)
      throw std::runtime_error
        ("AsyncSocketTransport.send: The socket is not connected");

    // Assume that this is called from a dispatch so that we are already in the
    // ioService_ thread. Just do a blocking write.
    boost::system::error_code errorCode;
    boost::asio::write
      (*socket_, boost::asio::buffer(data, dataLength), errorCode);
    if (errorCode != boost::system::errc::success)
      throw std::runtime_error("AsyncSocketTransport.send: Error in write");
  }

  bool
  getIsConnected()
  {
    return isConnected_;
  }

  /**
   * Close the connection to the host.
   */
  void
  close()
  {
    try {
      socket_->close();
    }
    catch (...) {
      // Ignore any exceptions.
    }

    isConnected_ = false;
  }

private:
  /**
   * This is called by async_connect to do the first async_receive.
   */
  void
  connectHandler
    (const boost::system::error_code& errorCode,
     const Transport::OnConnected& onConnected)
  {
    if (errorCode != boost::system::errc::success)
      // TODO: How to report errors to the application?
      throw std::runtime_error("AsyncSocketTransport: Error in async_connect");

    isConnected_ = true;
    onConnected();

    socket_->async_receive
      (boost::asio::buffer(receiveBuffer_, sizeof(receiveBuffer_)), 0,
       boost::bind(&AsyncSocketTransport::readHandler, this, _1, _2));
  }

  /**
   * This is called by async_receive to call elementReader_.onReceivedData and
   * to call itself again.
   */
  void
  readHandler(const boost::system::error_code& errorCode, size_t nBytesReceived)
  {
    if (errorCode != boost::system::errc::success) {
      if (errorCode == boost::system::errc::operation_canceled)
        // Assume the socket has been closed. Do nothing.
        return;

      close();
      // TODO: How to report errors to the application?
      throw std::runtime_error("AsyncSocketTransport: Error in async_receive");
    }

    ndn_Error error;
    if ((error = ndn_ElementReader_onReceivedData
         (&elementReader_, receiveBuffer_, nBytesReceived)))
      throw std::runtime_error(ndn_getErrorString(error));

    // Request another async receive to loop back to here.
    if (socket_->is_open())
      socket_->async_receive
        (boost::asio::buffer(receiveBuffer_, sizeof(receiveBuffer_)), 0,
         boost::bind(&AsyncSocketTransport::readHandler, this, _1, _2));
  }

  boost::asio::io_service& ioService_;
  ptr_lib::shared_ptr<typename AsioProtocol::socket> socket_;
  uint8_t receiveBuffer_[MAX_NDN_PACKET_SIZE];
  ptr_lib::shared_ptr<DynamicUInt8Vector> elementBuffer_;
  ndn_ElementReader elementReader_;
  bool isConnected_;
};

}

#endif
