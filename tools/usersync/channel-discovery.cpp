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

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <ndn-cpp/util/logging.hpp>
#include "../../src/encoding/base64.hpp"
#include <ndn-cpp-tools/usersync/channel-discovery.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

INIT_LOGGER("ndntools.ChannelDiscovery");

namespace ndntools {

ChannelDiscovery::Impl::Impl
  (ChannelDiscovery& parent, const Name& applicationDataPrefix,
   const string& channelListFilePath, Face& face, KeyChain& keyChain,
   const Name& certificateName,
   Milliseconds syncLifetime, const OnReceivedChannelList& onReceivedChannelList,
   const OnError& onError)
  : parent_(parent), applicationDataPrefix_(applicationDataPrefix),
    channelListFilePath_(channelListFilePath), face_(face),
    keyChain_(keyChain), certificateName_(certificateName),
    syncLifetime_(syncLifetime), onReceivedChannelList_(onReceivedChannelList),
    onError_(onError), enabled_(false), isInitialized_(false),
    needToPublish_(false)
{
}

void
ChannelDiscovery::Impl::initialize(const Name& applicationBroadcastPrefix)
{
  int previousSequenceNumber = -1;
  // Try to recover the previous sequence number and list of channels from the
  // Data packet saved in channelListFilePath_.
  Blob encodedData = readBase64Blob(channelListFilePath_);
  if (encodedData.isNull()) {
    // There is no saved Data packet, so initialize.
    channelListData_ = makeChannelListData(0);
    writeBase64Blob(channelListData_->wireEncode(), channelListFilePath_);
  }
  else {
    channelListData_.reset(new Data());
    channelListData_->wireDecode(encodedData);

    string userPrefix = channelListData_->getName().getPrefix(-2).toUri();
    if (userPrefix != applicationDataPrefix_.toUri())
      throw runtime_error
        ("The Data name prefix " + userPrefix + " in the channelListFilePath \"" +
         channelListFilePath_ + "\" does not match the given applicationDataPrefix " +
         applicationDataPrefix_.toUri());

    // Restore the sequence number and channel list.
    previousSequenceNumber = ::atoi
      (channelListData_->getName().get(-1).toEscapedString().c_str());
    parseChannelList(channelListData_->getContent(), myChannelList_);
  }

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
ChannelDiscovery::Impl::addChannel(const string& channel)
{
  // Check if the list already has the channel.
  if (find(myChannelList_.begin(), myChannelList_.end(), channel) !=
      myChannelList_.end())
    // Already have the channel.
    return;

  // Add the channel, sort and publish.
  myChannelList_.push_back(channel);
  sort(myChannelList_.begin(), myChannelList_.end());
  publishChannelListData();
}

void
ChannelDiscovery::Impl::removeChannel(const string& channel)
{
  const vector<string>::iterator it = find
    (myChannelList_.begin(), myChannelList_.end(), channel);
  if (it == myChannelList_.end())
    // Already removed.
    return;

  myChannelList_.erase(it);
  publishChannelListData();
}

ptr_lib::shared_ptr<vector<Name>>
ChannelDiscovery::Impl::getOtherUserPrefixes()
{
  ptr_lib::shared_ptr<vector<Name>> result(new vector<Name>());
  for (map<string, vector<string>>::iterator entry = otherUserChannelList_.begin();
       entry != otherUserChannelList_.end(); ++entry)
    result->push_back(Name(entry->first));

  return result;
}

ptr_lib::shared_ptr<vector<string>>
ChannelDiscovery::Impl::getChannelList(const Name& userPrefix)
{
  if (userPrefix.equals(applicationDataPrefix_))
    // Use the vector copy constructor.
    return ptr_lib::make_shared<vector<string>>(myChannelList_);
  else {
    map<string, vector<string>>::iterator list = otherUserChannelList_.find
      (userPrefix.toUri());
    if (list == otherUserChannelList_.end())
      // The userPrefix is not found.
      return ptr_lib::make_shared<vector<string>>();
    else
      // Use the vector copy constructor.
      return ptr_lib::make_shared<vector<string>>(list->second);
  }
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
  isInitialized_ = true;

  if (needToPublish_) {
    needToPublish_ = false;
    publishChannelListData();
  }
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

  ptr_lib::shared_ptr<Name> userPrefix
    (new Name(interest->getName().getPrefix(-2)));
  if (userPrefix->equals(applicationDataPrefix_))
    // We got data for this user. We don't expect this because we never ask for it.
    return;

  vector<string> channelList;
  parseChannelList(data->getContent(), channelList);
  otherUserChannelList_[userPrefix->toUri()] = channelList;
  try {
    onReceivedChannelList_(parent_, userPrefix);
  } catch (const std::exception& ex) {
    _LOG_ERROR("ChannelDiscovery::onData: Error in onReceivedChannelList: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ChannelDiscovery::onData: Error in onReceivedChannelList.");
  }
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
    // Debug: This shouldn't be necessary. But when ChronoChat starts, it increments
    // and publishes a new sequence number which the other users fetch.
    channelListData_ = makeChannelListData(sequenceNo);
    writeBase64Blob(channelListData_->wireEncode(), channelListFilePath_);
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
  // TODO: Re-send the interest?
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

void
ChannelDiscovery::Impl::publishChannelListData()
{
  if (!isInitialized_) {
    // Assume this is being called before ChronoSync is initialized.
    // onInitialized will check needToPublish_ and call this.
    needToPublish_ = true;
    return;
  }

  sync_->publishNextSequenceNo();
  // Assume that we can create the Data packet before an Interest arrives for it
  // from another user, and before the application calls add/removeChannel again.
  channelListData_ = makeChannelListData(sync_->getSequenceNo());
  writeBase64Blob(channelListData_->wireEncode(), channelListFilePath_);
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
  ostringstream content;
  for (size_t i = 0; i < myChannelList_.size(); ++i) {
    if (i > 0)
      content << "\n";
    Blob channelBlob((const uint8_t*)&myChannelList_[i][0], myChannelList_[i].size());
    content << Name::toEscapedString(*channelBlob);
  }
  string contentString = content.str();
  data->setContent(Blob((const uint8_t*)&contentString[0], contentString.size()));

  keyChain_.sign(*data, certificateName_);

  return data;
}

void
ChannelDiscovery::Impl::parseChannelList
  (const Blob& blob, vector<string>& channelList)
{
  stringstream is(blob.toRawStr());
  string str;
  while (getline(is, str))
    channelList.push_back(Name::fromEscapedString(str).toRawStr());
}

ndn::Blob
ChannelDiscovery::Impl::readBase64Blob(const std::string& filePath)
{
  ifstream file(filePath.c_str());
  if (!file.good())
    return Blob();

  stringstream base64;
  base64 << file.rdbuf();
  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t> > blob(new vector<uint8_t>());
  fromBase64(base64.str(), *blob);

  return Blob(blob, false);
}

void
ChannelDiscovery::Impl::writeBase64Blob(const Blob& blob, const string& filePath)
{
  ofstream file(filePath.c_str());
  file << toBase64(blob.buf(), blob.size(), true);
}

}
