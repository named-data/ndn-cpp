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

#ifndef NDN_TCPTRANSPORT_HPP
#define NDN_TCPTRANSPORT_HPP

#include <string>
#include "../common.hpp"
#include "transport.hpp"

struct ndn_TcpTransport;
struct ndn_ElementReader;

namespace ndn {
  
class TcpTransport : public Transport {
public:
  /**
   * A TcpTransport::ConnectionInfo extends Transport::ConnectionInfo to hold the host and port info for the TCP connection.
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

  TcpTransport();
  
  /**
   * Connect according to the info in ConnectionInfo, and processEvents() will use elementListener.
   * @param connectionInfo A reference to a TcpTransport::ConnectionInfo.
   * @param elementListener Not a shared_ptr because we assume that it will remain valid during the life of this object.
   */
  virtual void connect(const Transport::ConnectionInfo& connectionInfo, ElementListener& elementListener);
  
  /**
   * Set data to the host
   * @param data A pointer to the buffer of data to send.
   * @param dataLength The number of bytes in data.
   */
  virtual void send(const uint8_t *data, size_t dataLength);

  /**
   * Process any data to receive.  For each element received, call elementListener.onReceivedElement.
   * This is non-blocking and will return immediately if there is no data to receive.
   * You should normally not call this directly since it is called by Face.processEvents.
   * @throw This may throw an exception for reading data or in the callback for processing the data.  If you
   * call this from an main event loop, you may want to catch and log/disregard all exceptions.
   */
  virtual void processEvents();

  virtual bool getIsConnected();

  /**
   * Close the connection to the host.
   */
  virtual void close();
  
  ~TcpTransport();
  
private:
  ptr_lib::shared_ptr<struct ndn_TcpTransport> transport_;
  bool isConnected_;
  // TODO: This belongs in the socket listener.
  ptr_lib::shared_ptr<struct ndn_ElementReader> elementReader_;
};

}

#endif
