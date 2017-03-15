/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from ChronoChat-js by Qiuhan Ding and Wentao Shang.
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

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <stdexcept>
#include <ndn-cpp/util/logging.hpp>
#include "sync-state.pb.h"
#include "../c/util/time.h"
#include "digest-tree.hpp"
#include <ndn-cpp/sync/chrono-sync2013.hpp>

INIT_LOGGER("ndn.ChronoSync2013");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

ChronoSync2013::Impl::Impl
  (const OnReceivedSyncState& onReceivedSyncState,
   const OnInitialized& onInitialized, const Name& applicationDataPrefix,
   const Name& applicationBroadcastPrefix, int sessionNo, Face& face,
   KeyChain& keyChain, const Name& certificateName, Milliseconds syncLifetime,
   int previousSequenceNumber)
: onReceivedSyncState_(onReceivedSyncState), onInitialized_(onInitialized),
  applicationDataPrefixUri_(applicationDataPrefix.toUri()),
  applicationBroadcastPrefix_(applicationBroadcastPrefix), sessionNo_(sessionNo),
  face_(face), keyChain_(keyChain), certificateName_(certificateName),
  syncLifetime_(syncLifetime), initialPreviousSequenceNo_(previousSequenceNumber),
  sequenceNo_(previousSequenceNumber), digestTree_(new DigestTree()),
  contentCache_(&face), enabled_(true)
{
}

void
ChronoSync2013::Impl::initialize(const OnRegisterFailed& onRegisterFailed)
{
  Sync::SyncStateMsg emptyContent;
  digestLog_.push_back(ptr_lib::make_shared<DigestLogEntry>
    ("00", emptyContent.ss()));

  // Register the prefix with the contentCache_ and use our own onInterest
  //   as the onDataNotFound fallback.
  contentCache_.registerPrefix
    (applicationBroadcastPrefix_, onRegisterFailed,
     (OnInterestCallback)bind(&ChronoSync2013::Impl::onInterest, shared_from_this(), _1, _2, _3, _4, _5));

  Interest interest(applicationBroadcastPrefix_);
  interest.getName().append("00");
  interest.setInterestLifetimeMilliseconds(1000);
  face_.expressInterest
    (interest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
     bind(&ChronoSync2013::Impl::initialTimeOut, shared_from_this(), _1));
  _LOG_DEBUG("initial sync expressed");
  _LOG_DEBUG(interest.getName().toUri());
}

int
ChronoSync2013::Impl::logFind(const std::string& digest) const
{
  for (size_t i = 0; i < digestLog_.size(); ++i) {
    if (digest == digestLog_[i]->getDigest())
      return i;
  }

  return -1;
};

bool
ChronoSync2013::Impl::update
  (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content)
{
  for (size_t i = 0; i < content.size(); ++i) {
    if (content.Get(i).type() == Sync::SyncState_ActionType_UPDATE) {
      if (digestTree_->update
          (content.Get(i).name(), content.Get(i).seqno().session(),
           content.Get(i).seqno().seq())) {
        // The digest tree was updated.
        if (applicationDataPrefixUri_ == content.Get(i).name())
          sequenceNo_ = content.Get(i).seqno().seq();
      }
    }
  }

  if (logFind(digestTree_->getRoot()) == -1) {
    digestLog_.push_back(ptr_lib::make_shared<DigestLogEntry>
      (digestTree_->getRoot(), content));
    return true;
  }
  else
    return false;
}

void
ChronoSync2013::Impl::getProducerPrefixes
  (vector<PrefixAndSessionNo>& prefixes) const
{
  prefixes.clear();
  prefixes.reserve(digestTree_->size());

  for (size_t i = 0; i < digestTree_->size(); ++i) {
    const DigestTree::Node& node = digestTree_->get(i);
    prefixes.push_back(PrefixAndSessionNo(node.getDataPrefix(), node.getSessionNo()));
  }
}

int
ChronoSync2013::Impl::getProducerSequenceNo(const std::string& dataPrefix, int sessionNo) const
{
  int index = digestTree_->find(dataPrefix, sessionNo);
  if (index < 0)
    return -1;
  else
    return digestTree_->get(index).getSequenceNo();
}

void
ChronoSync2013::Impl::publishNextSequenceNo(const Blob& applicationInfo)
{
  ++sequenceNo_;

  Sync::SyncStateMsg syncMessage;
  Sync::SyncState* content = syncMessage.add_ss();
  content->set_name(applicationDataPrefixUri_);
  content->set_type(Sync::SyncState_ActionType_UPDATE);
  content->mutable_seqno()->set_seq(sequenceNo_);
  content->mutable_seqno()->set_session(sessionNo_);
  if (!applicationInfo.isNull() && applicationInfo.size() > 0)
    content->set_application_info(applicationInfo.buf(), applicationInfo.size());


  broadcastSyncState(digestTree_->getRoot(), syncMessage);

  if (!update(syncMessage.ss()))
    // Since we incremented the sequence number, we expect there to be a
    //   new digest log entry.
    throw runtime_error
      ("ChronoSync: update did not create a new digest log entry");

  // TODO: Should we have an option to not express an interest if this is the
  //   final publish of the session?
  Interest interest(applicationBroadcastPrefix_);
  interest.getName().append(digestTree_->getRoot());
  interest.setInterestLifetimeMilliseconds(syncLifetime_);
  face_.expressInterest
    (interest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
     bind(&ChronoSync2013::Impl::syncTimeout, shared_from_this(), _1));
}

void
ChronoSync2013::Impl::onInterest
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   uint64_t registerPrefixId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  // Search if the digest already exists in the digest log.
   _LOG_DEBUG("Sync Interest received in callback.");
   _LOG_DEBUG(interest->getName().toUri());

  string syncDigest = interest->getName().get
    (applicationBroadcastPrefix_.size()).toEscapedString();
  if (interest->getName().size() == applicationBroadcastPrefix_.size() + 2)
    // Assume this is a recovery interest.
    syncDigest = interest->getName().get
      (applicationBroadcastPrefix_.size() + 1).toEscapedString();
   _LOG_DEBUG("syncDigest: " + syncDigest);
  if (interest->getName().size() == applicationBroadcastPrefix_.size() + 2 ||
      syncDigest == "00")
    // Recovery interest or newcomer interest.
    processRecoveryInterest(*interest, syncDigest, face);
  else {
    contentCache_.storePendingInterest(interest, face);

    if (syncDigest != digestTree_->getRoot()) {
      size_t index = logFind(syncDigest);
      if (index == -1) {
        // To see whether there is any data packet coming back, wait 2 seconds
        // using the Interest timeout mechanism.
        // TODO: Are we sure using a "/local/timeout" interest is the best future call approach?
        Interest timeout("/local/timeout");
        timeout.setInterestLifetimeMilliseconds(2000);
        face_.expressInterest
          (timeout, dummyOnData,
           bind(&ChronoSync2013::Impl::judgeRecovery, shared_from_this(), _1,
                syncDigest, &face));
        _LOG_DEBUG("set timer recover");
      }
      else
        // common interest processing
        processSyncInterest(index, syncDigest, face);
    }
  }
}

void
ChronoSync2013::Impl::onData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  _LOG_DEBUG("Sync ContentObject received in callback");
  _LOG_DEBUG("name: " + data->getName().toUri());
  Sync::SyncStateMsg tempContent;
  tempContent.ParseFromArray(data->getContent().buf(), data->getContent().size());

  const google::protobuf::RepeatedPtrField<Sync::SyncState >&content = tempContent.ss();
  bool isRecovery;
  if (digestTree_->getRoot() == "00") {
    isRecovery = true;
    //processing initial sync data
    initialOndata(content);
  }
  else {
    update(content);
    if (interest->getName().size() == applicationBroadcastPrefix_.size() + 2)
      // Assume this is a recovery interest.
      isRecovery = true;
    else
      isRecovery = false;
  }

  // Send the interests to fetch the application data.
  vector<SyncState> syncStates;
  for (size_t i = 0; i < content.size(); ++i) {
    // Only report UPDATE sync states.
    if (content.Get(i).type() == Sync::SyncState_ActionType_UPDATE) {
      Blob applicationInfo;
      if (content.Get(i).has_application_info() &&
          content.Get(i).application_info().size() > 0)
        applicationInfo = Blob
          ((const uint8_t*)&content.Get(i).application_info()[0],
           content.Get(i).application_info().size());

      syncStates.push_back(SyncState
        (content.Get(i).name(), content.Get(i).seqno().session(),
         content.Get(i).seqno().seq(), applicationInfo));
    }
  }
  try {
    onReceivedSyncState_(syncStates, isRecovery);
  } catch (const std::exception& ex) {
    _LOG_ERROR("ChronoSync2013::Impl::onData: Error in onReceivedSyncState: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ChronoSync2013::Impl::onData: Error in onReceivedSyncState.");
  }

  Name name(applicationBroadcastPrefix_);
  name.append(digestTree_->getRoot());
  Interest syncInterest(name);
  syncInterest.setInterestLifetimeMilliseconds(syncLifetime_);
  face_.expressInterest
    (syncInterest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
     bind(&ChronoSync2013::Impl::syncTimeout, shared_from_this(), _1));
  _LOG_DEBUG("Syncinterest expressed:");
  _LOG_DEBUG(name.toUri());
}

void
ChronoSync2013::Impl::processRecoveryInterest
  (const Interest& interest, const string& syncDigest, Face& face)
{
  _LOG_DEBUG("processRecoveryInst");
  if (logFind(syncDigest) != -1) {
    Sync::SyncStateMsg tempContent;
    for (size_t i = 0; i < digestTree_->size(); ++i) {
      Sync::SyncState* content = tempContent.add_ss();
      content->set_name(digestTree_->get(i).getDataPrefix());
      content->set_type(Sync::SyncState_ActionType_UPDATE);
      content->mutable_seqno()->set_seq(digestTree_->get(i).getSequenceNo());
      content->mutable_seqno()->set_session(digestTree_->get(i).getSessionNo());
    }

    if (tempContent.ss_size() != 0) {
      ptr_lib::shared_ptr<vector<uint8_t> > array(new vector<uint8_t>(tempContent.ByteSize()));
      tempContent.SerializeToArray(&array->front(), array->size());
      Data data(interest.getName());
      data.setContent(Blob(array, false));
      if (interest.getName().get(-1).toEscapedString() == "00")
        // Limit the lifetime of replies to interest for "00" since they can be different.
        data.getMetaInfo().setFreshnessPeriod(1000);

      keyChain_.sign(data, certificateName_);
      try {
        face.putData(data);
        _LOG_DEBUG("send recovery data back");
        _LOG_DEBUG(interest.getName().toUri());
      }
      catch (std::exception& e) {
        _LOG_DEBUG(e.what());
      }
    }
  }
}

bool
ChronoSync2013::Impl::processSyncInterest
  (int index, const string& syncDigest, Face& face)
{
  vector<string> nameList;
  vector<int> sequenceNoList;
  vector<int> sessionNoList;
  for (size_t j = index + 1; j < digestLog_.size(); ++j) {
    const google::protobuf::RepeatedPtrField<Sync::SyncState>& temp =
      digestLog_[j]->getData();
    for (size_t i = 0; i < temp.size(); ++i) {
      if (temp.Get(i).type() != Sync::SyncState_ActionType_UPDATE)
        continue;

      if (digestTree_->find(temp.Get(i).name(), temp.Get(i).seqno().session()) != -1) {
        int n = -1;
        for (size_t k = 0; k < nameList.size(); ++k) {
          if (nameList[k] == temp.Get(i).name()) {
            n = k;
            break;
          }
        }
        if (n == -1) {
          nameList.push_back(temp.Get(i).name());
          sequenceNoList.push_back(temp.Get(i).seqno().seq());
          sessionNoList.push_back(temp.Get(i).seqno().session());
        }
        else {
          sequenceNoList[n] = temp.Get(i).seqno().seq();
          sessionNoList[n] = temp.Get(i).seqno().session();
        }
      }
    }
  }

  Sync::SyncStateMsg tempContent;
  for (size_t i = 0; i < nameList.size(); ++i) {
    Sync::SyncState* content = tempContent.add_ss();
    content->set_name(nameList[i]);
    content->set_type(Sync::SyncState_ActionType_UPDATE);
    content->mutable_seqno()->set_seq(sequenceNoList[i]);
    content->mutable_seqno()->set_session(sessionNoList[i]);
  }

  bool sent = false;
  if (tempContent.ss_size() != 0) {
    Name name(applicationBroadcastPrefix_);
    name.append(syncDigest);
    ptr_lib::shared_ptr<vector<uint8_t> > array(new vector<uint8_t>(tempContent.ByteSize()));
    tempContent.SerializeToArray(&array->front(), array->size());
    Data data(name);
    data.setContent(Blob(array, false));
    keyChain_.sign(data, certificateName_);
    try {
      face.putData(data);
      sent = true;
      _LOG_DEBUG("Sync Data send");
      _LOG_DEBUG(name.toUri());
    } catch (std::exception& e) {
      _LOG_DEBUG(e.what());
    }
  }

  return sent;
}

void
ChronoSync2013::Impl::sendRecovery(const string& syncdigest_t)
{
  _LOG_DEBUG("unknown digest: ");
  Name name(applicationBroadcastPrefix_);
  name.append("recovery").append(syncdigest_t);
  Interest interest(name);
  interest.setInterestLifetimeMilliseconds(syncLifetime_);
  face_.expressInterest
    (interest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
     bind(&ChronoSync2013::Impl::syncTimeout, shared_from_this(), _1));
  _LOG_DEBUG("Recovery Syncinterest expressed:");
  _LOG_DEBUG(name.toUri());
}

void
ChronoSync2013::Impl::judgeRecovery
  (const ptr_lib::shared_ptr<const Interest> &interest,
   const string& syncDigest, Face* face)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  int index2 = logFind(syncDigest);
  if (index2 != -1) {
    if (syncDigest != digestTree_->getRoot())
      processSyncInterest(index2, syncDigest, *face);
  }
  else
    sendRecovery(syncDigest);
}

void
ChronoSync2013::Impl::syncTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

   _LOG_DEBUG("Sync Interest time out.");
   _LOG_DEBUG("Sync Interest name: " + interest->getName().toUri());
  string component = interest->getName().get
    (applicationBroadcastPrefix_.size()).toEscapedString();
  if (component == digestTree_->getRoot()) {
    Name name(interest->getName());
    Interest retryInterest(interest->getName());
    retryInterest.setInterestLifetimeMilliseconds(syncLifetime_);
    face_.expressInterest
      (retryInterest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
       bind(&ChronoSync2013::Impl::syncTimeout, shared_from_this(), _1));
     _LOG_DEBUG("Syncinterest expressed:");
     _LOG_DEBUG(name.toUri());
  }
}

void
ChronoSync2013::Impl::initialOndata
  (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content)
{
  // The user is a new comer and receive data of all other people in the group.
  update(content);
  string digest = digestTree_->getRoot();
  for (size_t i = 0; i < content.size(); ++i) {
    if (content.Get(i).name() == applicationDataPrefixUri_ && content.Get(i).seqno().session() == sessionNo_) {
      // If the user was an old comer, after add the static log he needs to increase his seqno by 1.
      Sync::SyncStateMsg tempContent;
      Sync::SyncState* content2 = tempContent.add_ss();
      content2->set_name(applicationDataPrefixUri_);
      content2->set_type(Sync::SyncState_ActionType_UPDATE);
      content2->mutable_seqno()->set_seq(content.Get(i).seqno().seq() + 1);
      content2->mutable_seqno()->set_session(sessionNo_);
      if (update(tempContent.ss())) {
        try {
          onInitialized_();
        } catch (const std::exception& ex) {
          _LOG_ERROR("ChronoSync2013::Impl::initialOndata: Error in onInitialized: " << ex.what());
        } catch (...) {
          _LOG_ERROR("ChronoSync2013::Impl::initialOndata: Error in onInitialized.");
        }
      }
    }
  }

  Sync::SyncStateMsg tempContent2;
  if (sequenceNo_ >= 0) {
    // Send the data packet with the new seqno back.
    Sync::SyncState* content2 = tempContent2.add_ss();
    content2->set_name(applicationDataPrefixUri_);
    content2->set_type(Sync::SyncState_ActionType_UPDATE);
    content2->mutable_seqno()->set_seq(sequenceNo_);
    content2->mutable_seqno()->set_session(sessionNo_);
  }
  else {
    Sync::SyncState* content2 = tempContent2.add_ss();
    content2->set_name(applicationDataPrefixUri_);
    content2->set_type(Sync::SyncState_ActionType_UPDATE);
    content2->mutable_seqno()->set_seq(0);
    content2->mutable_seqno()->set_session(sessionNo_);
  }

  broadcastSyncState(digest, tempContent2);

  if (digestTree_->find(applicationDataPrefixUri_, sessionNo_) == -1) {
    // the user hasn't put himself in the digest tree.
    _LOG_DEBUG("initial state");
    ++sequenceNo_;
    Sync::SyncStateMsg tempContent;
    Sync::SyncState* content2 = tempContent.add_ss();
    content2->set_name(applicationDataPrefixUri_);
    content2->set_type(Sync::SyncState_ActionType_UPDATE);
    content2->mutable_seqno()->set_seq(sequenceNo_);
    content2->mutable_seqno()->set_session(sessionNo_);

    if (update(tempContent.ss())) {
      try {
        onInitialized_();
      } catch (const std::exception& ex) {
        _LOG_ERROR("ChronoSync2013::Impl::initialOndata: Error in onInitialized: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ChronoSync2013::Impl::initialOndata: Error in onInitialized.");
      }
    }
  }
}

void
ChronoSync2013::Impl::initialTimeOut(const ptr_lib::shared_ptr<const Interest>& interest)
{
  if (!enabled_)
    // Ignore callbacks after the application calls shutdown().
    return;

  _LOG_DEBUG("initial sync timeout");
  _LOG_DEBUG("no other people");
  ++sequenceNo_;
  if (sequenceNo_ != initialPreviousSequenceNo_ + 1) {
    // Since there were no other users, we expect the sequence number to follow
    // the initial value.
    _LOG_ERROR
      ("ChronoSync: sequenceNo_ is not the expected value for first use.");
    return;
  }

  Sync::SyncStateMsg tempContent;
  Sync::SyncState* content = tempContent.add_ss();
  content->set_name(applicationDataPrefixUri_);
  content->set_type(Sync::SyncState_ActionType_UPDATE);
  content->mutable_seqno()->set_seq(sequenceNo_);
  content->mutable_seqno()->set_session(sessionNo_);
  update(tempContent.ss());

  try {
    onInitialized_();
  } catch (const std::exception& ex) {
    _LOG_ERROR("ChronoSync2013::Impl::initialTimeOut: Error in onInitialized: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ChronoSync2013::Impl::initialTimeOut: Error in onInitialized.");
  }

  Name name(applicationBroadcastPrefix_);
  name.append(digestTree_->getRoot());
  Interest retryInterest(name);
  retryInterest.setInterestLifetimeMilliseconds(syncLifetime_);
  face_.expressInterest
    (retryInterest, bind(&ChronoSync2013::Impl::onData, shared_from_this(), _1, _2),
     bind(&ChronoSync2013::Impl::syncTimeout, shared_from_this(), _1));
  _LOG_DEBUG("Syncinterest expressed:");
  _LOG_DEBUG(name.toUri());
}

void
ChronoSync2013::Impl::broadcastSyncState
  (const string& digest, const Sync::SyncStateMsg& syncMessage)
{
  ptr_lib::shared_ptr<vector<uint8_t> > array(new vector<uint8_t>(syncMessage.ByteSize()));
  syncMessage.SerializeToArray(&array->front(), array->size());
  Data data(applicationBroadcastPrefix_);
  data.getName().append(digest);
  data.setContent(Blob(array, false));
  keyChain_.sign(data, certificateName_);
  contentCache_.add(data);
}

ChronoSync2013::DigestLogEntry::DigestLogEntry
  (const std::string& digest,
   const google::protobuf::RepeatedPtrField<Sync::SyncState>& data)
  : digest_(digest),
   data_(new google::protobuf::RepeatedPtrField<Sync::SyncState>(data))
{
}

void
ChronoSync2013::Impl::dummyOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
}

}

#endif // NDN_CPP_HAVE_PROTOBUF
