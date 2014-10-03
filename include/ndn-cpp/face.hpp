/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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

#ifndef NDN_FACE_HPP
#define NDN_FACE_HPP

#include "interest.hpp"
#include "data.hpp"
#include "forwarding-flags.hpp"
#include "encoding/wire-format.hpp"
#include <ndn-cpp/transport/transport.hpp>

namespace ndn {

/**
 * An OnData function object is used to pass a callback to expressInterest.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<const Interest>&, const ptr_lib::shared_ptr<Data>&)> OnData;

/**
 * An OnTimeout function object is used to pass a callback to expressInterest.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<const Interest>&)> OnTimeout;

/**
 * An OnInterest function object is used to pass a callback to registerPrefix.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Name>&, const ptr_lib::shared_ptr<const Interest>&, Transport&, uint64_t)> OnInterest;

/**
 * An OnRegisterFailed function object is used to report when registerPrefix fails.
 */
typedef func_lib::function<void(const ptr_lib::shared_ptr<const Name>&)> OnRegisterFailed;

class Node;
class KeyChain;

/**
 * The Face class provides the main methods for NDN communication.
 */
class Face {
public:
  /**
   * Create a new Face for communication with an NDN hub with the given Transport object and connectionInfo.
   * @param transport A shared_ptr to a Transport object used for communication.
   * @param transport A shared_ptr to a Transport::ConnectionInfo to be used to connect to the transport.
   */
  Face(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo);

  /**
   * Create a new Face for communication with an NDN hub at host:port using the
   * default TcpTransport.
   * @param host The host of the NDN hub.
   * @param port (optional) The port of the NDN hub. If omitted. use 6363.
   */
  Face(const char *host, unsigned short port = 6363);

  /**
   * Create a new Face for communication with an NDN hub using a default
   * connection as follows. If the forwarder's Unix socket file exists, then
   * connect using UnixTransport. Otherwise, connect to "localhost" on port
   * 6363 using TcpTransport.
   */
  Face();

  ~Face();

  /**
   * Send the Interest through the transport, read the entire response and call onData(interest, data).
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData A function object to call when a matching data packet is received.  This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * @param onTimeout A function object to call if the interest times out.  If onTimeout is an empty OnTimeout(), this does not use it.
   * This copies the function object, so you may need to use func_lib::ref() as appropriate.
   * @param wireFormat A WireFormat object used to encode the message. If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   */
  uint64_t
  expressInterest
    (const Interest& interest, const OnData& onData, const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Encode name as an Interest. If interestTemplate is not 0, use its interest selectors.
   * Send the interest through the transport, read the entire response and call onData(interest, data).
   * @param name A reference to a Name for the interest.  This copies the Name.
   * @param interestTemplate if not 0, copy interest selectors from the template.   This does not keep a pointer to the Interest object.
   * @param onData A function object to call when a matching data packet is received.  This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * @param onTimeout A function object to call if the interest times out.  If onTimeout is an empty OnTimeout(), this does not use it.
   * This copies the function object, so you may need to use func_lib::ref() as appropriate.
   * @param wireFormat A WireFormat object used to encode the message. If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   */
  uint64_t
  expressInterest
    (const Name& name, const Interest *interestTemplate, const OnData& onData, const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Encode name as an Interest, using a default interest lifetime.
   * Send the interest through the transport, read the entire response and call onData(interest, data).
   * @param name A reference to a Name for the interest.  This copies the Name.
   * @param onData A function object to call when a matching data packet is received.  This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * @param onTimeout A function object to call if the interest times out.  If onTimeout is an empty OnTimeout(), this does not use it.
   * This copies the function object, so you may need to use func_lib::ref() as appropriate.
   * @param wireFormat A WireFormat object used to encode the message. If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   */
  uint64_t
  expressInterest
    (const Name& name, const OnData& onData, const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return expressInterest(name, 0, onData, onTimeout, wireFormat);
  }

  /**
   * Remove the pending interest entry with the pendingInterestId from the pending interest table.
   * This does not affect another pending interest with a different pendingInterestId, even if it has the same interest name.
   * If there is no entry with the pendingInterestId, do nothing.
   * @param pendingInterestId The ID returned from expressInterest.
   */
  void
  removePendingInterest(uint64_t pendingInterestId);

  /**
   * Set the KeyChain and certificate name used to sign command interests
   * (e.g. for registerPrefix).
   * @param keyChain The KeyChain object for signing interests, which
   * must remain valid for the life of this Face. You must create the KeyChain
   * object and pass it in. You can create a default KeyChain for your
   * system with the default KeyChain constructor.
   * @param certificateName The certificate name for signing interests.
   * This makes a copy of the Name. You can get the default certificate name
   * with keyChain.getDefaultCertificateName() .
   */
  void
  setCommandSigningInfo(KeyChain& keyChain, const Name& certificateName)
  {
    commandKeyChain_ = &keyChain;
    commandCertificateName_ = certificateName;
  }

  /**
   * Set the certificate name used to sign command interest (e.g. for
   * registerPrefix), using the KeyChain that was set with setCommandSigningInfo.
   * @param certificateName The certificate name for signing interest.
   * This makes a copy of the Name.
   */
  void
  setCommandCertificateName(const Name& certificateName)
  {
    commandCertificateName_ = certificateName;
  }

  /**
   * Append a timestamp component and a random value component to interest's
   * name. Then use the keyChain and certificateName from setCommandSigningInfo
   * to sign the interest. If the interest lifetime is not set, this sets it.
   * @param interest The interest whose name is appended with components.
   * @param wireFormat A WireFormat object used to encode the SignatureInfo and
   * to encode the interest name for signing. If omitted, use
   * WireFormat getDefaultWireFormat().
   * @note This method is an experimental feature. See the API docs for more detail at
   * http://named-data.net/doc/ndn-ccl-api/face.html#face-makecommandinterest-method .
   */
  void
  makeCommandInterest
    (Interest& interest,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Register prefix with the connected NDN hub and call onInterest when a
   * matching interest is received. If you have not called setCommandSigningInfo,
   * this assumes you are connecting to NDNx. If you have called
   * setCommandSigningInfo, this first sends an NFD registration request, and if
   * that times out then this sends an NDNx registration request. If you need to
   * register a prefix with NFD, you must first call setCommandSigningInfo.
   * @param prefix A reference to a Name for the prefix to register.  This copies the Name.
   * @param onInterest A function object to call when a matching interest is received.  This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * @param onRegisterFailed A function object to call if failed to retrieve the connected hub’s ID or failed to register the prefix.
   * This calls onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * @param flags The flags for finer control of which interests are forward to the application.  If omitted, use
   * the default flags defined by the default ForwardingFlags constructor.
   * @param wireFormat A WireFormat object used to encode the message. If omitted, use WireFormat getDefaultWireFormat().
   * @return The registered prefix ID which can be used with removeRegisteredPrefix.
   */
  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterest& onInterest, const OnRegisterFailed& onRegisterFailed,
     const ForwardingFlags& flags = ForwardingFlags(), WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Remove the registered prefix entry with the registeredPrefixId from the registered prefix table.
   * This does not affect another registered prefix with a different registeredPrefixId, even if it has the same prefix name.
   * If there is no entry with the registeredPrefixId, do nothing.
   * @param registeredPrefixId The ID returned from registerPrefix.
   */
  void
  removeRegisteredPrefix(uint64_t registeredPrefixId);

  /**
   * Process any packets to receive and call callbacks such as onData,
   * onInterest or onTimeout. This returns immediately if there is no data to
   * receive. This blocks while calling the callbacks. You should repeatedly
   * call this from an event loop, with calls to sleep as needed so that the
   * loop doesn’t use 100% of the CPU. Since processEvents modifies the pending
   * interest table, your application should make sure that it calls
   * processEvents in the same thread as expressInterest (which also modifies
   * the pending interest table).
   * @throw This may throw an exception for reading data or in the callback for processing the data.  If you
   * call this from an main event loop, you may want to catch and log/disregard all exceptions.
   */
  void
  processEvents();

  /**
   * Shut down and disconnect this Face.
   */
  void
  shutdown();

private:
  Node *node_;
  KeyChain* commandKeyChain_;
  Name commandCertificateName_;
};

}

#endif
