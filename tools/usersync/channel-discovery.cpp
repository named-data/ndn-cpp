/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

#include <sstream>
#include "../../src/util/logging.hpp"
#include "../../src/encoding/base64.hpp"
#include <ndn-cpp-tools/usersync/channel-discovery.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

INIT_LOGGER("ndntools.ChannelDiscovery");

namespace ndntools {

ChannelDiscovery::Impl::Impl
  (const Name& applicationDataPrefix, const string& channelListFilePath, 
   Face& face, KeyChain& keyChain, const Name& certificateName,
   Milliseconds syncLifetime, const OnError& onError)
  : applicationDataPrefix_(applicationDataPrefix),
    channelListFilePath_(channelListFilePath), face_(face),
    keyChain_(keyChain), certificateName_(certificateName),
    syncLifetime_(syncLifetime), onError_(onError)
{
}

void
ChannelDiscovery::Impl::initialize(const Name& applicationBroadcastPrefix)
{
  int previousSequenceNumber = -1;
  userChannels_[applicationDataPrefix_.toUri()] = vector<string>();
  // Try to recover the previous sequence number and list of channels from the
  // Data packet saved in channelListFilePath_.
  // TODO: Load and parse the packet, set previousSequenceNumber.

  // Create the initial Data now so that we get any exceptions right away.
  channelListData_ = makeChannelListData(0);
  // TODO: Save the Data wire encoding;

  enabled_ = true;
  // After registering, proceed to onRegisterApplicationPrefixSuccess.
  face_.registerPrefix
    (applicationDataPrefix_,
     bind(&ChannelDiscovery::Impl::onApplicationInterest, shared_from_this(),
          _1, _2, _3, _4, _5),
     bind(&ChannelDiscovery::Impl::onRegisterFailed, shared_from_this(), _1),
     bind(&ChannelDiscovery::Impl::onRegisterApplicationPrefixSuccess,
          shared_from_this(), _1, _2, applicationBroadcastPrefix,
          previousSequenceNumber));
}

void
ChannelDiscovery::Impl::onRegisterApplicationPrefixSuccess
  (const ptr_lib::shared_ptr<const Name>& prefix,
   uint64_t registeredPrefixId, const Name& applicationBroadcastPrefix,
   int previousSequenceNumber)
{
  // Always use the same session number since we remember the last Data packet
  // and sequence number in channelListFilePath_, so that in a new session we
  // continue increasing the sequence number without needing a new session number.
  sessionNo_ = 0;
  sync_.reset(new ChronoSync2013
    (bind(&ChannelDiscovery::Impl::onReceivedSyncState, shared_from_this(), _1, _2),
     bind(&ChannelDiscovery::Impl::onInitialized, shared_from_this()),
     applicationDataPrefix_, applicationBroadcastPrefix, sessionNo_, face_,
     keyChain_, certificateName_, syncLifetime_,
     bind(&ChannelDiscovery::Impl::onRegisterFailed, shared_from_this(), _1),
     previousSequenceNumber));
}

void
ChannelDiscovery::Impl::onInitialized()
{
}

void
ChannelDiscovery::Impl::onReceivedSyncState
  (const std::vector<ChronoSync2013::SyncState>& syncStates,
   bool isRecovery)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  // Request the channel lists from other users.
  for (size_t i = 0; i < syncStates.size(); ++i) {
    if (syncStates[i].getDataPrefix() == applicationDataPrefix_.toUri())
      // Ignore updates from this user.
      continue;
    
    Interest interest(Name(syncStates[i].getDataPrefix()));
    ostringstream sessionNoString;
    sessionNoString << syncStates[i].getSessionNo();
    ostringstream sequenceNoString;
    sequenceNoString << syncStates[i].getSequenceNo();
    interest.getName().append(sessionNoString.str());
    interest.getName().append(sequenceNoString.str());
    interest.setInterestLifetimeMilliseconds(syncLifetime_);

    face_.expressInterest
      (interest, 
       bind(&ChannelDiscovery::Impl::onData, shared_from_this(), _1, _2),
       bind(&ChannelDiscovery::Impl::onTimeout, shared_from_this(), _1));
    
  }
}

void
ChannelDiscovery::Impl::onData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  // Split the new-line separated list and URI decode each channel name.
  vector<string> channels;
  stringstream is(data->getContent().toRawStr());
  string str;
  while (getline(is, str))
    channels.push_back(Name::fromEscapedString(str).toRawStr());

  userChannels_[interest->getName().getPrefix(-2).toUri()] = channels;
}

void
ChannelDiscovery::Impl::onApplicationInterest
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face, 
   uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  if (interest->getName().size() != applicationDataPrefix_.size() + 2)
    // This shouldn't really happen.
    return;

  int sessionNo = ::atoi(interest->getName().get(-2).toEscapedString().c_str());
  int sequenceNo = ::atoi(interest->getName().get(-1).toEscapedString().c_str());
  if (sessionNo != sessionNo_)
    // This shouldn't really happen.
    return;
  if (sequenceNo != sync_->getSequenceNo())
    // Don't respond to requests for old channel lists.
    return;

  if (!channelListData_ || 
      !channelListData_->getName().equals(interest->getName())) {
    channelListData_ = makeChannelListData(sequenceNo);
    // TODO: Save the data wire encoding.
  }

  face.putData(*channelListData_);
}

void
ChannelDiscovery::Impl::onRegisterFailed
  (const ptr_lib::shared_ptr<const Name>& prefix)
{
  enabled_ = false;
  try {
    onError_
      (ErrorCode::REGISTER_FAILED,
       string("Register failed for prefix ") + prefix->toUri());
  } catch (const std::exception& ex) {
    _LOG_ERROR("ChannelDiscovery::onRegisterFailed: Error in onError: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ChannelDiscovery::onRegisterFailed: Error in onError.");
  }
}

void
ChannelDiscovery::Impl::onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest)
{
  try {
    onError_
      (ErrorCode::INTEREST_TIMEOUT,
       string("Timeout for interest ") + interest->getName().toUri());
  } catch (const std::exception& ex) {
    _LOG_ERROR("ChannelDiscovery::onTimeout: Error in onError: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ChannelDiscovery::onTimeout: Error in onError.");
  }
}

ptr_lib::shared_ptr<Data>
ChannelDiscovery::Impl::makeChannelListData(int sequenceNo)
{
  ptr_lib::shared_ptr<Data> data(new Data(applicationDataPrefix_));
  ostringstream sessionNoString;
  sessionNoString << sessionNo_;
  ostringstream sequenceNoString;
  sequenceNoString << sequenceNo;
  data->getName().append(sessionNoString.str());
  data->getName().append(sequenceNoString.str());

  // Make a newline separate list of URI-escaped channel names.
  vector<string> channelList = userChannels_[applicationDataPrefix_.toUri()];
  ostringstream content;
  for (size_t i = 0; i < channelList.size(); ++i) {
    if (i > 0)
      content << "\n";
    Blob channelBlob((const uint8_t*)&channelList[i][0], channelList[i].size());
    content << Name::toEscapedString(*channelBlob);
  }
  string contentString = content.str();
  data->setContent(Blob((const uint8_t*)&contentString[0], contentString.size()));

  keyChain_.sign(*data, certificateName_);

  return data;
}

}
