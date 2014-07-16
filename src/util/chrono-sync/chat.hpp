/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_CHAT_HPP
#define NDN_CHAT_HPP

#include <vector>
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/util/chrono-sync/chrono-sync.hpp>

namespace ndn {

class Chat {
public:
  Chat
    (const std::string& screenName, const std::string& chatRoom,
     const std::string& localPrefix, Transport& transport, Face& face,
     KeyChain& keyChain, const Name& certificateName);

  // Send a chat message.
  void
  sendMessage(const std::string& chatmsg);

  // Send leave message and leave.
  void
  leave();

private:
  // Initialization: push the JOIN message in to the msgcache, update roster and start heartbeat.
  void
  initial();
  
  // Send a Chat Interest to fetch chat messages after get the user gets the Sync data packet back but will not send interest.
  void
  sendInterest
    (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content,
     bool isRecovery);
  
  // Send back Chat Data Packet which contains the user's message.
  void 
  onInterest
    (const ptr_lib::shared_ptr<const Name>& prefix, 
     const ptr_lib::shared_ptr<const Interest>& inst, Transport& transport,
     uint64_t registeredPrefixId);
  
  // Processing the incoming Chat data.
  void 
  onData
    (const ptr_lib::shared_ptr<const Interest>& inst, 
     const ptr_lib::shared_ptr<Data>& co);
  
  void 
  chatTimeout(const ptr_lib::shared_ptr<const Interest>& interest);
  
  // Send a hearbeat message.
  void
  heartbeat();
  
  // Check whether a user is leaving by checking whether his seqno has changed.
  void
  alive
    (int temp_seq, const std::string& name, int session, 
     const std::string& prefix);
  
  // Generate a random name for ChronoSync.
  static std::string
  getRandomString();

  static void
  onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix);

  class CachedMessage {
  public:
    CachedMessage
      (int seqno, int msgtype, std::string msg, MillisecondsSince1970 time)
    : seqno_(seqno), msgtype_(msgtype), msg_(msg), time_(time)
    {}

    int
    getSequenceNo() const { return seqno_; }

    int
    getMessageType() const { return msgtype_; }

    const std::string&
    getMessage() const { return msg_; }

    MillisecondsSince1970
    getTime() const { return time_; }

  private:
    int seqno_;
    // This is really enum SyncDemo::ChatMessage_ChatMessageType, but make it
    //   in int so that the head doesn't need to include the protobuf header.
    int msgtype_;
    std::string msg_;
    MillisecondsSince1970 time_;
  };

  std::vector<ptr_lib::shared_ptr<CachedMessage> > msgcache_;
  std::vector<std::string> roster_;
  size_t maxmsgcachelength_;
  bool isRecoverySyncState_;
  std::string screen_name_;
  std::string chatroom_;
  std::string usrname_;
  std::string chat_prefix_;
  Milliseconds sync_lifetime_;
  ptr_lib::shared_ptr<ChronoSync> sync_;
  Face& face_;
  KeyChain& keyChain_;
  Name certificateName_;
};

}

#endif
