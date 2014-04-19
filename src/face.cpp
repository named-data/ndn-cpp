/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#include "node.hpp"
#include <ndn-cpp/face.hpp>

using namespace std;

namespace ndn {

Face::Face(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo)
: node_(new Node(transport, connectionInfo)), commandKeyChain_(0)
{
}

Face::Face(const char *host, unsigned short port)
: node_(new Node(ptr_lib::shared_ptr<TcpTransport>(new TcpTransport()), 
                 ptr_lib::make_shared<TcpTransport::ConnectionInfo>(host, port))), 
  commandKeyChain_(0)
{
}

Face::~Face()
{
  delete node_;
}

uint64_t 
Face::expressInterest
  (const Interest& interest, const OnData& onData, const OnTimeout& onTimeout, WireFormat& wireFormat)
{
  return node_->expressInterest(interest, onData, onTimeout, wireFormat);
}
  
uint64_t 
Face::expressInterest
  (const Name& name, const Interest *interestTemplate, const OnData& onData, const OnTimeout& onTimeout,
   WireFormat& wireFormat)
{
  if (interestTemplate)
    return node_->expressInterest(Interest
      (name, interestTemplate->getMinSuffixComponents(), interestTemplate->getMaxSuffixComponents(),
       interestTemplate->getKeyLocator(), interestTemplate->getExclude(),
       interestTemplate->getChildSelector(), interestTemplate->getAnswerOriginKind(),
       interestTemplate->getScope(), interestTemplate->getInterestLifetimeMilliseconds()), onData, onTimeout, wireFormat);
  else
    return node_->expressInterest(Interest(name, 4000.0), onData, onTimeout, wireFormat);  
}

void
Face::removePendingInterest(uint64_t pendingInterestId)
{
  node_->removePendingInterest(pendingInterestId);
}

uint64_t 
Face::registerPrefix
  (const Name& prefix, const OnInterest& onInterest, const OnRegisterFailed& onRegisterFailed, 
   const ForwardingFlags& flags, WireFormat& wireFormat)
{
  return node_->registerPrefix
    (prefix, onInterest, onRegisterFailed, flags, wireFormat, *commandKeyChain_,
     commandCertificateName_);
}

void
Face::removeRegisteredPrefix(uint64_t registeredPrefixId)
{
  node_->removeRegisteredPrefix(registeredPrefixId);
}

void 
Face::processEvents()
{
  // Just call Node's processEvents.
  node_->processEvents();
}

void 
Face::shutdown()
{
  node_->shutdown();
}

}
