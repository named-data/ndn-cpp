/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_TRANSPORT_HPP
#define NDN_TRANSPORT_HPP

#include <vector>
#include "../common.hpp"

namespace ndn {

class ElementListener;

class Transport {
public:
  /**
   * A Transport::ConnectionInfo is a base class for connection information used
   * by subclasses of Transport.
   */
  class ConnectionInfo {
  public:
    virtual ~ConnectionInfo();
  };

  /**
   * Determine whether this transport connecting according to connectionInfo is
   * to a node on the current machine. This affects the processing of
   * Face.registerPrefix(): if the NFD is local, registration occurs with the
   * '/localhost/nfd...' prefix; if non-local, the library will attempt to use
   * remote prefix registration using '/localhop/nfd...'
   * @param connectionInfo A ConnectionInfo with the host to check.
   * @return True if the host is local, false if not.
   */
  virtual bool
  isLocal(const Transport::ConnectionInfo& connectionInfo);

  /**
   * Connect according to the info in ConnectionInfo, and processEvents() will
   * use elementListener.
   * @param connectionInfo A reference to an object of a subclass of
   * ConnectionInfo.
   * @param elementListener Not a shared_ptr because we assume that it will
   * remain valid during the life of this object.
   */
  virtual void
  connect(const Transport::ConnectionInfo& connectionInfo,
          ElementListener& elementListener);

  /**
   * Set data to the host
   * @param data A pointer to the buffer of data to send.
   * @param dataLength The number of bytes in data.
   */
  virtual void
  send(const uint8_t *data, size_t dataLength);

  void
  send(const std::vector<uint8_t>& data)
  {
    send(&data[0], data.size());
  }

  /**
   * Process any data to receive.  For each element received, call
   * elementListener.onReceivedElement. This is non-blocking and will return
   * immediately if there is no data to receive. You should normally not call
   * this directly since it is called by Face.processEvents.
   * @throws This may throw an exception for reading data or in the callback for
   * processing the data.  If you call this from an main event loop, you may
   * want to catch and log/disregard all exceptions.
   */
  virtual void
  processEvents() = 0;

  virtual bool
  getIsConnected();

  /**
   * Close the connection.  This base class implementation does nothing, but
   * your derived class can override.
   */
  virtual void
  close();

  virtual ~Transport();
};

}

#endif
