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
#include "../../face.hpp"
#include "../../security/key-chain.hpp"

namespace google { namespace protobuf { template <typename Element> class RepeatedPtrField; } }
namespace Sync { class SyncStateMsg; }
namespace Sync { class SyncState; }

namespace ndn {

class DigestTree;

class ChronoSync {
public:
  typedef func_lib::function<void
    (const google::protobuf::RepeatedPtrField<Sync::SyncState>& content)> SendChatInterest;

  typedef func_lib::function<void()> InitialChat;

  /**
   * Create a new ChronoSync to communicate using the given face. Initialize
   * the broadcast prefix.  Initialize the digest log with a digest of "00" and
   * and empty content. Register the prefix to receive interests for the
   * chatroom and express an interest for the initial root digest "00".
   * @param sendchatinterest
   * @param initialchat
   * @param chatPrefix
   * @param chatroom
   * @param session
   * @param transport
   * @param face
   * @param keyChain
   * @param certificateName
   * @param sync_lifetime
   * @param onRegisterFailed A function object to call if failed to register the
   * prefix to receive interests for the chatroom. This calls
   * onRegisterFailed(prefix) where prefix is the broadcast prefix + chatroom.
   */
  ChronoSync
    (SendChatInterest sendchatinterest, InitialChat initialchat,
     const std::string& chatPrefix, const std::string& chatroom, int session,
     Transport& transport, Face& face, KeyChain& keyChain,
     const Name& certificateName, Milliseconds sync_lifetime,
     const OnRegisterFailed& onRegisterFailed);

  /**
   * Get the current sequence number in the digest tree for the given
   * producer namePrefix and sessionNo.
   * @param namePrefix The producer name prefix.
   * @param sessionNo The producer session number.
   * @return The current producer sequence number, or -1 if the producer
   * namePrefix and sessionNo are not in the digest tree.
   */
  int
  getProducerSequenceNo(const std::string& namePrefix, int sessionNo) const;

  /**
   * Increment the sequence number, create a sync message with the new
   * sequence number and publish a data packet where the name is
   * the broadcast prefix + chatroom_ + the root digest of the current digest
   * tree. Then add the sync message to the digest tree and digest log which
   * creates a new root digest. Finally, express an interest for the next sync
   * update with the name broadcast prefix + chatroom_ + the new root digest.
   * After this, you should publish the content for the new sequence number.
   * You can get the new sequence number with getSequenceNo().
   */
  void
  publishNextSequenceNo();

  int
  getFlag() const { return flag_; }

  const std::string&
  getBroadcastPrefix() const { return broadcastPrefix_; }

  int
  getSequenceNo() const { return usrseq_; }

private:
  class DigestLogEntry {
  public:
    DigestLogEntry
      (const std::string& digest,
       const google::protobuf::RepeatedPtrField<Sync::SyncState>& data);

    const std::string&
    getDigest() const { return digest_; }

    const google::protobuf::RepeatedPtrField<Sync::SyncState>&
    getData() const { return *data_; }

  private:
    std::string digest_;
    ptr_lib::shared_ptr<google::protobuf::RepeatedPtrField<Sync::SyncState> > data_;
  };

  /**
   * Make a data packet with the syncMessage and with name 
   * broadcastPrefix_ + chatroom_ + "/" + digest. Sign and send.
   * @param digest The root digest as a hex string for the data packet name.
   * @param syncMessage The SyncStateMsg which updates the digest tree state
   * with the given digest.
   */
  void
  broadcastSyncState
    (const std::string& digest, const Sync::SyncStateMsg& syncMessage);

  /**
   * Update the digest tree with the messages in content. If the digest tree
   * root is not in the digest log, also add a log entry with the content.
   * @param content The sync state messages.
   * @return True if added a digest log entry (because the updated digest
   * tree root was not in the log), false if didn't add a log entry.
   */
  bool
  update(const google::protobuf::RepeatedPtrField<Sync::SyncState >& content);

  // Search the digest log by digest.
  int
  logfind(const std::string& digest) const;

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

  // Initial sync interest timeout, which means there are no other people in the chatroom.
  void
  initialTimeOut(const ptr_lib::shared_ptr<const Interest>& interest);

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

  std::string broadcastPrefix_;
  Transport& transport_;
  Face& face_;
  KeyChain& keyChain_;
  Name certificateName_;
  Milliseconds sync_lifetime_;
  SendChatInterest sendChatInterest_;
  InitialChat initialChat_;
  std::vector<ptr_lib::shared_ptr<DigestLogEntry> > digest_log_;
  ptr_lib::shared_ptr<DigestTree> digest_tree_;
  std::string chat_prefix_;
  std::string chatroom_;
  int flag_; // This will not display the old chatmsg on the screen if the flag is 1.
  int session_;
  int usrseq_;
};

}

#endif
