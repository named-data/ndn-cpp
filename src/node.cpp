/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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

#include <stdexcept>
#include <ndn-cpp/encoding/tlv-wire-format.hpp>
#include <ndn-cpp/control-response.hpp>
#include <ndn-cpp/lite/encoding/tlv-0_2-wire-format-lite.hpp>
#include <ndn-cpp/lite/lp/lp-packet-lite.hpp>
#include <ndn-cpp/network-nack.hpp>
#include "c/util/time.h"
#include "encoding/tlv-decoder.hpp"
#include <ndn-cpp/util/logging.hpp>
#include "lp/lp-packet.hpp"
#include "node.hpp"

INIT_LOGGER("ndn.Node");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

Node::Node(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo)
: transport_(transport), connectionInfo_(connectionInfo),
  timeoutPrefix_(Name("/local/timeout")),
  lastEntryId_(0), connectStatus_(ConnectStatus_UNCONNECTED),
  registeredPrefixTable_(interestFilterTable_),
  nonceTemplate_((const uint8_t*)"\0\0\0\0", 4)
{
}

void
Node::expressInterest
  (uint64_t pendingInterestId,
   const ptr_lib::shared_ptr<const Interest>& interestCopy, const OnData& onData,
   const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
   WireFormat& wireFormat, Face* face)
{
  // Set the nonce in our copy of the Interest so it is saved in the PIT.
  const_cast<Interest*>(interestCopy.get())->setNonce(nonceTemplate_);
  const_cast<Interest*>(interestCopy.get())->refreshNonce();

  if (connectStatus_ == ConnectStatus_CONNECT_COMPLETE) {
    // We are connected. Simply send the interest.
    expressInterestHelper
      (pendingInterestId, interestCopy, onData, onTimeout, onNetworkNack,
       &wireFormat, face);
    return;
  }

  // TODO: Properly check if we are already connected to the expected host.
  if (!transport_->isAsync()) {
    // The simple case: Just do a blocking connect and express.
    transport_->connect(*connectionInfo_, *this, Transport::OnConnected());
    expressInterestHelper
      (pendingInterestId, interestCopy, onData, onTimeout, onNetworkNack,
       &wireFormat, face);
    // Make future calls to expressInterest send directly to the Transport.
    connectStatus_ = ConnectStatus_CONNECT_COMPLETE;

    return;
  }

  // Handle the async case.
  if (connectStatus_ == ConnectStatus_UNCONNECTED) {
    connectStatus_ = ConnectStatus_CONNECT_REQUESTED;

    // expressInterestHelper will be called by onConnected.
    onConnectedCallbacks_.push_back(bind
      (&Node::expressInterestHelper, this, pendingInterestId, interestCopy,
       onData, onTimeout, onNetworkNack, &wireFormat, face));

    transport_->connect
      (*connectionInfo_, *this, bind(&Node::onConnected, this));
  }
  else if (connectStatus_ == ConnectStatus_CONNECT_REQUESTED) {
    // Still connecting. add to the interests to express by onConnected.
    onConnectedCallbacks_.push_back(bind
      (&Node::expressInterestHelper, this, pendingInterestId, interestCopy,
       onData, onTimeout, onNetworkNack, &wireFormat, face));
  }
  else
    // Don't expect this to happen.
    throw runtime_error("Node: Unrecognized connectStatus_");
}

void
Node::onConnected()
{
  // Assume that further calls to expressInterest dispatched to the event loop
  // are queued and won't enter expressInterest until this method completes and
  // sets CONNECT_COMPLETE.
  // Call each callback added while the connection was opening.
  for (size_t i = 0; i < onConnectedCallbacks_.size(); ++i)
    onConnectedCallbacks_[i]();
  onConnectedCallbacks_.clear();

  // Make future calls to expressInterest send directly to the Transport.
  connectStatus_ = ConnectStatus_CONNECT_COMPLETE;
}

void
Node::registerPrefix
  (uint64_t registeredPrefixId,
   const ptr_lib::shared_ptr<const Name>& prefixCopy,
   const OnInterestCallback& onInterest,
   const OnRegisterFailed& onRegisterFailed,
   const OnRegisterSuccess& onRegisterSuccess, const ForwardingFlags& flags,
   WireFormat& wireFormat, KeyChain& commandKeyChain,
   const Name& commandCertificateName, Face* face)
{
  nfdRegisterPrefix
    (registeredPrefixId, prefixCopy, onInterest, onRegisterFailed,
     onRegisterSuccess, flags, commandKeyChain, commandCertificateName,
     wireFormat, face);
}

void
Node::send(const uint8_t *encoding, size_t encodingLength)
{
  if (encodingLength > getMaxNdnPacketSize())
    throw runtime_error
      ("The encoded packet size exceeds the maximum limit getMaxNdnPacketSize()");

  transport_->send(encoding, encodingLength);
}

uint64_t
Node::getNextEntryId()
{
  // This is an atomic_uint64_t, so increment is thread-safe.
  return ++lastEntryId_;
}

void
Node::RegisterResponse::operator()(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& responseData)
{
  // Decode responseData->getContent() and check for a success code.
  ControlResponse controlResponse;
  try {
    controlResponse.wireDecode(responseData->getContent(), *TlvWireFormat::get());
  }
  catch (runtime_error& ex) {
    _LOG_DEBUG
      ("Register prefix failed: Error decoding the NFD response: " <<  ex.what());
    try {
      info_->onRegisterFailed_(info_->prefix_);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed.");
    }
    return;
  }

  // Status code 200 is "OK".
  if (controlResponse.getStatusCode() != 200) {
    _LOG_DEBUG
      ("Register prefix failed: Expected NFD status code 200, got: " <<
       controlResponse.getStatusCode());
    try {
      info_->onRegisterFailed_(info_->prefix_);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed.");
    }
    return;
  }

  // Success, so we can add to the registered prefix table.
  if (info_->registeredPrefixId_ != 0) {
    uint64_t interestFilterId = 0;
    if (info_->onInterest_) {
      // registerPrefix was called with the "combined" form that includes the
      // callback, so add an InterestFilterEntry.
      interestFilterId = parent_.getNextEntryId();
      parent_.setInterestFilter
        (interestFilterId,
         ptr_lib::make_shared<const InterestFilter>(*info_->prefix_),
         info_->onInterest_, info_->face_);
    }

    if (!parent_.registeredPrefixTable_.add
        (info_->registeredPrefixId_, info_->prefix_, interestFilterId)) {
      // removeRegisteredPrefix was already called with the registeredPrefixId.
      if (interestFilterId > 0)
        // Remove the related interest filter we just added.
        parent_.unsetInterestFilter(interestFilterId);

      return;
    }
  }

  _LOG_DEBUG("Register prefix succeeded with the NFD forwarder for prefix " <<
             info_->prefix_->toUri());
  if (info_->onRegisterSuccess_) {
    try {
      info_->onRegisterSuccess_(info_->prefix_, info_->registeredPrefixId_);
    } catch (const std::exception& ex) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterSuccess: " << ex.what());
    } catch (...) {
      _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterSuccess.");
    }
  }
}

void
Node::RegisterResponse::operator()(const ptr_lib::shared_ptr<const Interest>& timedOutInterest)
{
  _LOG_DEBUG("Timeout for NFD register prefix command.");
  try {
    info_->onRegisterFailed_(info_->prefix_);
  } catch (const std::exception& ex) {
    _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed: " << ex.what());
  } catch (...) {
    _LOG_ERROR("Node::RegisterResponse::operator(): Error in onRegisterFailed.");
  }
}

void
Node::nfdRegisterPrefix
  (uint64_t registeredPrefixId, const ptr_lib::shared_ptr<const Name>& prefix,
   const OnInterestCallback& onInterest, const OnRegisterFailed& onRegisterFailed,
   const OnRegisterSuccess& onRegisterSuccess, const ForwardingFlags& flags,
   KeyChain& commandKeyChain, const Name& commandCertificateName,
   WireFormat& wireFormat, Face* face)
{
  KeyChain* commandKeyChainPointer = &commandKeyChain;
  if (!commandKeyChainPointer)
    throw runtime_error
      ("registerPrefix: The command KeyChain has not been set. You must call setCommandSigningInfo.");
  if (commandCertificateName.size() == 0)
    throw runtime_error
      ("registerPrefix: The command certificate name has not been set. You must call setCommandSigningInfo.");

  ControlParameters controlParameters;
  controlParameters.setName(*prefix);
  controlParameters.setForwardingFlags(flags);

  ptr_lib::shared_ptr<Interest> commandInterest(new Interest());
  if (isLocal()) {
    commandInterest->setName(Name("/localhost/nfd/rib/register"));
    // The interest is answered by the local host, so set a short timeout.
    commandInterest->setInterestLifetimeMilliseconds(2000.0);
  }
  else {
    commandInterest->setName(Name("/localhop/nfd/rib/register"));
    // The host is remote, so set a longer timeout.
    commandInterest->setInterestLifetimeMilliseconds(4000.0);
  }
  // NFD only accepts TlvWireFormat packets.
  commandInterest->getName().append
    (controlParameters.wireEncode(*TlvWireFormat::get()));
  makeCommandInterest
    (*commandInterest, commandKeyChain, commandCertificateName,
     *TlvWireFormat::get());

  // Send the registration interest.
  RegisterResponse response
    (ptr_lib::shared_ptr<RegisterResponse::Info>(new RegisterResponse::Info
     (prefix, onRegisterFailed, onRegisterSuccess, registeredPrefixId,
      onInterest, face)),
     *this);
  // It is OK for func_lib::function make a copy of the function object because
  //   the Info is in a ptr_lib::shared_ptr.
  expressInterest
    (getNextEntryId(), commandInterest, response, response, OnNetworkNack(),
     wireFormat, face);
}

void
Node::processEvents()
{
  transport_->processEvents();

  // If Face::callLater is overridden to use a different mechanism, then
  // processEvents is not needed to check for delayed calls.
  delayedCallTable_.callTimedOut();
}

void
Node::onReceivedElement(const uint8_t *element, size_t elementLength)
{
  ptr_lib::shared_ptr<LpPacket> lpPacket;
  if (element[0] == ndn_Tlv_LpPacket_LpPacket) {
    // Decode the LpPacket and replace element with the fragment.
    // Use LpPacketLite to avoid copying the fragment.
    struct ndn_LpPacketHeaderField headerFields[5];
    LpPacketLite lpPacketLite
      (headerFields, sizeof(headerFields) / sizeof(headerFields[0]));

    ndn_Error error;
    if ((error = Tlv0_2WireFormatLite::decodeLpPacket
         (lpPacketLite, element, elementLength)))
      throw runtime_error(ndn_getErrorString(error));
    element = lpPacketLite.getFragmentWireEncoding().buf();
    elementLength = lpPacketLite.getFragmentWireEncoding().size();

    // We have saved the wire encoding, so clear to copy it to lpPacket.
    lpPacketLite.setFragmentWireEncoding(BlobLite());

    lpPacket.reset(new LpPacket());
    lpPacket->set(lpPacketLite);
  }

  // First, decode as Interest or Data.
  ptr_lib::shared_ptr<Interest> interest;
  ptr_lib::shared_ptr<Data> data;

  if (element[0] == ndn_Tlv_Interest || element[0] == ndn_Tlv_Data) {
    TlvDecoder decoder(element, elementLength);
    if (decoder.peekType(ndn_Tlv_Interest, elementLength)) {
      interest.reset(new Interest());
      interest->wireDecode(element, elementLength, *TlvWireFormat::get());

      if (lpPacket)
        interest->setLpPacket(lpPacket);
    }
    else if (decoder.peekType(ndn_Tlv_Data, elementLength)) {
      data.reset(new Data());
      data->wireDecode(element, elementLength, *TlvWireFormat::get());

      if (lpPacket)
        data->setLpPacket(lpPacket);
    }
  }

  if (lpPacket) {
    ptr_lib::shared_ptr<NetworkNack> networkNack =
      NetworkNack::getFirstHeader(*lpPacket);
    if (networkNack) {
      if (!interest)
        // We got a Nack but not for an Interest, so drop the packet.
        return;

      vector<ptr_lib::shared_ptr<PendingInterestTable::Entry> > pitEntries;
      pendingInterestTable_.extractEntriesForNackInterest(*interest, pitEntries);
      for (size_t i = 0; i < pitEntries.size(); ++i) {
        try {
          pitEntries[i]->getOnNetworkNack()
            (pitEntries[i]->getInterest(), networkNack);
        } catch (const std::exception& ex) {
          _LOG_ERROR("Node::onReceivedElement: Error in onNetworkNack: " << ex.what());
        } catch (...) {
          _LOG_ERROR("Node::onReceivedElement: Error in onNetworkNack.");
        }
      }

      // We have processed the network Nack packet.
      return;
    }
  }

  // Now process as Interest or Data.
  if (interest) {
    // Call all interest filter callbacks which match.
    vector<ptr_lib::shared_ptr<InterestFilterTable::Entry> > matchedFilters;
    interestFilterTable_.getMatchedFilters(*interest, matchedFilters);

    for (size_t i = 0; i < matchedFilters.size(); ++i) {
      InterestFilterTable::Entry &entry = *matchedFilters[i];
      try {
        entry.getOnInterest()
          (entry.getPrefix(), interest, entry.getFace(),
           entry.getInterestFilterId(), entry.getFilter());
      } catch (const std::exception& ex) {
        _LOG_ERROR("Node::onReceivedElement: Error in onInterest: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Node::onReceivedElement: Error in onInterest.");
      }
    }
  }
  else if (data) {
    vector<ptr_lib::shared_ptr<PendingInterestTable::Entry> > pitEntries;
    pendingInterestTable_.extractEntriesForExpressedInterest(*data, pitEntries);
    for (size_t i = 0; i < pitEntries.size(); ++i) {
      try {
        pitEntries[i]->getOnData()(pitEntries[i]->getInterest(), data);
      } catch (const std::exception& ex) {
        _LOG_ERROR("Node::onReceivedElement: Error in onData: " << ex.what());
      } catch (...) {
        _LOG_ERROR("Node::onReceivedElement: Error in onData.");
      }
    }
  }
}

void
Node::shutdown()
{
  transport_->close();
}

void
Node::expressInterestHelper
  (uint64_t pendingInterestId,
   const ptr_lib::shared_ptr<const Interest>& interestCopy,
   const OnData& onData, const OnTimeout& onTimeout, 
   const OnNetworkNack& onNetworkNack, WireFormat* wireFormat, Face* face)
{
  ptr_lib::shared_ptr<PendingInterestTable::Entry> pendingInterest =
    pendingInterestTable_.add
      (pendingInterestId, interestCopy, onData, onTimeout, onNetworkNack);
  if (!pendingInterest)
    // removePendingInterest was already called with the pendingInterestId.
    return;

  if (onTimeout || interestCopy->getInterestLifetimeMilliseconds() >= 0.0) {
    // Set up the timeout.
    double delayMilliseconds = interestCopy->getInterestLifetimeMilliseconds();
    if (delayMilliseconds < 0.0)
      // Use a default timeout delay.
      delayMilliseconds = 4000.0;

    face->callLater
      (delayMilliseconds,
       bind(&Node::processInterestTimeout, this, pendingInterest));
  }

  // Special case: For timeoutPrefix_ we don't actually send the interest.
  if (!timeoutPrefix_.match(interestCopy->getName())) {
    Blob encoding = interestCopy->wireEncode(*wireFormat);
    if (encoding.size() > getMaxNdnPacketSize())
      throw runtime_error
        ("The encoded interest size exceeds the maximum limit getMaxNdnPacketSize()");
    transport_->send(*encoding);
  }
}

void
Node::processInterestTimeout
  (ptr_lib::shared_ptr<PendingInterestTable::Entry> pendingInterest)
{
  if (pendingInterestTable_.removeEntry(pendingInterest))
    pendingInterest->callTimeout();
}

}
