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

#ifndef NDN_CHRONO_SYNC_HPP
#define NDN_CHRONO_SYNC_HPP

#include <vector>
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include "digest-tree.hpp"

namespace google { namespace protobuf { template <typename Element> class RepeatedPtrField; } }
namespace Sync { class SyncState; }

namespace ndn {

class ChronoSync {
public:
  typedef func_lib::function<void
    (const google::protobuf::RepeatedPtrField<Sync::SyncState>& content)> SendChatInterest;

  typedef func_lib::function<void(int sequence)> InitialChat;

  ChronoSync
    (SendChatInterest sendchatinterest, InitialChat initialchat,
     const std::string& chatroom, int session, Transport& transport, Face& face,
     KeyChain& keyChain, const Name& certificateName, Milliseconds sync_lifetime)
  : sendChatInterest_(sendchatinterest), initialChat_(initialchat),
    chatroom_(chatroom), session_(session), transport_(transport), face_(face),
    keyChain_(keyChain), certificateName_(certificateName),
    sync_lifetime_(sync_lifetime), prefix_("/ndn/broadcast/ChronoChat-0.3/"),
    usrseq_(-1), flag_(0)
  {}

  class DigestLogEntry {
  public:
    DigestLogEntry
      (const std::string& digest,
       const google::protobuf::RepeatedPtrField<Sync::SyncState>& data);

    // TODO: Make private.
    std::string digest_;
    ptr_lib::shared_ptr<google::protobuf::RepeatedPtrField<Sync::SyncState> > data_;
  };

  void
  pokeData(const Data& data) { transport_.send(*data.wireEncode()); }

  // Search the digest log by digest.
  int
  logfind(const std::string& digest);

  /**
   * Get the digest tree for read-only.
   * @return A const reference to the digest tree.
   */
  const DigestTree&
  getDigestTree() const { return digest_tree_; }

  /**
   * Update the digest tree with the messages in content.
   * @param content The sync state messages.
   */
  void
  update(const google::protobuf::RepeatedPtrField<Sync::SyncState >& content);

  // Process Sync Interest.
  void
  onInterest
    (const ptr_lib::shared_ptr<const Name>& prefix,
     const ptr_lib::shared_ptr<const Interest>& inst, Transport& transport,
     uint64_t registerPrefixId);

  // Process Sync Data.
  void
  onData
    (const ptr_lib::shared_ptr<const Interest>& inst,
     const ptr_lib::shared_ptr<Data>& co);

  // Process Recovery Interest, go through digest tree and send data including info of all nodes
  void
  processRecoveryInst
    (const Interest& inst, const std::string& syncdigest, Transport& transport);

  // Common Interest Processing, using digest log to find the difference after syncdigest_t
  void
  processSyncInst(int index, const std::string& syncdigest_t, Transport& transport);

  // Send Recovery Interest.
  void
  sendRecovery(const std::string& syncdigest_t);

  // Check if recovery is needed.
  void
  judgeRecovery(const std::string& syncdigest_t, Transport& transport);

  // Sync interest time out, if the interest is the static one send again.
  void
  syncTimeout(const ptr_lib::shared_ptr<const Interest>& interest);

  // Process initial data which usually include all other users' info in the chatroom, and send back the new comer's own info.
  void
  initialOndata(const google::protobuf::RepeatedPtrField<Sync::SyncState >& content);

  // Initial sync interest timeout, which means there are no other people in the chatroom.
  void
  initialTimeOut(const ptr_lib::shared_ptr<const Interest>& interest);

  /**
   * Add a new entry to the digest log.
   * @param digest The digest. This is copied.
   * @param data The data. This is copied.
   */
  void addDigestLogEntry
    (const std::string& digest,
     const google::protobuf::RepeatedPtrField<Sync::SyncState>& data)
  {
    digest_log_.push_back(ptr_lib::make_shared<DigestLogEntry>(digest, data));
  }

  int
  getFlag() const { return flag_; }

  // TODO: Make private.
  int usrseq_;
  std::string prefix_;
  std::string chat_prefix_;

private:
  Transport& transport_;
  Face& face_;
  KeyChain& keyChain_;
  Name certificateName_;
  Milliseconds sync_lifetime_;
  SendChatInterest sendChatInterest_;
  InitialChat initialChat_;
  std::vector<ptr_lib::shared_ptr<DigestLogEntry> > digest_log_;
  DigestTree digest_tree_;
  std::string chatroom_;
  int flag_; // This will not display the old chatmsg on the screen if the flag is 1.
  int session_;
};

}

#endif
