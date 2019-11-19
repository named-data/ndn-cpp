/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2019 Regents of the University of California.
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
#include "network-nack.hpp"
#include "registration-options.hpp"
#include "encoding/wire-format.hpp"
#include "interest-filter.hpp"
#include "transport/transport.hpp"

namespace ndn {

class Face;

/**
 * An OnData function object is used to pass a callback to expressInterest.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)> OnData;

/**
 * An OnTimeout function object is used to pass a callback to expressInterest.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Interest>& interest)> OnTimeout;

/**
 * An OnNetworkNack function object is used to pass a callback to expressInterest.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<NetworkNack>& networkNack)> OnNetworkNack;

/**
 * An OnInterestCallback function object is used to pass a callback to
 * setInterestFilter and optionally to registerPrefix.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)> OnInterestCallback;

/**
 * An OnRegisterFailed function object is used to report when registerPrefix fails.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Name>& prefix)> OnRegisterFailed;

/**
 * An OnRegisterSuccess function object is used to report when registerPrefix succeeds.
 */
typedef func_lib::function<void
  (const ptr_lib::shared_ptr<const Name>& prefix,
   uint64_t registeredPrefixId)> OnRegisterSuccess;

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
   * @param port (optional) The port of the NDN hub. If omitted, use 6363.
   */
  Face(const char *host, unsigned short port = 6363);

  /**
   * Create a new Face for communication with an NDN hub using a default
   * connection as follows. If the forwarder's Unix socket file exists, then
   * connect using UnixTransport. Otherwise, connect to "localhost" on port
   * 6363 using TcpTransport.
   */
  Face();

  virtual ~Face();

  /**
   * Enable or disable Interest loopback. If Interest loopback is enabled, then 
   * an Interest to expressInterest is also sent to each of the matching
   * OnInterest callbacks that the application gave to registerPrefix or
   * setInterestFilter, and a Data that the application gives to putData can
   * satisfy pending Interests. This way one part of an application can do
   * Interest/Data exchange with another part through the same Face. Interest
   * loopback is disabled by default.
   * @param interestLoopbackEnabled If True, enable Interest loopback, otherwise
   * disable it.
   */
  void
  setInterestLoopbackEnabled(bool interestLoopbackEnabled);

  /**
   * Send the Interest through the transport, read the entire response and call
   * onData, onTimeout or onNetworkNack as described below.
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is an empty OnTimeout(), this does
   * not use it. This copies the function object, so you may need to use
   * func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onNetworkNack When a network Nack packet for the interest is
   * received and onNetworkNack is not null, this calls
   * onNetworkNack(interest, networkNack) and does not call onTimeout. However,
   * if a network Nack is received and onNetworkNack is an empty OnNetworkNack(),
   * do nothing and wait for the interest to time out. This copies the function
   * object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  virtual uint64_t
  expressInterest
    (const Interest& interest, const OnData& onData,
     const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Encode name as an Interest. If interestTemplate is not 0, use its interest
   * selectors.
   * Send the Interest through the transport, read the entire response and call
   * onData, onTimeout or onNetworkNack as described below.
   * @param name A reference to a Name for the interest.  This copies the Name.
   * @param interestTemplate if not 0, copy interest selectors from the template.   This does not keep a pointer to the Interest object.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is an empty OnTimeout(), this does
   * not use it. This copies the function object, so you may need to use
   * func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onNetworkNack When a network Nack packet for the interest is
   * received and onNetworkNack is not null, this calls
   * onNetworkNack(interest, networkNack) and does not call onTimeout. However,
   * if a network Nack is received and onNetworkNack is an empty OnNetworkNack(),
   * do nothing and wait for the interest to time out. This copies the function
   * object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  virtual uint64_t
  expressInterest
    (const Name& name, const Interest *interestTemplate, const OnData& onData,
     const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Encode name as an Interest, using a default interest lifetime.
   * Send the Interest through the transport, read the entire response and call
   * onData, onTimeout or onNetworkNack as described below.
   * @param name A reference to a Name for the interest.  This copies the Name.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is an empty OnTimeout(), this does
   * not use it. This copies the function object, so you may need to use
   * func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onNetworkNack When a network Nack packet for the interest is
   * received and onNetworkNack is not null, this calls
   * onNetworkNack(interest, networkNack) and does not call onTimeout. However,
   * if a network Nack is received and onNetworkNack is an empty OnNetworkNack(),
   * do nothing and wait for the interest to time out. This copies the function
   * object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds getMaxNdnPacketSize().
   */
  uint64_t
  expressInterest
    (const Name& name, const OnData& onData,
     const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return expressInterest(name, 0, onData, onTimeout, onNetworkNack, wireFormat);
  }

  /**
   * Send the Interest through the transport, read the entire response and call
   * onData or onTimeout as described below.
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout (optional) If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is omitted or an empty OnTimeout(),
   * this does not use it. This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  uint64_t
  expressInterest
    (const Interest& interest, const OnData& onData,
     const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return expressInterest
      (interest, onData, onTimeout, OnNetworkNack(), wireFormat);
  }

  /**
   * Encode name as an Interest. If interestTemplate is not 0, use its interest
   * selectors.
   * Send the Interest through the transport, read the entire response and call
   * onData or onTimeout as described below.
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout (optional) If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is omitted or an empty OnTimeout(),
   * this does not use it. This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  uint64_t
  expressInterest
    (const Name& name, const Interest *interestTemplate, const OnData& onData,
     const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return expressInterest
      (name, interestTemplate, onData, onTimeout, OnNetworkNack(), wireFormat);
  }

  /**
   * Encode name as an Interest, using a default interest lifetime.
   * Send the Interest through the transport, read the entire response and call
   * onData or onTimeout as described below.
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData  When a matching data packet is received, this calls
   * onData(interest, data) where interest is the interest given to
   * expressInterest and data is the received Data object. This copies the
   * function object, so you may need to use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onTimeout (optional) If the interest times out according to the interest
   * lifetime, this calls onTimeout(interest) where interest is the interest
   * given to expressInterest. If onTimeout is omitted or an empty OnTimeout(),
   * this does not use it. This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  uint64_t
  expressInterest
    (const Name& name, const OnData& onData,
     const OnTimeout& onTimeout = OnTimeout(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return expressInterest
      (name, 0, onData, onTimeout, OnNetworkNack(), wireFormat);
  }

  /**
   * Remove the pending interest entry with the pendingInterestId from the pending interest table.
   * This does not affect another pending interest with a different pendingInterestId, even if it has the same interest name.
   * If there is no entry with the pendingInterestId, do nothing.
   * @param pendingInterestId The ID returned from expressInterest.
   */
  virtual void
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

  KeyChain*
  getCommandKeyChain() { return commandKeyChain_; }

  const Name&
  getCommandCertificateName() { return commandCertificateName_; }

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
  virtual void
  makeCommandInterest
    (Interest& interest,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Register prefix with the connected NDN hub and call onInterest when a
   * matching interest is received. To register a prefix with NFD, you must
   * first call setCommandSigningInfo.
   * @param prefix A reference to a Name for the prefix to register.  This copies the Name.
   * @param onInterest (optional) If not an empty OnInterestCallback(), this
   * creates an interest filter from prefix so that when an Interest is received
   * which matches the filter, this calls the function object
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * This copies the function object, so you may need to use func_lib::ref() as
   * appropriate. If onInterest is an empty OnInterestCallback(), it is ignored
   * and you must call setInterestFilter.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onRegisterFailed A function object to call if failed to retrieve the connected hub’s ID or failed to register the prefix.
   * This calls onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onRegisterSuccess (optional) A function object to call registerPrefix
   * receives a success message from the forwarder. This calls
   * onRegisterSuccess(prefix, registeredPrefixId) where  prefix and
   * registeredPrefixId are the values given to registerPrefix. If
   * onRegisterSuccess is an empty OnRegisterSuccess(), this does not use it.
   * (The onRegisterSuccess parameter comes after onRegisterFailed because it
   * can be empty or omitted, unlike onRegisterFailed.)
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param registrationOptions (optional) The registration options for finer
   * control of how to forward an interest and other options. If omitted, use
   * the default RegistrationOptions constructor.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The registered prefix ID which can be used with removeRegisteredPrefix.
   */
  virtual uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed,
     const OnRegisterSuccess& onRegisterSuccess,
     const RegistrationOptions& registrationOptions = RegistrationOptions(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Register prefix with the connected NDN hub and call onInterest when a
   * matching interest is received. To register a prefix with NFD, you must
   * first call setCommandSigningInfo.
   * @param prefix A reference to a Name for the prefix to register.  This copies the Name.
   * @param onInterest (optional) If not an empty OnInterestCallback(), this
   * creates an interest filter from prefix so that when an Interest is received
   * which matches the filter, this calls the function object
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * This copies the function object, so you may need to use func_lib::ref() as
   * appropriate. If onInterest is an empty OnInterestCallback(), it is ignored
   * and you must call setInterestFilter.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onRegisterFailed A function object to call if failed to retrieve the connected hub’s ID or failed to register the prefix.
   * This calls onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param registrationOptions The registration options for finer control of
   * how to forward an interest and other options.
   * @param wireFormat (optional) A WireFormat object used to encode the message.
   * If omitted, use WireFormat getDefaultWireFormat().
   * @return The registered prefix ID which can be used with removeRegisteredPrefix.
   */
  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed,
     const RegistrationOptions& registrationOptions,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return registerPrefix
      (prefix, onInterest, onRegisterFailed, OnRegisterSuccess(), registrationOptions,
       wireFormat);
  }

  /**
   * Register prefix with the connected NDN hub and call onInterest when a
   * matching interest is received. To register a prefix with NFD, you must
   * first call setCommandSigningInfo.
   * @param prefix A reference to a Name for the prefix to register.  This copies the Name.
   * @param onInterest (optional) If not an empty OnInterestCallback(), this
   * creates an interest filter from prefix so that when an Interest is received
   * which matches the filter, this calls the function object
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * This copies the function object, so you may need to use func_lib::ref() as
   * appropriate. If onInterest is an empty OnInterestCallback(), it is ignored
   * and you must call setInterestFilter.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onRegisterFailed A function object to call if failed to retrieve the connected hub’s ID or failed to register the prefix.
   * This calls onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return The registered prefix ID which can be used with removeRegisteredPrefix.
   */
  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed)
  {
    return registerPrefix
      (prefix, onInterest, onRegisterFailed, OnRegisterSuccess(),
       RegistrationOptions(), *WireFormat::getDefaultWireFormat());
  }

  /**
   * Remove the registered prefix entry with the registeredPrefixId from the
   * registered prefix table. This does not affect another registered prefix
   * with a different registeredPrefixId, even if it has the same prefix name.
   * If an interest filter was automatically created by registerPrefix, also
   * remove it. If there is no entry with the registeredPrefixId, do nothing.
   * @param registeredPrefixId The ID returned from registerPrefix.
   */
  virtual void
  removeRegisteredPrefix(uint64_t registeredPrefixId);

  /**
   * Add an entry to the local interest filter table to call the onInterest
   * callback for a matching incoming Interest. This method only modifies the
   * library's local callback table and does not register the prefix with the
   * forwarder. It will always succeed. To register a prefix with the forwarder,
   * use registerPrefix.
   * @param filter The InterestFilter with a prefix and optional regex filter
   * used to match the name of an incoming Interest. This makes a copy of filter.
   * @param onInterest When an Interest is received which matches the filter,
   * this calls
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return The interest filter ID which can be used with unsetInterestFilter.
   */
  virtual uint64_t
  setInterestFilter
    (const InterestFilter& filter, const OnInterestCallback& onInterest);

  /**
   * Add an entry to the local interest filter table to call the onInterest
   * callback for a matching incoming Interest. This method only modifies the
   * library's local callback table and does not register the prefix with the
   * forwarder. It will always succeed. To register a prefix with the forwarder,
   * use registerPrefix.
   * @param prefix The Name prefix used to match the name of an incoming
   * Interest.
   * @param onInterest This creates an interest filter from prefix so that when
   * an Interest is received which matches the filter, this calls
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return The interest filter ID which can be used with unsetInterestFilter.
   */
  virtual uint64_t
  setInterestFilter(const Name& prefix, const OnInterestCallback& onInterest);

  /**
   * Remove the interest filter entry which has the interestFilterId from the
   * interest filter table. This does not affect another interest filter with
   * a different interestFilterId, even if it has the same prefix name.
   * If there is no entry with the interestFilterId, do nothing.
   * @param interestFilterId The ID returned from setInterestFilter.
   */
  virtual void
  unsetInterestFilter(uint64_t interestFilterId);

  /**
   * The OnInterestCallback calls this to put a Data packet which satisfies an
   * Interest.
   * @param data The Data packet which satisfies the interest.
   * @param wireFormat (optional) A WireFormat object used to encode the Data
   * packet. If omitted, use WireFormat getDefaultWireFormat().
   * @throws runtime_error If the encoded Data packet size exceeds
   * getMaxNdnPacketSize().
   */
  virtual void
  putData
    (const Data& data,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * The OnInterest callback can call this to put a Nack for the received Interest.
   * @note This method is an experimental feature, and the API may change.
   * @param interest The Interest to put in the Nack packet.
   * @param networkNack The NetworkNack with the reason code. For example,
   * NetworkNack().setReason(ndn_NetworkNackReason_NO_ROUTE).
   * @throws runtime_error If the encoded Nack packet size exceeds
   * getMaxNdnPacketSize().
   */
  virtual void
  putNack(const Interest& interest, const NetworkNack& networkNack);

  /**
   * Send the encoded packet out through the face.
   * @param encoding The blob with the the encoded packet to send.
   * @throws runtime_error If the encoded Data packet size exceeds
   * getMaxNdnPacketSize().
   */
  void
  send(const Blob& encoding)
  {
    send(encoding.buf(), encoding.size());
  }

  /**
   * Send the encoded packet out through the face.
   * @param encoding The array of bytes for the encoded packet to send.
   * @param encodingLength The number of bytes in the encoding array.
   * @throws runtime_error If the encoded Data packet size exceeds
   * getMaxNdnPacketSize().
   */
  virtual void
  send(const uint8_t *encoding, size_t encodingLength);

  /**
   * Process any packets to receive and call callbacks such as onData,
   * onInterest or onTimeout. This returns immediately if there is no data to
   * receive. This blocks while calling the callbacks. You should repeatedly
   * call this from an event loop, with calls to sleep as needed so that the
   * loop doesn’t use 100% of the CPU. Since processEvents modifies the pending
   * interest table, your application should make sure that it calls
   * processEvents in the same thread as expressInterest (which also modifies
   * the pending interest table).
   * @throws This may throw an exception for reading data or in the callback for processing the data.  If you
   * call this from an main event loop, you may want to catch and log/disregard all exceptions.
   */
  virtual void
  processEvents();

  /**
   * Check if the face is local based on the current connection through the
   * Transport; some Transport may cause network I/O (e.g. an IP host name lookup).
   * @return True if the face is local, false if not.
   * @note This is an experimental feature. This API may change in the future.
   */
  virtual bool
  isLocal();

  /**
   * Shut down and disconnect this Face.
   */
  virtual void
  shutdown();

  /**
   * Get the practical limit of the size of a network-layer packet. If a packet
   * is larger than this, the library or application MAY drop it. This is a
   * static inline method wrapping a const, so you can to use as a constant, e.g.:
   * uint8_t buffer[Face::getMaxNdnPacketSize()].
   * @return The maximum NDN packet size.
   */
  static size_t
  getMaxNdnPacketSize() { return MAX_NDN_PACKET_SIZE; }

  /**
   * Face::Callback is used internally in callLater.
   */
  typedef func_lib::function<void()> Callback;

  /**
   * Call callback() after the given delay. Even though this is public,
   * it is not part of the public API of Face. This default implementation just
   * calls Node::callLater, but a subclass can override.
   * @param delayMilliseconds The delay in milliseconds.
   * @param callback This calls callback.callback() after the delay.
   */
  virtual void
  callLater(Milliseconds delayMilliseconds, const Callback& callback);

protected:
  /**
   * If the forwarder's Unix socket file path exists, then return the file path.
   * Otherwise return an empty string.
   * @return The Unix socket file path to use, or an empty string.
   */
  static std::string
  getUnixSocketFilePathForLocalhost();

  /**
   * Do the work of expressInterest to make an Interest based on name and
   * interestTemplate.
   * @param name A reference to a Name for the interest.  This copies the Name.
   * @param interestTemplate if not 0, copy interest selectors from the template.
   * This does not keep a pointer to the Interest object.
   * @return A shared_ptr of the Interest, suitable for Node.expressInterest.
   */
  static ptr_lib::shared_ptr<const Interest>
  getInterestCopy(const Name& name, const Interest *interestTemplate)
  {
    if (interestTemplate) {
      // Copy the interestTemplate.
      ptr_lib::shared_ptr<Interest> interestCopy(new Interest(*interestTemplate));
      interestCopy->setName(name);
      return interestCopy;
    }
    else
      return ptr_lib::make_shared<Interest>(name, 4000.0);
  }

  Node *node_;
  KeyChain* commandKeyChain_;
  Name commandCertificateName_;

private:
  // Disable the copy constructor and assignment operator.
  Face(const Face& other);
  Face& operator=(const Face& other);

  static ptr_lib::shared_ptr<Transport>
  getDefaultTransport();

  static ptr_lib::shared_ptr<Transport::ConnectionInfo>
  getDefaultConnectionInfo();
};

}

#endif
