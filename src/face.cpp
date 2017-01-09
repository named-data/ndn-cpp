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

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include "node.hpp"
#include <ndn-cpp/transport/tcp-transport.hpp>
#include <ndn-cpp/transport/unix-transport.hpp>
#include <ndn-cpp/face.hpp>
#if NDN_CPP_HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

// Only compile the default Face constructor if we have Unix support.
#if NDN_CPP_HAVE_UNISTD_H
string
Face::getUnixSocketFilePathForLocalhost()
{
  string filePath = "/var/run/nfd.sock";
  if (::access(filePath.c_str(), R_OK) == 0)
    return filePath;
  else {
    filePath = "/tmp/.ndnd.sock";
    if (::access(filePath.c_str(), R_OK) == 0)
      return filePath;
    else
      return "";
  }
}

ptr_lib::shared_ptr<Transport>
Face::getDefaultTransport()
{
  if (getUnixSocketFilePathForLocalhost() == "")
    return ptr_lib::make_shared<TcpTransport>();
  else
    return ptr_lib::make_shared<UnixTransport>();
}

ptr_lib::shared_ptr<Transport::ConnectionInfo>
Face::getDefaultConnectionInfo()
{
  string filePath = getUnixSocketFilePathForLocalhost();
  if (filePath == "")
    return ptr_lib::make_shared<TcpTransport::ConnectionInfo>("localhost");
  else
    return ptr_lib::shared_ptr<UnixTransport::ConnectionInfo>
      (new UnixTransport::ConnectionInfo(filePath.c_str()));
}

Face::Face()
: node_(new Node(getDefaultTransport(), getDefaultConnectionInfo())),
  commandKeyChain_(0)
{
}

Face::Face(const char *host, unsigned short port)
: node_(new Node(ptr_lib::shared_ptr<TcpTransport>(new TcpTransport()),
                 ptr_lib::make_shared<TcpTransport::ConnectionInfo>(host, port))),
  commandKeyChain_(0)
{
}
#endif

Face::Face(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo)
: node_(new Node(transport, connectionInfo)), commandKeyChain_(0)
{
}

Face::~Face()
{
  delete node_;
}

uint64_t
Face::expressInterest
  (const Interest& interest, const OnData& onData, const OnTimeout& onTimeout,
   const OnNetworkNack& onNetworkNack, WireFormat& wireFormat)
{
  uint64_t pendingInterestId = node_->getNextEntryId();

  // This copies the interest as required by Node.expressInterest.
  node_->expressInterest
    (pendingInterestId, ptr_lib::make_shared<const Interest>(interest), onData,
     onTimeout, onNetworkNack, wireFormat, this);

  return pendingInterestId;
}

uint64_t
Face::expressInterest
  (const Name& name, const Interest *interestTemplate, const OnData& onData, 
   const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
   WireFormat& wireFormat)
{
  uint64_t pendingInterestId = node_->getNextEntryId();

  // This copies the name object as required by Node.expressInterest.
  node_->expressInterest
    (pendingInterestId, getInterestCopy(name, interestTemplate), onData,
     onTimeout, onNetworkNack, wireFormat, this);

  return pendingInterestId;
}

void
Face::removePendingInterest(uint64_t pendingInterestId)
{
  node_->removePendingInterest(pendingInterestId);
}

void
Face::makeCommandInterest(Interest& interest, WireFormat& wireFormat)
{
  node_->makeCommandInterest
    (interest, *commandKeyChain_, commandCertificateName_, wireFormat);
}

uint64_t
Face::registerPrefix
  (const Name& prefix, const OnInterestCallback& onInterest,
   const OnRegisterFailed& onRegisterFailed,
   const OnRegisterSuccess& onRegisterSuccess, const ForwardingFlags& flags,
   WireFormat& wireFormat)
{
  uint64_t registeredPrefixId = node_->getNextEntryId();

  // This copies the prefix object as required by Node.registerPrefix.
  node_->registerPrefix
    (registeredPrefixId, ptr_lib::make_shared<const Name>(prefix), onInterest,
     onRegisterFailed, onRegisterSuccess, flags, wireFormat, *commandKeyChain_,
     commandCertificateName_, this);

  return registeredPrefixId;
}

void
Face::removeRegisteredPrefix(uint64_t registeredPrefixId)
{
  node_->removeRegisteredPrefix(registeredPrefixId);
}

uint64_t
Face::setInterestFilter
  (const InterestFilter& filter, const OnInterestCallback& onInterest)
{
  uint64_t interestFilterId = node_->getNextEntryId();

  // This copies the filter as required by Node.setInterestFilter.
  node_->setInterestFilter
    (interestFilterId,
     ptr_lib::make_shared<const InterestFilter>(filter), onInterest, this);

  return interestFilterId;
}

uint64_t
Face::setInterestFilter(const Name& prefix, const OnInterestCallback& onInterest)
{
  uint64_t interestFilterId = node_->getNextEntryId();

  // This copies the prefix object as required by Node.setInterestFilter.
  // We could just call setInterestFilter(InterestFilter(prefix), onInterest),
  // but that would make yet another copy of prefix, which we want to avoid.
  node_->setInterestFilter
    (interestFilterId,
     ptr_lib::make_shared<const InterestFilter>(prefix), onInterest, this);

  return interestFilterId;
}

void
Face::unsetInterestFilter(uint64_t interestFilterId)
{
  node_->unsetInterestFilter(interestFilterId);
}

void
Face::putData(const Data& data, WireFormat& wireFormat)
{
  // We get the encoding now before calling send because it may dispatch to
  // async I/O to be called later, and the caller may modify data before then.
  Blob encoding = data.wireEncode(wireFormat);
  if (encoding.size() > getMaxNdnPacketSize())
    throw runtime_error
      ("The encoded Data packet size exceeds the maximum limit getMaxNdnPacketSize()");

  send(encoding);
}

void
Face::send(const uint8_t *encoding, size_t encodingLength)
{
  node_->send(encoding, encodingLength);
}

void
Face::processEvents()
{
  // Just call Node's processEvents.
  node_->processEvents();
}

bool
Face::isLocal()
{
  return node_->isLocal();
}

void
Face::shutdown()
{
  node_->shutdown();
}

void
Face::callLater(Milliseconds delayMilliseconds, const Callback& callback)
{
  node_->callLater(delayMilliseconds, callback);
}

}
