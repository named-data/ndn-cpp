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

#ifndef NDN_THREADSAFE_FACE_HPP
#define NDN_THREADSAFE_FACE_HPP

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_ASIO.
#include "ndn-cpp-config.h"
#ifdef NDN_CPP_HAVE_BOOST_ASIO

#include <boost/asio.hpp>
#include "face.hpp"

namespace ndn {

/**
 * A ThreadsafeFace extends Face to use a Boost asio io_service to process events
 * and schedule communication calls. You must start the service on the thread in
 * which you want the library to call communication callbacks such as onData and
 * onInterest. For usage, see the example test-get-async-threadsafe.cpp.
 */
class ThreadsafeFace : public Face {
public:
  /**
   * Create a new Face for communication with an NDN hub with the given
   * Transport object and connectionInfo.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   * @param transport A shared_ptr to a Transport object used for communication.
   * If you do not want to call processEvents, then the transport should be an
   * async transport like AsyncTcpTransport, in which case the transport should
   * use the same ioService.
   * @param transport A shared_ptr to a Transport::ConnectionInfo to be used to
   * connect to the transport.
   */
  ThreadsafeFace
    (boost::asio::io_service& ioService,
     const ptr_lib::shared_ptr<Transport>& transport,
     const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo);

  /**
   * Create a new ThreadsafeFace for communication with an NDN hub at host:port
   * using the an AsyncTcpTransport. With this constructor, you do not need to
   * call processEvents since the ioService does all processing.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   * @param host The host of the NDN hub.
   * @param port (optional) The port of the NDN hub. If omitted, use 6363.
   */
  ThreadsafeFace
    (boost::asio::io_service& ioService, const char *host,
     unsigned short port = 6363);

  /**
   * Create a new Face for communication with an NDN hub using a default
   * connection as follows. If the forwarder's Unix socket file exists, then
   * connect using AsyncUnixTransport. Otherwise, connect to "localhost" on port
   * 6363 using AsyncTcpTransport. With this constructor, you do not need to
   * call processEvents since the ioService does all processing.
   * @param ioService The asio io_service. It is the responsibility of the
   * application to start and stop the service.
   */
  ThreadsafeFace(boost::asio::io_service& ioService);

  /**
   * Get the asio io_service that was given to the constructor.
   * @return THe asio io_service.
   */
  boost::asio::io_service&
  getIoService() { return ioService_; }

  /**
   * Override to use the ioService given to the constructor to dispatch
   * expressInterest to be called in a thread-safe manner. See
   * Face.expressInterest for calling details.
   */
  virtual uint64_t
  expressInterest
    (const Interest& interest, const OnData& onData,
     const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Override to use the ioService given to the constructor to dispatch
   * expressInterest to be called in a thread-safe manner. See
   * Face.expressInterest for calling details.
   */
  virtual uint64_t
  expressInterest
    (const Name& name, const Interest *interestTemplate, const OnData& onData,
     const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  uint64_t
  expressInterest
    (const Name& name, const OnData& onData, const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    // This is needed, otherwise C++ will use the signature with
    // const Interest& by automatically converting the Name to an Interest
    // using the constructor Interest(const Name& name). Just call the Face
    // method with the same signature as this.
    return Face::expressInterest(name, onData, onTimeout, wireFormat);
  }

  /**
   * Override to use the ioService given to the constructor to dispatch
   * removePendingInterest to be called in a thread-safe manner. See
   * Face.removePendingInterest for calling details.
   */
  virtual void
  removePendingInterest(uint64_t pendingInterestId);

  /**
   * Override to use the ioService given to the constructor to dispatch
   * registerPrefix to be called in a thread-safe manner. See
   * Face.registerPrefix for calling details.
   */
  virtual uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed,
     const OnRegisterSuccess& onRegisterSuccess,
     const RegistrationOptions& registrationOptions = RegistrationOptions(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed,
     const RegistrationOptions& registrationOptions,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    // This is needed, otherwise C++ will try to use the signature with
    // const OnRegisterSuccess&. Just call the Face method with the same
    // signature as this.
    return Face::registerPrefix
      (prefix, onInterest, onRegisterFailed, registrationOptions, wireFormat);
  }

  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed)
  {
    // This is needed, otherwise C++ will try to use the signature with
    // const OnRegisterSuccess&. Just call the Face method with the same
    // signature as this.
    return Face::registerPrefix(prefix, onInterest, onRegisterFailed);
  }

  /**
   * Override to use the ioService given to the constructor to dispatch
   * removeRegisteredPrefix to be called in a thread-safe manner. See
   * Face.removeRegisteredPrefix for calling details.
   */
  virtual void
  removeRegisteredPrefix(uint64_t registeredPrefixId);

  /**
   * Override to use the ioService given to the constructor to dispatch
   * setInterestFilter to be called in a thread-safe manner. See
   * Face.setInterestFilter for calling details.
   */
  virtual uint64_t
  setInterestFilter
    (const InterestFilter& filter, const OnInterestCallback& onInterest);

  /**
   * Override to use the ioService given to the constructor to dispatch
   * setInterestFilter to be called in a thread-safe manner. See
   * Face.setInterestFilter for calling details.
   */
  virtual uint64_t
  setInterestFilter(const Name& prefix, const OnInterestCallback& onInterest);

  /**
   * Override to use the ioService given to the constructor to dispatch
   * unsetInterestFilter to be called in a thread-safe manner. See
   * Face.unsetInterestFilter for calling details.
   */
  virtual void
  unsetInterestFilter(uint64_t interestFilterId);

  /**
   * Override to use the ioService given to the constructor to dispatch putData
   * to be called in a thread-safe manner. See Face.putData for calling details.
   */
  virtual void
  putData
    (const Data& data,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Override to use the ioService given to the constructor to dispatch putNack
   * to be called in a thread-safe manner. See Face.putNack for calling details.
   */
  virtual void
  putNack(const Interest& interest, const NetworkNack& networkNack);

  /**
   * Override to use the ioService given to the constructor to dispatch send to
   * be called in a thread-safe manner. See Face.send for calling details.
   */
  virtual void
  send(const uint8_t *encoding, size_t encodingLength);

  /**
   * Override to use the ioService given to the constructor to dispatch shutdown
   * to be called in a thread-safe manner. See Face.shutdown for calling details.
   */
  virtual void
  shutdown();

  /**
   * Override to call callback() after the given delay, using a
   * basic_deadline_timer with the ioService given to the constructor. Even
   * though this is public, it is not part of the public API of Face.
   * @param delayMilliseconds The delay in milliseconds.
   * @param callback This calls callback.callback() after the delay.
   */
  virtual void
  callLater(Milliseconds delayMilliseconds, const Callback& callback);

private:
  static ptr_lib::shared_ptr<Transport>
  getDefaultTransport(boost::asio::io_service& ioService);

  static ptr_lib::shared_ptr<Transport::ConnectionInfo>
  getDefaultConnectionInfo();

  boost::asio::io_service& ioService_;
};

}

#endif // NDN_CPP_HAVE_BOOST_ASIO

#endif
