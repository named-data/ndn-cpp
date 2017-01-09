/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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

#include <cstdlib>
#include <string>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <math.h>
#include <sstream>
#include <stdexcept>
#include <ndn-cpp/security/identity/memory-identity-storage.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>
#include <ndn-cpp/transport/tcp-transport.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/sync/chrono-sync2013.hpp>
// This include is produced by:
// protoc --cpp_out=. chatbuf.proto
#include "chatbuf.pb.h"
#if NDN_CPP_HAVE_TIME_H
#include <time.h>
#endif
#if NDN_CPP_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

// Define the Chat class here so that the ChronoChat demo is self-contained.

class Chat : public ptr_lib::enable_shared_from_this<Chat> {
public:
  /**
   * Create a new Chat, which should belong to a shared_ptr. Then you must
   * call initialize().
   */
  Chat
    (const string& screenName, const string& chatRoom,
     const Name& hubPrefix, Face& face, KeyChain& keyChain,
     const Name& certificateName)
    : screenName_(screenName), chatRoom_(chatRoom), maxMessageCacheLength_(100),
      isRecoverySyncState_(true), syncLifetime_(5000.0), face_(face),
      keyChain_(keyChain), certificateName_(certificateName)
  {
    // This should only be called once, so get the random string here.
    chatPrefix_ = Name(hubPrefix).append(chatRoom_).append(Chat::getRandomString());
  }

  /**
   * Create the ChronoSync2013 and call registerPrefix for chat interests. You
   * must call this after creating this Chat and making it belong to a
   * shared_ptr. This is a separate method from the constructor because we need
   * to call shared_from_this(), but in the constructor this object does not yet
   * belong to a shared_ptr.
   */
  void
  initialize()
  {
    int sessionNo = (int)::round(getNowMilliseconds()  / 1000.0);
    ostringstream tempStream;
    tempStream << screenName_ << sessionNo;
    userName_ = tempStream.str();

    sync_.reset(new ChronoSync2013
      (bind(&Chat::sendInterest, shared_from_this(), _1, _2),
       bind(&Chat::initial, shared_from_this()), chatPrefix_,
       Name("/ndn/broadcast/ChronoChat-0.3").append(chatRoom_), sessionNo,
       face_, keyChain_, certificateName_, syncLifetime_, onRegisterFailed));

    // TODO: After we remove the registerPrefix with the deprecated OnInterest,
    // we can remove the explicit cast to OnInterestCallback (needed for boost).
    face_.registerPrefix
      (chatPrefix_, (const OnInterestCallback&)bind(&Chat::onInterest, shared_from_this(), _1, _2, _3, _4, _5),
       onRegisterFailed);
  }

  // Send a chat message.
  void
  sendMessage(const string& chatmsg);

  // Send leave message and leave.
  void
  leave();

  /**
   * Use gettimeofday to return the current time in milliseconds.
   * @return The current time in milliseconds since 1/1/1970, including fractions
   * of a millisecond according to timeval.tv_usec.
   */
  static MillisecondsSince1970
  getNowMilliseconds();

private:
  // Initialization: push the JOIN message in to the messageCache_, update roster and start heartbeat.
  void
  initial();

  // Send a Chat Interest to fetch chat messages after get the user gets the Sync data packet back but will not send interest.
  void
  sendInterest
    (const vector<ChronoSync2013::SyncState>& syncStates, bool isRecovery);

  // Send back Chat Data Packet which contains the user's message.
  void
  onInterest
    (const ptr_lib::shared_ptr<const Name>& prefix,
     const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
     uint64_t interestFilterId,
     const ptr_lib::shared_ptr<const InterestFilter>& filter);

  // Processing the incoming Chat data.
  void
  onData
    (const ptr_lib::shared_ptr<const Interest>& interest,
     const ptr_lib::shared_ptr<Data>& data);

  void
  chatTimeout(const ptr_lib::shared_ptr<const Interest>& interest);

  /**
   * This repeatedly calls itself after a timeout to send a heartbeat message
   * (chat message type HELLO).
   * This method has an "interest" argument because we use it as the onTimeout
   * for Face.expressInterest.
   */
  void
  heartbeat(const ptr_lib::shared_ptr<const Interest> &interest);

  /**
   * This is called after a timeout to check if the user with prefix has a newer
   * sequence number than the given tempSequenceNo. If not, assume the user is idle and
   * remove from the roster and print a leave message.
   * This method has an "interest" argument because we use it as the onTimeout
   * for Face.expressInterest.
   */
  void
  alive
    (const ptr_lib::shared_ptr<const Interest> &interest, int tempSequenceNo,
     const string& name, int sessionNo, const string& prefix);

  /**
   * Append a new CachedMessage to messageCache_, using given messageType and message,
   * the sequence number from sync_->getSequenceNo() and the current time. Also
   * remove elements from the front of the cache as needed to keep
   * the size to maxMessageCacheLength_.
   */
  void
  messageCacheAppend(int messageType, const string& message);

  /**
   * Search for an entry in the roster_.
   * @param nameAndSession The entry to search for.
   * @return The index on roster_, or -1 if not found.
   */
  int
  rosterFind(const string& nameAndSession);

  // Generate a random name for ChronoSync.
  static string
  getRandomString();

  static void
  onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix);

  /**
   * This is a do-nothing onData for using expressInterest for timeouts.
   * This should never be called.
   */
  static void
  dummyOnData
    (const ptr_lib::shared_ptr<const Interest>& interest,
     const ptr_lib::shared_ptr<Data>& data);

  class CachedMessage {
  public:
    CachedMessage
      (int sequenceNo, int messageType, const string& message, MillisecondsSince1970 time)
    : sequenceNo_(sequenceNo), messageType_(messageType), message_(message), time_(time)
    {}

    int
    getSequenceNo() const { return sequenceNo_; }

    int
    getMessageType() const { return messageType_; }

    const string&
    getMessage() const { return message_; }

    MillisecondsSince1970
    getTime() const { return time_; }

  private:
    int sequenceNo_;
    // This is really enum SyncDemo::ChatMessage_ChatMessageType, but make it
    //   in int so that the head doesn't need to include the protobuf header.
    int messageType_;
    string message_;
    MillisecondsSince1970 time_;
  };

  vector<ptr_lib::shared_ptr<CachedMessage> > messageCache_;
  vector<string> roster_;
  size_t maxMessageCacheLength_;
  bool isRecoverySyncState_;
  string screenName_;
  string chatRoom_;
  string userName_;
  Name chatPrefix_;
  Milliseconds syncLifetime_;
  ptr_lib::shared_ptr<ChronoSync2013> sync_;
  Face& face_;
  KeyChain& keyChain_;
  Name certificateName_;
};

void
Chat::initial()
{
  // Set the heartbeat timeout using the Interest timeout mechanism. The
  // heartbeat() function will call itself again after a timeout.
  // TODO: Are we sure using a "/local/timeout" interest is the best future call approach?
  Interest timeout("/local/timeout");
  timeout.setInterestLifetimeMilliseconds(60000);
  face_.expressInterest(timeout, dummyOnData, bind(&Chat::heartbeat, shared_from_this(), _1));

  if (rosterFind(userName_) < 0) {
    roster_.push_back(userName_);
    cout << "Member: " << screenName_ << endl;
    cout << screenName_ << ": Join" << endl;
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");
  }
}

void
Chat::sendInterest
  (const vector<ChronoSync2013::SyncState>& syncStates, bool isRecovery)
{
  // This is used by onData to decide whether to display the chat messages.
  isRecoverySyncState_ = isRecovery;

  vector<string> sendList;
  vector<int> sessionNoList;
  vector<int> sequenceNoList;
  for (size_t j = 0; j < syncStates.size(); ++j) {
    Name nameComponents(syncStates[j].getDataPrefix());
    string tempName = nameComponents.get(-1).toEscapedString();
    int sessionNo = syncStates[j].getSessionNo();
    if (tempName != screenName_) {
      int index = -1;
      for (size_t k = 0; k < sendList.size(); ++k) {
        if (sendList[k] == syncStates[j].getDataPrefix()) {
          index = k;
          break;
        }
      }
      if (index != -1) {
        sessionNoList[index] = sessionNo;
        sequenceNoList[index] = syncStates[j].getSequenceNo();
      }
      else{
        sendList.push_back(syncStates[j].getDataPrefix());
        sessionNoList.push_back(sessionNo);
        sequenceNoList.push_back(syncStates[j].getSequenceNo());
      }
    }
  }

  for (size_t i = 0; i < sendList.size(); ++i) {
    ostringstream uri;
    uri << sendList[i] << "/" << sessionNoList[i] << "/" << sequenceNoList[i];
    Interest interest(uri.str());
    interest.setInterestLifetimeMilliseconds(syncLifetime_);
    face_.expressInterest
      (interest, bind(&Chat::onData, shared_from_this(), _1, _2),
       bind(&Chat::chatTimeout, shared_from_this(), _1));
  }
}

void
Chat::onInterest
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  SyncDemo::ChatMessage content;
  int sequenceNo = ::atoi(interest->getName().get(chatPrefix_.size() + 1).toEscapedString().c_str());
  for (int i = messageCache_.size() - 1; i >= 0; --i) {
    if (messageCache_[i]->getSequenceNo() == sequenceNo) {
      if (messageCache_[i]->getMessageType() != SyncDemo::ChatMessage_ChatMessageType_CHAT) {
        content.set_from(screenName_);
        content.set_to(chatRoom_);
        content.set_type((SyncDemo::ChatMessage_ChatMessageType)messageCache_[i]->getMessageType());
        content.set_timestamp(::round(messageCache_[i]->getTime() / 1000.0));
      }
      else {
        content.set_from(screenName_);
        content.set_to(chatRoom_);
        content.set_type((SyncDemo::ChatMessage_ChatMessageType)messageCache_[i]->getMessageType());
        content.set_data(messageCache_[i]->getMessage());
        content.set_timestamp(::round(messageCache_[i]->getTime() / 1000.0));
      }
      break;
    }
  }

  if (content.from().size() != 0) {
    ptr_lib::shared_ptr<vector<uint8_t> > array(new vector<uint8_t>(content.ByteSize()));
    content.SerializeToArray(&array->front(), array->size());
    Data data(interest->getName());
    data.setContent(Blob(array, false));
    keyChain_.sign(data, certificateName_);
    try {
      face.putData(data);
    }
    catch (std::exception& e) {
      cout << "Error sending the chat data " << e.what() << endl;
    }
  }
}

void
Chat::onData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
  SyncDemo::ChatMessage content;
  content.ParseFromArray(data->getContent().buf(), data->getContent().size());
  if (getNowMilliseconds() - content.timestamp() * 1000.0 < 120000.0) {
    string name = content.from();
    string prefix = data->getName().getPrefix(-2).toUri();
    int sessionNo = ::atoi(data->getName().get(-2).toEscapedString().c_str());
    int sequenceNo = ::atoi(data->getName().get(-1).toEscapedString().c_str());
    ostringstream tempStream;
    tempStream << name << sessionNo;
    string nameAndSession = tempStream.str();

    size_t l = 0;
    //update roster
    while (l < roster_.size()) {
      string tempName2 = roster_[l].substr(0, roster_[l].size() - 10);
      int tempSessionNo = ::atoi(roster_[l].substr(roster_[l].size() - 10, 10).c_str());
      if (name != tempName2 && content.type() != SyncDemo::ChatMessage_ChatMessageType_LEAVE)
        ++l;
      else {
        if (name == tempName2 && sessionNo > tempSessionNo)
          roster_[l] = nameAndSession;
        break;
      }
    }

    if (l == roster_.size()) {
      roster_.push_back(nameAndSession);
      cout << name << ": Join" << endl;
    }

    // Set the alive timeout using the Interest timeout mechanism.
    // TODO: Are we sure using a "/local/timeout" interest is the best future call approach?
    Interest timeout("/local/timeout");
    timeout.setInterestLifetimeMilliseconds(120000);
    face_.expressInterest
      (timeout, dummyOnData,
       bind(&Chat::alive, shared_from_this(), _1, sequenceNo, name, sessionNo, prefix));

    // isRecoverySyncState_ was set by sendInterest.
    // TODO: If isRecoverySyncState_ changed, this assumes that we won't get
    //   data from an interest sent before it changed.
    if (content.type() == SyncDemo::ChatMessage_ChatMessageType_CHAT &&
        !isRecoverySyncState_ && content.from() != screenName_)
      cout << content.from() << ": " << content.data() << endl;
    else if (content.type() == SyncDemo::ChatMessage_ChatMessageType_LEAVE) {
      // leave message
      int n = rosterFind(nameAndSession);
      if (n >= 0 && name != screenName_) {
        roster_.erase(roster_.begin() + n);
        cout << name << ": Leave" << endl;
      }
    }
  }
}

void
Chat::chatTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
  cout << "Timeout waiting for chat data" << endl;
}

void
Chat::heartbeat(const ptr_lib::shared_ptr<const Interest> &interest)
{
  if (messageCache_.size() == 0)
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");

  sync_->publishNextSequenceNo();
  messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_HELLO, "xxx");

  // Call again.
  // TODO: Are we sure using a "/local/timeout" interest is the best future call approach?
  Interest timeout("/local/timeout");
  timeout.setInterestLifetimeMilliseconds(60000);
  face_.expressInterest(timeout, dummyOnData, bind(&Chat::heartbeat, shared_from_this(), _1));
}

void
Chat::sendMessage(const string& chatMessage)
{
  if (messageCache_.size() == 0)
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_JOIN, "xxx");

  // Ignore an empty message.
  // forming Sync Data Packet.
  if (chatMessage != "") {
    sync_->publishNextSequenceNo();
    messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_CHAT, chatMessage);
    cout << screenName_ << ": " << chatMessage << endl;
  }
}

void
Chat::leave()
{
  sync_->publishNextSequenceNo();
  messageCacheAppend(SyncDemo::ChatMessage_ChatMessageType_LEAVE, "xxx");
}

void
Chat::alive
  (const ptr_lib::shared_ptr<const Interest> &interest, int tempSequenceNo,
   const string& name, int sessionNo, const string& prefix)
{
  int sequenceNo = sync_->getProducerSequenceNo(prefix, sessionNo);
  ostringstream tempStream;
  tempStream << name << sessionNo;
  string nameAndSession = tempStream.str();
  int n = rosterFind(nameAndSession);
  if (sequenceNo != -1 && n >= 0) {
    if (tempSequenceNo == sequenceNo){
      roster_.erase(roster_.begin() + n);
      cout << name << ": Leave" << endl;
    }
  }
}

void
Chat::messageCacheAppend(int messageType, const string& message)
{
  messageCache_.push_back(ptr_lib::make_shared<CachedMessage>
    (sync_->getSequenceNo(), messageType, message, getNowMilliseconds()));
  while (messageCache_.size() > maxMessageCacheLength_)
    messageCache_.erase(messageCache_.begin());
}

int
Chat::rosterFind(const string& nameAndSession)
{
  for (size_t i = 0; i < roster_.size(); ++i) {
    if (roster_[i] == nameAndSession)
      return i;
  }

  return -1;
}

string
Chat::getRandomString()
{
  string seed("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789");
  string result;
  uint8_t random[10];
  CryptoLite::generateRandomBytes(random, sizeof(random));
  for (int i = 0; i < 10; ++i) {
    // Using % means the distribution isn't uniform, but that's OK.
    size_t pos = (size_t)random[i] % seed.size();
    result += seed[pos];
  }

  return result;
}

void
Chat::onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
{
  cout << "Register failed for prefix " << prefix->toUri() << endl;
}

MillisecondsSince1970
Chat::getNowMilliseconds()
{
  struct timeval t;
  // Note: configure.ac requires gettimeofday.
  gettimeofday(&t, 0);
  return t.tv_sec * 1000.0 + t.tv_usec / 1000.0;
}

void
Chat::dummyOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data)
{
}

static uint8_t DEFAULT_RSA_PUBLIC_KEY_DER[] = {
  0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01,
  0x00, 0xb8, 0x09, 0xa7, 0x59, 0x82, 0x84, 0xec, 0x4f, 0x06, 0xfa, 0x1c, 0xb2, 0xe1, 0x38, 0x93,
  0x53, 0xbb, 0x7d, 0xd4, 0xac, 0x88, 0x1a, 0xf8, 0x25, 0x11, 0xe4, 0xfa, 0x1d, 0x61, 0x24, 0x5b,
  0x82, 0xca, 0xcd, 0x72, 0xce, 0xdb, 0x66, 0xb5, 0x8d, 0x54, 0xbd, 0xfb, 0x23, 0xfd, 0xe8, 0x8e,
  0xaf, 0xa7, 0xb3, 0x79, 0xbe, 0x94, 0xb5, 0xb7, 0xba, 0x17, 0xb6, 0x05, 0xae, 0xce, 0x43, 0xbe,
  0x3b, 0xce, 0x6e, 0xea, 0x07, 0xdb, 0xbf, 0x0a, 0x7e, 0xeb, 0xbc, 0xc9, 0x7b, 0x62, 0x3c, 0xf5,
  0xe1, 0xce, 0xe1, 0xd9, 0x8d, 0x9c, 0xfe, 0x1f, 0xc7, 0xf8, 0xfb, 0x59, 0xc0, 0x94, 0x0b, 0x2c,
  0xd9, 0x7d, 0xbc, 0x96, 0xeb, 0xb8, 0x79, 0x22, 0x8a, 0x2e, 0xa0, 0x12, 0x1d, 0x42, 0x07, 0xb6,
  0x5d, 0xdb, 0xe1, 0xf6, 0xb1, 0x5d, 0x7b, 0x1f, 0x54, 0x52, 0x1c, 0xa3, 0x11, 0x9b, 0xf9, 0xeb,
  0xbe, 0xb3, 0x95, 0xca, 0xa5, 0x87, 0x3f, 0x31, 0x18, 0x1a, 0xc9, 0x99, 0x01, 0xec, 0xaa, 0x90,
  0xfd, 0x8a, 0x36, 0x35, 0x5e, 0x12, 0x81, 0xbe, 0x84, 0x88, 0xa1, 0x0d, 0x19, 0x2a, 0x4a, 0x66,
  0xc1, 0x59, 0x3c, 0x41, 0x83, 0x3d, 0x3d, 0xb8, 0xd4, 0xab, 0x34, 0x90, 0x06, 0x3e, 0x1a, 0x61,
  0x74, 0xbe, 0x04, 0xf5, 0x7a, 0x69, 0x1b, 0x9d, 0x56, 0xfc, 0x83, 0xb7, 0x60, 0xc1, 0x5e, 0x9d,
  0x85, 0x34, 0xfd, 0x02, 0x1a, 0xba, 0x2c, 0x09, 0x72, 0xa7, 0x4a, 0x5e, 0x18, 0xbf, 0xc0, 0x58,
  0xa7, 0x49, 0x34, 0x46, 0x61, 0x59, 0x0e, 0xe2, 0x6e, 0x9e, 0xd2, 0xdb, 0xfd, 0x72, 0x2f, 0x3c,
  0x47, 0xcc, 0x5f, 0x99, 0x62, 0xee, 0x0d, 0xf3, 0x1f, 0x30, 0x25, 0x20, 0x92, 0x15, 0x4b, 0x04,
  0xfe, 0x15, 0x19, 0x1d, 0xdc, 0x7e, 0x5c, 0x10, 0x21, 0x52, 0x21, 0x91, 0x54, 0x60, 0x8b, 0x92,
  0x41, 0x02, 0x03, 0x01, 0x00, 0x01
};

static uint8_t DEFAULT_RSA_PRIVATE_KEY_DER[] = {
  0x30, 0x82, 0x04, 0xa5, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00, 0xb8, 0x09, 0xa7, 0x59,
  0x82, 0x84, 0xec, 0x4f, 0x06, 0xfa, 0x1c, 0xb2, 0xe1, 0x38, 0x93, 0x53, 0xbb, 0x7d, 0xd4, 0xac,
  0x88, 0x1a, 0xf8, 0x25, 0x11, 0xe4, 0xfa, 0x1d, 0x61, 0x24, 0x5b, 0x82, 0xca, 0xcd, 0x72, 0xce,
  0xdb, 0x66, 0xb5, 0x8d, 0x54, 0xbd, 0xfb, 0x23, 0xfd, 0xe8, 0x8e, 0xaf, 0xa7, 0xb3, 0x79, 0xbe,
  0x94, 0xb5, 0xb7, 0xba, 0x17, 0xb6, 0x05, 0xae, 0xce, 0x43, 0xbe, 0x3b, 0xce, 0x6e, 0xea, 0x07,
  0xdb, 0xbf, 0x0a, 0x7e, 0xeb, 0xbc, 0xc9, 0x7b, 0x62, 0x3c, 0xf5, 0xe1, 0xce, 0xe1, 0xd9, 0x8d,
  0x9c, 0xfe, 0x1f, 0xc7, 0xf8, 0xfb, 0x59, 0xc0, 0x94, 0x0b, 0x2c, 0xd9, 0x7d, 0xbc, 0x96, 0xeb,
  0xb8, 0x79, 0x22, 0x8a, 0x2e, 0xa0, 0x12, 0x1d, 0x42, 0x07, 0xb6, 0x5d, 0xdb, 0xe1, 0xf6, 0xb1,
  0x5d, 0x7b, 0x1f, 0x54, 0x52, 0x1c, 0xa3, 0x11, 0x9b, 0xf9, 0xeb, 0xbe, 0xb3, 0x95, 0xca, 0xa5,
  0x87, 0x3f, 0x31, 0x18, 0x1a, 0xc9, 0x99, 0x01, 0xec, 0xaa, 0x90, 0xfd, 0x8a, 0x36, 0x35, 0x5e,
  0x12, 0x81, 0xbe, 0x84, 0x88, 0xa1, 0x0d, 0x19, 0x2a, 0x4a, 0x66, 0xc1, 0x59, 0x3c, 0x41, 0x83,
  0x3d, 0x3d, 0xb8, 0xd4, 0xab, 0x34, 0x90, 0x06, 0x3e, 0x1a, 0x61, 0x74, 0xbe, 0x04, 0xf5, 0x7a,
  0x69, 0x1b, 0x9d, 0x56, 0xfc, 0x83, 0xb7, 0x60, 0xc1, 0x5e, 0x9d, 0x85, 0x34, 0xfd, 0x02, 0x1a,
  0xba, 0x2c, 0x09, 0x72, 0xa7, 0x4a, 0x5e, 0x18, 0xbf, 0xc0, 0x58, 0xa7, 0x49, 0x34, 0x46, 0x61,
  0x59, 0x0e, 0xe2, 0x6e, 0x9e, 0xd2, 0xdb, 0xfd, 0x72, 0x2f, 0x3c, 0x47, 0xcc, 0x5f, 0x99, 0x62,
  0xee, 0x0d, 0xf3, 0x1f, 0x30, 0x25, 0x20, 0x92, 0x15, 0x4b, 0x04, 0xfe, 0x15, 0x19, 0x1d, 0xdc,
  0x7e, 0x5c, 0x10, 0x21, 0x52, 0x21, 0x91, 0x54, 0x60, 0x8b, 0x92, 0x41, 0x02, 0x03, 0x01, 0x00,
  0x01, 0x02, 0x82, 0x01, 0x01, 0x00, 0x8a, 0x05, 0xfb, 0x73, 0x7f, 0x16, 0xaf, 0x9f, 0xa9, 0x4c,
  0xe5, 0x3f, 0x26, 0xf8, 0x66, 0x4d, 0xd2, 0xfc, 0xd1, 0x06, 0xc0, 0x60, 0xf1, 0x9f, 0xe3, 0xa6,
  0xc6, 0x0a, 0x48, 0xb3, 0x9a, 0xca, 0x21, 0xcd, 0x29, 0x80, 0x88, 0x3d, 0xa4, 0x85, 0xa5, 0x7b,
  0x82, 0x21, 0x81, 0x28, 0xeb, 0xf2, 0x43, 0x24, 0xb0, 0x76, 0xc5, 0x52, 0xef, 0xc2, 0xea, 0x4b,
  0x82, 0x41, 0x92, 0xc2, 0x6d, 0xa6, 0xae, 0xf0, 0xb2, 0x26, 0x48, 0xa1, 0x23, 0x7f, 0x02, 0xcf,
  0xa8, 0x90, 0x17, 0xa2, 0x3e, 0x8a, 0x26, 0xbd, 0x6d, 0x8a, 0xee, 0xa6, 0x0c, 0x31, 0xce, 0xc2,
  0xbb, 0x92, 0x59, 0xb5, 0x73, 0xe2, 0x7d, 0x91, 0x75, 0xe2, 0xbd, 0x8c, 0x63, 0xe2, 0x1c, 0x8b,
  0xc2, 0x6a, 0x1c, 0xfe, 0x69, 0xc0, 0x44, 0xcb, 0x58, 0x57, 0xb7, 0x13, 0x42, 0xf0, 0xdb, 0x50,
  0x4c, 0xe0, 0x45, 0x09, 0x8f, 0xca, 0x45, 0x8a, 0x06, 0xfe, 0x98, 0xd1, 0x22, 0xf5, 0x5a, 0x9a,
  0xdf, 0x89, 0x17, 0xca, 0x20, 0xcc, 0x12, 0xa9, 0x09, 0x3d, 0xd5, 0xf7, 0xe3, 0xeb, 0x08, 0x4a,
  0xc4, 0x12, 0xc0, 0xb9, 0x47, 0x6c, 0x79, 0x50, 0x66, 0xa3, 0xf8, 0xaf, 0x2c, 0xfa, 0xb4, 0x6b,
  0xec, 0x03, 0xad, 0xcb, 0xda, 0x24, 0x0c, 0x52, 0x07, 0x87, 0x88, 0xc0, 0x21, 0xf3, 0x02, 0xe8,
  0x24, 0x44, 0x0f, 0xcd, 0xa0, 0xad, 0x2f, 0x1b, 0x79, 0xab, 0x6b, 0x49, 0x4a, 0xe6, 0x3b, 0xd0,
  0xad, 0xc3, 0x48, 0xb9, 0xf7, 0xf1, 0x34, 0x09, 0xeb, 0x7a, 0xc0, 0xd5, 0x0d, 0x39, 0xd8, 0x45,
  0xce, 0x36, 0x7a, 0xd8, 0xde, 0x3c, 0xb0, 0x21, 0x96, 0x97, 0x8a, 0xff, 0x8b, 0x23, 0x60, 0x4f,
  0xf0, 0x3d, 0xd7, 0x8f, 0xf3, 0x2c, 0xcb, 0x1d, 0x48, 0x3f, 0x86, 0xc4, 0xa9, 0x00, 0xf2, 0x23,
  0x2d, 0x72, 0x4d, 0x66, 0xa5, 0x01, 0x02, 0x81, 0x81, 0x00, 0xdc, 0x4f, 0x99, 0x44, 0x0d, 0x7f,
  0x59, 0x46, 0x1e, 0x8f, 0xe7, 0x2d, 0x8d, 0xdd, 0x54, 0xc0, 0xf7, 0xfa, 0x46, 0x0d, 0x9d, 0x35,
  0x03, 0xf1, 0x7c, 0x12, 0xf3, 0x5a, 0x9d, 0x83, 0xcf, 0xdd, 0x37, 0x21, 0x7c, 0xb7, 0xee, 0xc3,
  0x39, 0xd2, 0x75, 0x8f, 0xb2, 0x2d, 0x6f, 0xec, 0xc6, 0x03, 0x55, 0xd7, 0x00, 0x67, 0xd3, 0x9b,
  0xa2, 0x68, 0x50, 0x6f, 0x9e, 0x28, 0xa4, 0x76, 0x39, 0x2b, 0xb2, 0x65, 0xcc, 0x72, 0x82, 0x93,
  0xa0, 0xcf, 0x10, 0x05, 0x6a, 0x75, 0xca, 0x85, 0x35, 0x99, 0xb0, 0xa6, 0xc6, 0xef, 0x4c, 0x4d,
  0x99, 0x7d, 0x2c, 0x38, 0x01, 0x21, 0xb5, 0x31, 0xac, 0x80, 0x54, 0xc4, 0x18, 0x4b, 0xfd, 0xef,
  0xb3, 0x30, 0x22, 0x51, 0x5a, 0xea, 0x7d, 0x9b, 0xb2, 0x9d, 0xcb, 0xba, 0x3f, 0xc0, 0x1a, 0x6b,
  0xcd, 0xb0, 0xe6, 0x2f, 0x04, 0x33, 0xd7, 0x3a, 0x49, 0x71, 0x02, 0x81, 0x81, 0x00, 0xd5, 0xd9,
  0xc9, 0x70, 0x1a, 0x13, 0xb3, 0x39, 0x24, 0x02, 0xee, 0xb0, 0xbb, 0x84, 0x17, 0x12, 0xc6, 0xbd,
  0x65, 0x73, 0xe9, 0x34, 0x5d, 0x43, 0xff, 0xdc, 0xf8, 0x55, 0xaf, 0x2a, 0xb9, 0xe1, 0xfa, 0x71,
  0x65, 0x4e, 0x50, 0x0f, 0xa4, 0x3b, 0xe5, 0x68, 0xf2, 0x49, 0x71, 0xaf, 0x15, 0x88, 0xd7, 0xaf,
  0xc4, 0x9d, 0x94, 0x84, 0x6b, 0x5b, 0x10, 0xd5, 0xc0, 0xaa, 0x0c, 0x13, 0x62, 0x99, 0xc0, 0x8b,
  0xfc, 0x90, 0x0f, 0x87, 0x40, 0x4d, 0x58, 0x88, 0xbd, 0xe2, 0xba, 0x3e, 0x7e, 0x2d, 0xd7, 0x69,
  0xa9, 0x3c, 0x09, 0x64, 0x31, 0xb6, 0xcc, 0x4d, 0x1f, 0x23, 0xb6, 0x9e, 0x65, 0xd6, 0x81, 0xdc,
  0x85, 0xcc, 0x1e, 0xf1, 0x0b, 0x84, 0x38, 0xab, 0x93, 0x5f, 0x9f, 0x92, 0x4e, 0x93, 0x46, 0x95,
  0x6b, 0x3e, 0xb6, 0xc3, 0x1b, 0xd7, 0x69, 0xa1, 0x0a, 0x97, 0x37, 0x78, 0xed, 0xd1, 0x02, 0x81,
  0x80, 0x33, 0x18, 0xc3, 0x13, 0x65, 0x8e, 0x03, 0xc6, 0x9f, 0x90, 0x00, 0xae, 0x30, 0x19, 0x05,
  0x6f, 0x3c, 0x14, 0x6f, 0xea, 0xf8, 0x6b, 0x33, 0x5e, 0xee, 0xc7, 0xf6, 0x69, 0x2d, 0xdf, 0x44,
  0x76, 0xaa, 0x32, 0xba, 0x1a, 0x6e, 0xe6, 0x18, 0xa3, 0x17, 0x61, 0x1c, 0x92, 0x2d, 0x43, 0x5d,
  0x29, 0xa8, 0xdf, 0x14, 0xd8, 0xff, 0xdb, 0x38, 0xef, 0xb8, 0xb8, 0x2a, 0x96, 0x82, 0x8e, 0x68,
  0xf4, 0x19, 0x8c, 0x42, 0xbe, 0xcc, 0x4a, 0x31, 0x21, 0xd5, 0x35, 0x6c, 0x5b, 0xa5, 0x7c, 0xff,
  0xd1, 0x85, 0x87, 0x28, 0xdc, 0x97, 0x75, 0xe8, 0x03, 0x80, 0x1d, 0xfd, 0x25, 0x34, 0x41, 0x31,
  0x21, 0x12, 0x87, 0xe8, 0x9a, 0xb7, 0x6a, 0xc0, 0xc4, 0x89, 0x31, 0x15, 0x45, 0x0d, 0x9c, 0xee,
  0xf0, 0x6a, 0x2f, 0xe8, 0x59, 0x45, 0xc7, 0x7b, 0x0d, 0x6c, 0x55, 0xbb, 0x43, 0xca, 0xc7, 0x5a,
  0x01, 0x02, 0x81, 0x81, 0x00, 0xab, 0xf4, 0xd5, 0xcf, 0x78, 0x88, 0x82, 0xc2, 0xdd, 0xbc, 0x25,
  0xe6, 0xa2, 0xc1, 0xd2, 0x33, 0xdc, 0xef, 0x0a, 0x97, 0x2b, 0xdc, 0x59, 0x6a, 0x86, 0x61, 0x4e,
  0xa6, 0xc7, 0x95, 0x99, 0xa6, 0xa6, 0x55, 0x6c, 0x5a, 0x8e, 0x72, 0x25, 0x63, 0xac, 0x52, 0xb9,
  0x10, 0x69, 0x83, 0x99, 0xd3, 0x51, 0x6c, 0x1a, 0xb3, 0x83, 0x6a, 0xff, 0x50, 0x58, 0xb7, 0x28,
  0x97, 0x13, 0xe2, 0xba, 0x94, 0x5b, 0x89, 0xb4, 0xea, 0xba, 0x31, 0xcd, 0x78, 0xe4, 0x4a, 0x00,
  0x36, 0x42, 0x00, 0x62, 0x41, 0xc6, 0x47, 0x46, 0x37, 0xea, 0x6d, 0x50, 0xb4, 0x66, 0x8f, 0x55,
  0x0c, 0xc8, 0x99, 0x91, 0xd5, 0xec, 0xd2, 0x40, 0x1c, 0x24, 0x7d, 0x3a, 0xff, 0x74, 0xfa, 0x32,
  0x24, 0xe0, 0x11, 0x2b, 0x71, 0xad, 0x7e, 0x14, 0xa0, 0x77, 0x21, 0x68, 0x4f, 0xcc, 0xb6, 0x1b,
  0xe8, 0x00, 0x49, 0x13, 0x21, 0x02, 0x81, 0x81, 0x00, 0xb6, 0x18, 0x73, 0x59, 0x2c, 0x4f, 0x92,
  0xac, 0xa2, 0x2e, 0x5f, 0xb6, 0xbe, 0x78, 0x5d, 0x47, 0x71, 0x04, 0x92, 0xf0, 0xd7, 0xe8, 0xc5,
  0x7a, 0x84, 0x6b, 0xb8, 0xb4, 0x30, 0x1f, 0xd8, 0x0d, 0x58, 0xd0, 0x64, 0x80, 0xa7, 0x21, 0x1a,
  0x48, 0x00, 0x37, 0xd6, 0x19, 0x71, 0xbb, 0x91, 0x20, 0x9d, 0xe2, 0xc3, 0xec, 0xdb, 0x36, 0x1c,
  0xca, 0x48, 0x7d, 0x03, 0x32, 0x74, 0x1e, 0x65, 0x73, 0x02, 0x90, 0x73, 0xd8, 0x3f, 0xb5, 0x52,
  0x35, 0x79, 0x1c, 0xee, 0x93, 0xa3, 0x32, 0x8b, 0xed, 0x89, 0x98, 0xf1, 0x0c, 0xd8, 0x12, 0xf2,
  0x89, 0x7f, 0x32, 0x23, 0xec, 0x67, 0x66, 0x52, 0x83, 0x89, 0x99, 0x5e, 0x42, 0x2b, 0x42, 0x4b,
  0x84, 0x50, 0x1b, 0x3e, 0x47, 0x6d, 0x74, 0xfb, 0xd1, 0xa6, 0x10, 0x20, 0x6c, 0x6e, 0xbe, 0x44,
  0x3f, 0xb9, 0xfe, 0xbc, 0x8d, 0xda, 0xcb, 0xea, 0x8f
};

static const char *WHITESPACE_CHARS = " \n\r\t";

/**
 * Modify str in place to erase whitespace on the left.
 * @param str
 */
static inline void
trimLeft(string& str)
{
  size_t found = str.find_first_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found > 0)
      str.erase(0, found);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the right.
 * @param str
 */
static inline void
trimRight(string& str)
{
  size_t found = str.find_last_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found + 1 < str.size())
      str.erase(found + 1);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the left and right.
 * @param str
 */
static void
trim(string& str)
{
  trimLeft(str);
  trimRight(str);
}

static void
onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
{
  cout << "Register failed for prefix " << prefix->toUri() << endl;
}

/**
 * Poll stdin and return true if it is ready to ready (e.g. from stdinReadLine).
 */
static bool
isStdinReady()
{
  struct pollfd pollInfo;
  pollInfo.fd = STDIN_FILENO;
  pollInfo.events = POLLIN;

  return poll(&pollInfo, 1, 0) > 0;
}

/**
 * Read a line from from stdin and return a trimmed string.  (We don't use
 * cin because it ignores a blank line.)
 */
static string
stdinReadLine()
{
  char inputBuffer[1000];
  ssize_t nBytes = ::read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1);
  if (nBytes < 0)
    // Don't expect an error reading from stdin.
    throw runtime_error("stdinReadLine: error reading from STDIN_FILENO");

  inputBuffer[nBytes] = 0;
  string input(inputBuffer);
  trim(input);

  return input;
}

int main(int argc, char** argv)
{
  try {
    cout << "Enter your chat username:" << endl;
    string screenName = stdinReadLine();

    string defaultHubPrefix = "ndn/edu/ucla/remap";
    cout << "Enter your hub prefix [" << defaultHubPrefix << "]" << endl;
    string hubPrefix = stdinReadLine();
    if (hubPrefix == "")
      hubPrefix = defaultHubPrefix;

    string defaultChatRoom = "ndnchat";
    cout << "Enter the chatroom name [" << defaultChatRoom << "]:" << endl;
    string chatRoom = stdinReadLine();
    if (chatRoom == "")
      chatRoom = defaultChatRoom;

    const char* host = "localhost";
    cout << "Connecting to " << host << ", Chatroom: " << chatRoom <<
      ", Username: " << screenName << endl << endl;

    // Set up the key chain.
    Face face(host);

    ptr_lib::shared_ptr<MemoryIdentityStorage> identityStorage
      (new MemoryIdentityStorage());
    ptr_lib::shared_ptr<MemoryPrivateKeyStorage> privateKeyStorage
      (new MemoryPrivateKeyStorage());
    KeyChain keyChain
      (ptr_lib::make_shared<IdentityManager>(identityStorage, privateKeyStorage),
       ptr_lib::shared_ptr<NoVerifyPolicyManager>(new NoVerifyPolicyManager()));
    keyChain.setFace(&face);
    Name keyName("/testname/DSK-123");
    Name certificateName = keyName.getSubName(0, keyName.size() - 1).append
      ("KEY").append(keyName.get(keyName.size() - 1)).append("ID-CERT").append("0");
    identityStorage->addKey
      (keyName, KEY_TYPE_RSA, Blob(DEFAULT_RSA_PUBLIC_KEY_DER,
       sizeof(DEFAULT_RSA_PUBLIC_KEY_DER)));
    privateKeyStorage->setKeyPairForKeyName
      (keyName, KEY_TYPE_RSA, DEFAULT_RSA_PUBLIC_KEY_DER, sizeof(DEFAULT_RSA_PUBLIC_KEY_DER),
       DEFAULT_RSA_PRIVATE_KEY_DER, sizeof(DEFAULT_RSA_PRIVATE_KEY_DER));
    face.setCommandSigningInfo(keyChain, certificateName);

    ptr_lib::shared_ptr<Chat> chat(new Chat
      (screenName, chatRoom, Name(hubPrefix), face, keyChain, certificateName));
    chat->initialize();

    // The main loop to process Chat while checking stdin to send a message.
    cout << "Enter your chat message. To quit, enter \"leave\" or \"exit\"." << endl;
    while (true) {
      if (isStdinReady()) {
        string input = stdinReadLine();
        if (input == "leave" || input == "exit")
          // We will send the leave message below.
          break;

        chat->sendMessage(input);
      }

      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }

    // The user entered the command to leave.
    chat->leave();
    // Wait a little bit to allow other applications to fetch the leave message.
    ndn_MillisecondsSince1970 startTime = Chat::getNowMilliseconds();
    while (true)
    {
      if (Chat::getNowMilliseconds() - startTime >= 1000.0)
        break;

      face.processEvents();
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}

#else // NDN_CPP_HAVE_PROTOBUF

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  cout <<
    "This program uses Protobuf but it is not installed. Install it and ./configure again." << endl;
}

#endif // NDN_CPP_HAVE_PROTOBUF
