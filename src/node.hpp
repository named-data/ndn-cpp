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

#ifndef NDN_NODE_HPP
#define NDN_NODE_HPP

#include <ndn-cpp/common.hpp>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/forwarding-flags.hpp>
#include <ndn-cpp/interest-filter.hpp>
#include <ndn-cpp/face.hpp>
#include "util/command-interest-generator.hpp"
#include "encoding/element-listener.hpp"

struct ndn_Interest;

namespace ndn {

class KeyChain;

class Node : public ElementListener {
public:
  /**
   * Create a new Node for communication with an NDN hub with the given Transport object and connectionInfo.
   * @param transport A shared_ptr to a Transport object used for communication.
   * @param transport A shared_ptr to a Transport::ConnectionInfo to be used to connect to the transport.
   */
  Node(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo);

  /**
   * Send the Interest through the transport, read the entire response and call onData(interest, data).
   * @param interest A reference to the Interest.  This copies the Interest.
   * @param onData A function object to call when a matching data packet is received.  This copies the function object, so you may need to
   * use func_lib::ref() as appropriate.
   * @param onTimeout A function object to call if the interest times out.  If onTimeout is an empty OnTimeout(), this does not use it.
   * This copies the function object, so you may need to use func_lib::ref() as appropriate.
   * @param wireFormat A WireFormat object used to encode the message.
   * @return The pending interest ID which can be used with removePendingInterest.
   * @throws runtime_error If the encoded interest size exceeds
   * getMaxNdnPacketSize().
   */
  uint64_t
  expressInterest(const Interest& interest, const OnData& onData, const OnTimeout& onTimeout, WireFormat& wireFormat);

  /**
   * Remove the pending interest entry with the pendingInterestId from the pending interest table.
   * This does not affect another pending interest with a different pendingInterestId, even if it has the same interest name.
   * If there is no entry with the pendingInterestId, do nothing.
   * @param pendingInterestId The ID returned from expressInterest.
   */
  void
  removePendingInterest(uint64_t pendingInterestId);

  /**
   * Append a timestamp component and a random value component to interest's
   * name. Then use the keyChain and certificateName to sign the interest. If
   * the interest lifetime is not set, this sets it.
   * @param interest The interest whose name is append with components.
   * @param keyChain The KeyChain object for signing interests.
   * @param certificateName The certificate name for signing interests.
   * @param wireFormat A WireFormat object used to encode the SignatureInfo and
   * to encode interest name for signing.
   */
  void
  makeCommandInterest
    (Interest& interest, KeyChain& keyChain, const Name& certificateName,
     WireFormat& wireFormat)
  {
    commandInterestGenerator_.generate
      (interest, keyChain, certificateName, wireFormat);
  }

  /**
   * Register prefix with the connected NDN hub and call onInterest when a matching interest is received.
   * @param prefix A reference to a Name for the prefix to register.  This copies the Name.
   * @param onInterest (optional) If not null, this creates an interest filter
   * from prefix so that when an Interest is received which matches the filter,
   * this calls the function object
   * onInterest(prefix, interest, face, interestFilterId, filter).
   * This copies the function object, so you may need to use func_lib::ref() as
   * appropriate. If onInterest is null, it is ignored and you must call
   * setInterestFilter.
   * @param onRegisterFailed A function object to call if failed to retrieve the connected hub’s ID or failed to register the prefix.
   * This calls onRegisterFailed(prefix) where prefix is the prefix given to registerPrefix.
   * @param flags The flags for finer control of which interests are forwarded to the application.
   * @param wireFormat A WireFormat object used to encode the message.
   * @param commandKeyChain The KeyChain object for signing interests.  If null,
   * assume we are connected to a legacy NDNx forwarder.
   * @param commandCertificateName The certificate name for signing interests.
   * @param face The face which is passed to the onInterest callback. If
   * onInterest is null, this is ignored.
   * @return The registered prefix ID which can be used with removeRegisteredPrefix.
   */
  uint64_t
  registerPrefix
    (const Name& prefix, const OnInterestCallback& onInterest,
     const OnRegisterFailed& onRegisterFailed, const ForwardingFlags& flags,
     WireFormat& wireFormat, KeyChain& commandKeyChain,
     const Name& commandCertificateName, Face* face);

  /**
   * Remove the registered prefix entry with the registeredPrefixId from the
   * registered prefix table. This does not affect another registered prefix
   * with a different registeredPrefixId, even if it has the same prefix name.
   * If there is no entry with the registeredPrefixId, do nothing. If an
   * interest filter was automatically created by registerPrefix, also remove it.
   * @param registeredPrefixId The ID returned from registerPrefix.
   */
  void
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
   * @param face The face which is passed to the onInterest callback.
   * @return The interest filter ID which can be used with unsetInterestFilter.
   */
  uint64_t
  setInterestFilter
    (const InterestFilter& filter, const OnInterestCallback& onInterest,
     Face* face)
  {
    uint64_t interestFilterId = InterestFilterEntry::getNextInterestFilterId();
    interestFilterTable_.push_back(ptr_lib::make_shared<InterestFilterEntry>
      (interestFilterId, ptr_lib::make_shared<const InterestFilter>(filter),
       onInterest, face));

    return interestFilterId;
  }

  /**
   * Remove the interest filter entry which has the interestFilterId from the
   * interest filter table. This does not affect another interest filter with
   * a different interestFilterId, even if it has the same prefix name.
   * If there is no entry with the interestFilterId, do nothing.
   * @param interestFilterId The ID returned from setInterestFilter.
   */
  void
  unsetInterestFilter(uint64_t interestFilterId);

  /**
   * The OnInterestCallback calls this to put a Data packet which satisfies an
   * Interest.
   * @param data The Data packet which satisfies the interest.
   * @param wireFormat A WireFormat object used to encode the Data packet.
   * @throws runtime_error If the encoded Data packet size exceeds
   * getMaxNdnPacketSize().
   */
  void
  putData(const Data& data, WireFormat& wireFormat);

  /**
   * Send the encoded packet out through the face.
   * @param encoding The array of bytes for the encoded packet to send.
   * @param encodingLength The number of bytes in the encoding array.
   * @throws runtime_error If the encoded Data packet size exceeds
   * getMaxNdnPacketSize().
   */
  void
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
  void
  processEvents();

  const ptr_lib::shared_ptr<Transport>&
  getTransport() { return transport_; }

  const ptr_lib::shared_ptr<const Transport::ConnectionInfo>&
  getConnectionInfo() { return connectionInfo_; }

  void
  onReceivedElement(const uint8_t *element, size_t elementLength);

  /**
   * Check if the face is local based on the current connection through the
   * Transport; some Transport may cause network I/O (e.g. an IP host name lookup).
   * @return True if the face is local, false if not.
   */
  bool
  isLocal() { return transport_->isLocal(*connectionInfo_); }

  void
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

private:
  class PendingInterest {
  public:
    /**
     * Create a new PendingInterest and set the timeoutTime_ based on the current time and the interest lifetime.
     * @param pendingInterestId A unique ID for this entry, which you should get with getNextPendingInteresId().
     * @param interest A shared_ptr for the interest.
     * @param onData A function object to call when a matching data packet is received.
     * @param onTimeout A function object to call if the interest times out.  If onTimeout is an empty OnTimeout(), this does not use it.
     */
    PendingInterest
      (uint64_t pendingInterestId, const ptr_lib::shared_ptr<const Interest>& interest, const OnData& onData,
       const OnTimeout& onTimeout);

    /**
     * Return the next unique pending interest ID.
     */
    static uint64_t
    getNextPendingInterestId()
    {
      return ++lastPendingInterestId_;
    }

    /**
     * Return the pendingInterestId given to the constructor.
     */
    uint64_t
    getPendingInterestId() { return pendingInterestId_; }

    const ptr_lib::shared_ptr<const Interest>&
    getInterest() { return interest_; }

    const OnData&
    getOnData() { return onData_; }

    /**
     * Check if this interest is timed out.
     * @param nowMilliseconds The current time in milliseconds from ndn_getNowMilliseconds.
     * @return true if this interest timed out, otherwise false.
     */
    bool
    isTimedOut(MillisecondsSince1970 nowMilliseconds)
    {
      return timeoutTimeMilliseconds_ >= 0.0 && nowMilliseconds >= timeoutTimeMilliseconds_;
    }

    /**
     * Call onTimeout_ (if defined).  This ignores exceptions from the onTimeout_.
     */
    void
    callTimeout();

  private:
    ptr_lib::shared_ptr<const Interest> interest_;
    static uint64_t lastPendingInterestId_; /**< A class variable used to get the next unique ID. */
    uint64_t pendingInterestId_;            /**< A unique identifier for this entry so it can be deleted */
    const OnData onData_;
    const OnTimeout onTimeout_;
    MillisecondsSince1970 timeoutTimeMilliseconds_; /**< The time when the interest times out in milliseconds according to ndn_getNowMilliseconds, or -1 for no timeout. */
  };

  /**
   * A RegisteredPrefix holds a registeredPrefixId and information necessary
   * to remove the registration later. It optionally holds a related
   * interestFilterId if the InterestFilter was set in the same registerPrefix
   * operation.
   */
  class RegisteredPrefix {
  public:
    /**
     * Create a new RegisteredPrefix with the given values.
     * @param registeredPrefixId A unique ID for this entry, which you should get with getNextRegisteredPrefixId().
     * @param prefix A shared_ptr for the prefix.
     * @param relatedInterestFilterId (optional) The related interestFilterId
     * for the filter set in the same registerPrefix operation. If omitted, set
     * to 0.
     */
    RegisteredPrefix
      (uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
       uint64_t relatedInterestFilterId)
    : registeredPrefixId_(registeredPrefixId), prefix_(prefix),
      relatedInterestFilterId_(relatedInterestFilterId)
    {
    }

    /**
     * Return the next unique entry ID.
     * @return The next ID.
     */
    static uint64_t
    getNextRegisteredPrefixId()
    {
      return ++lastRegisteredPrefixId_;
    }

    /**
     * Return the registeredPrefixId given to the constructor.
     * @return The registeredPrefixId.
     */
    uint64_t
    getRegisteredPrefixId() { return registeredPrefixId_; }

    /**
     * Get the name prefix given to the constructor.
     * @return The name prefix.
     */
    const ptr_lib::shared_ptr<const Name>&
    getPrefix() { return prefix_; }

    /**
     * Get the related interestFilterId given to the constructor.
     * @return The related interestFilterId.
     */
    uint64_t
    getRelatedInterestFilterId() { return relatedInterestFilterId_; }

  private:
    static uint64_t lastRegisteredPrefixId_; /**< A class variable used to get the next unique ID. */
    uint64_t registeredPrefixId_;            /**< A unique identifier for this entry so it can be deleted */
    ptr_lib::shared_ptr<const Name> prefix_;
    uint64_t relatedInterestFilterId_;
  };

  /**
   * An InterestFilterEntry holds an interestFilterId, an InterestFilter and the
   * OnInterestCallback with its related Face.
   */
  class InterestFilterEntry {
  public:
    /**
     * Create a new InterestFilterEntry with the given values.
     * @param interestFilterId The ID from getNextInterestFilterId().
     * @param filter A shared_ptr for the InterestFilter for this entry.
     * @param onInterest A function object to call when a matching data packet
     * is received.
     * @param face The face on which was called registerPrefix or
     * setInterestFilter which is passed to the onInterest callback.
     */
    InterestFilterEntry
      (uint64_t interestFilterId,
       const ptr_lib::shared_ptr<const InterestFilter>& filter,
       const OnInterestCallback& onInterest, Face* face)
    : interestFilterId_(interestFilterId), filter_(filter),
      prefix_(new Name(filter->getPrefix())), onInterest_(onInterest), face_(face)
    {
    }

    /**
     * Get the next interest filter ID. This just calls
     * RegisteredPrefix::getNextRegisteredPrefixId() so that IDs come from the
     * same pool and won't be confused when removing entries from the two tables.
     * @return The next ID.
     */
    static uint64_t
    getNextInterestFilterId()
    {
      return RegisteredPrefix::getNextRegisteredPrefixId();
    }

    /**
     * Return the interestFilterId given to the constructor.
     * @return The interestFilterId.
     */
    uint64_t
    getInterestFilterId() { return interestFilterId_; }

    /**
     * Get the InterestFilter given to the constructor.
     * @return The InterestFilter.
     */
    const ptr_lib::shared_ptr<const InterestFilter>&
    getFilter() { return filter_; }

    /**
     * Get the prefix from the filter as a shared_ptr. We keep this cached value
     * so that we don't have to copy the name to pass a shared_ptr each time
     * we call the OnInterestCallback.
     * @return The filter's prefix.
     */
    const ptr_lib::shared_ptr<const Name>&
    getPrefix() { return prefix_; }

    /**
     * Get the OnInterestCallback given to the constructor.
     * @return The OnInterest callback.
     */
    const OnInterestCallback&
    getOnInterest() { return onInterest_; }

    /**
     * Get the Face given to the constructor.
     * @return The Face.
     */
    Face&
    getFace() { return *face_; }

  private:
    uint64_t interestFilterId_;  /**< A unique identifier for this entry so it can be deleted */
    ptr_lib::shared_ptr<const InterestFilter> filter_;
    ptr_lib::shared_ptr<const Name> prefix_;
    const OnInterestCallback onInterest_;
    Face* face_;
  };

  /**
   * An NdndIdFetcher receives the Data packet with the publisher public key digest for the connected NDN hub.
   * This class is a function object for the callbacks. It only holds a pointer to an Info object, so it is OK to copy the pointer.
   */
  class NdndIdFetcher {
  public:
    class Info;
    NdndIdFetcher(ptr_lib::shared_ptr<NdndIdFetcher::Info> info)
    : info_(info)
    {
    }

    /**
     * We received the ndnd ID.
     * @param interest
     * @param data
     */
    void
    operator()(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& ndndIdData);

    /**
     * We timed out fetching the ndnd ID.
     * @param interest
     */
    void
    operator()(const ptr_lib::shared_ptr<const Interest>& timedOutInterest);

    class Info {
    public:
      /**
       *
       * @param node
       * @param registeredPrefixId The RegisteredPrefix::getNextRegisteredPrefixId() which registerPrefix got so it could return it to the caller.
       * @param prefix
       * @param onInterest
       * @param onRegisterFailed
       * @param flags
       * @param wireFormat
       * @param flace
       */
      Info(Node *node, uint64_t registeredPrefixId, const Name& prefix,
           const OnInterestCallback& onInterest,
           const OnRegisterFailed& onRegisterFailed, const ForwardingFlags& flags,
           WireFormat& wireFormat, Face* face)
      : node_(*node), registeredPrefixId_(registeredPrefixId),
        prefix_(new Name(prefix)), onInterest_(onInterest),
        onRegisterFailed_(onRegisterFailed), flags_(flags),
        wireFormat_(wireFormat), face_(face)
      {
      }

      Node& node_;
      uint64_t registeredPrefixId_;
      ptr_lib::shared_ptr<const Name> prefix_;
      const OnInterestCallback onInterest_;
      const OnRegisterFailed onRegisterFailed_;
      ForwardingFlags flags_;
      WireFormat& wireFormat_;
      Face* face_;
    };

  private:
    ptr_lib::shared_ptr<Info> info_;
  };


  /**
   * A RegisterResponse receives the response Data packet from the register
   * prefix interest sent to the connected NDN hub.  If this gets a bad response
   * or a timeout, call onRegisterFailed.
   * This class is a function object for the callbacks.
   */
  class RegisterResponse {
  public:
    class Info;
    RegisterResponse(ptr_lib::shared_ptr<RegisterResponse::Info> info)
    : info_(info)
    {
    }

    /**
     * We received the response.
     * @param interest
     * @param data
     */
    void
    operator()(const ptr_lib::shared_ptr<const Interest>& interest,
               const ptr_lib::shared_ptr<Data>& responseData);

    /**
     * We timed out waiting for the response.
     * @param interest
     */
    void
    operator()(const ptr_lib::shared_ptr<const Interest>& timedOutInterest);

    class Info {
    public:
      Info(Node* node, const ptr_lib::shared_ptr<const Name>& prefix,
           const OnInterestCallback& onInterest,
           const OnRegisterFailed& onRegisterFailed,
           const ForwardingFlags& flags, WireFormat& wireFormat,
           bool isNfdCommand, Face* face)
      : node_(*node), prefix_(prefix), onInterest_(onInterest),
        onRegisterFailed_(onRegisterFailed), flags_(flags),
        wireFormat_(wireFormat), isNfdCommand_(isNfdCommand), face_(face)
      {
      }

      Node& node_;
      ptr_lib::shared_ptr<const Name> prefix_;
      const OnInterestCallback onInterest_;
      const OnRegisterFailed onRegisterFailed_;
      ForwardingFlags flags_;
      WireFormat& wireFormat_;
      bool isNfdCommand_;
      Face* face_;
    };

  private:
    ptr_lib::shared_ptr<Info> info_;
  };

  /**
   * Find all entries from pendingInterestTable_ where the name conforms to the
   * entry's interest selectors, remove the entries from the table and add to
   * the entries vector.
   * @param name The name to find the interest for (from the incoming data packet).
   * @param entries Add matching entries from pendingInterestTable_.  The caller
   * should pass in a reference to an empty vector.
   */
  void
  extractEntriesForExpressedInterest
    (const Name& name,
     std::vector<ptr_lib::shared_ptr<PendingInterest> > &entries);

  /**
   * Do the work of registerPrefix to register with NDNx once we have an ndndId_.
   * @param registeredPrefixId The RegisteredPrefix::getNextRegisteredPrefixId()
   * which registerPrefix got so it could return it to the caller. If this
   * is 0, then don't add to registeredPrefixTable_ (assuming it has already
   * been done).
   * @param prefix
   * @param onInterest
   * @param onRegisterFailed
   * @param flags
   * @param wireFormat
   * @param face
   */
  void
  registerPrefixHelper
    (uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
     const OnInterestCallback& onInterest, const OnRegisterFailed& onRegisterFailed,
     const ForwardingFlags& flags, WireFormat& wireFormat, Face* face);

  /**
   * Do the work of registerPrefix to register with NFD.
   * @param registeredPrefixId The RegisteredPrefix::getNextRegisteredPrefixId()
   * which registerPrefix got so it could return it to the caller. If this
   * is 0, then don't add to registeredPrefixTable_ (assuming it has already
   * been done).
   * @param prefix
   * @param onInterest
   * @param onRegisterFailed
   * @param flags
   * @param commandKeyChain
   * @param commandCertificateName
   * @param face
   */
  void
  nfdRegisterPrefix
    (uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
     const OnInterestCallback& onInterest, const OnRegisterFailed& onRegisterFailed,
     const ForwardingFlags& flags, KeyChain& commandKeyChain,
     const Name& commandCertificateName, WireFormat& wireFormat, Face* face);

  ptr_lib::shared_ptr<Transport> transport_;
  ptr_lib::shared_ptr<const Transport::ConnectionInfo> connectionInfo_;
  std::vector<ptr_lib::shared_ptr<PendingInterest> > pendingInterestTable_;
  std::vector<ptr_lib::shared_ptr<RegisteredPrefix> > registeredPrefixTable_;
  std::vector<ptr_lib::shared_ptr<InterestFilterEntry> > interestFilterTable_;
  Interest ndndIdFetcherInterest_;
  Blob ndndId_;
  CommandInterestGenerator commandInterestGenerator_;
  Name timeoutPrefix_;
};

}

#endif
