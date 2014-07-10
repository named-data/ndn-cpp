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

#include <algorithm>
#include <openssl/ssl.h>
#include "../../c/util/ndn_memory.h"
#include "../logging.hpp"
#include "sync-state.pb.h"
#include "chrono-sync.hpp"

using namespace std;

namespace ndn {

#if 0
void DigestTree::printTree() const
{
  cout << "root " << root_ << " size " << digestnode_.size() << endl;
}
#endif

/**
 * Convert the hex character to an integer from 0 to 15, or -1 if not a hex character.
 * @param c
 * @return
 */
static int
fromHexChar(uint8_t c)
{
  if (c >= '0' && c <= '9')
    return (int)c - (int)'0';
  else if (c >= 'A' && c <= 'F')
    return (int)c - (int)'A' + 10;
  else if (c >= 'a' && c <= 'f')
    return (int)c - (int)'a' + 10;
  else
    return -1;
}

/**
 * Convert the hex string to bytes and call SHA256_Update.
 * @param context The SHA256_CTX to update.
 * @param hex The hex string.
 * @return The result from SHA256_Update.
 */
static int
SHA256_UpdateHex(SHA256_CTX *context, const string& hex)
{
  vector<uint8_t> data(hex.size() / 2);
  for (size_t i = 0; i < data.size(); ++i)
    data[i] = (uint8_t)(16 * fromHexChar(hex[2 * i]) + fromHexChar(hex[2 * i + 1]));

  return SHA256_Update(context, &data[0], data.size());
}

void
DigestTree::initial(ChronoSync& self)
{
  digestnode_.push_back
    (ptr_lib::make_shared<Node>(self.chat_prefix_, 0, self.session_));

  recomputeRoot();

#if 0
  printTree();
#endif
}

void
DigestTree::update
  (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content,
   ChronoSync& self)
{
  for (size_t i = 0; i < content.size(); ++i) {
    if (content.Get(i).type() == 0) {
      int n_index = find(content.Get(i).name(), content.Get(i).seqno().session());
      _LOG_DEBUG(content.Get(i).name() << ", " << content.Get(i).seqno().session());
      _LOG_DEBUG("DigestTree::update session " << content.Get(i).seqno().session() << ", n_index " << n_index);
      if (n_index >= 0) {
        //only update the newer status
        if (digestnode_[n_index]->getSequenceNo() < content.Get(i).seqno().seq()) {
          if (self.chat_prefix_ == content.Get(i).name())
            self.usrseq_ = content.Get(i).seqno().seq();

          digestnode_[n_index]->setSequenceNo(content.Get(i).seqno().seq());
        }
      }
      else {
        _LOG_DEBUG("new comer " << content.Get(i).name() << "," <<
                   content.Get(i).seqno().seq() << "," <<
                   content.Get(i).seqno().session());
        if (self.chat_prefix_ == content.Get(i).name())
          self.usrseq_ = content.Get(i).seqno().seq();

        // Insert into digestnode_ sorted.
        ptr_lib::shared_ptr<Node> temp(new Node
          (content.Get(i).name(), content.Get(i).seqno().seq(),
           content.Get(i).seqno().session()));
        digestnode_.insert
          (std::lower_bound(digestnode_.begin(), digestnode_.end(), temp, nodeCompare_),
           temp);
      }
    }
  }

  recomputeRoot();

#if 0
  printTree();
#endif
}

void
DigestTree::recomputeRoot()
{
  SHA256_CTX sha256;

  SHA256_Init(&sha256);
  for (size_t i = 0; i < digestnode_.size(); ++i)
    SHA256_UpdateHex(&sha256, digestnode_[i]->getDigest());
  uint8_t digest_root[SHA256_DIGEST_LENGTH];
  SHA256_Final(&digest_root[0], &sha256);
  root_ = toHex(digest_root, sizeof(digest_root));
  _LOG_DEBUG("update root to: " + root_);
}

int
DigestTree::find(const string& name, int session) const
{
  for (size_t i = 0; i < digestnode_.size(); ++i) {
    if (digestnode_[i]->getPrefixName() == name &&
        digestnode_[i]->getSessionNo() == session)
      return i;
  }

  return -1;
}

void
DigestTree::Node::recomputeDigest()
{
  SHA256_CTX sha256;

  SHA256_Init(&sha256);
  uint8_t number[4];
  int32ToLittleEndian(seqno_session_, number);
  SHA256_Update(&sha256, number, sizeof(number));
  int32ToLittleEndian(seqno_seq_, number);
  SHA256_Update(&sha256, number, sizeof(number));
  uint8_t digest_seq[SHA256_DIGEST_LENGTH];
  SHA256_Final(digest_seq, &sha256);

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, &prefix_name_[0], prefix_name_.size());
  uint8_t digest_name[SHA256_DIGEST_LENGTH];
  SHA256_Final(digest_name, &sha256);

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, digest_name, sizeof(digest_name));
  SHA256_Update(&sha256, digest_seq, sizeof(digest_seq));
  uint8_t digest_node[SHA256_DIGEST_LENGTH];
  SHA256_Final(digest_node, &sha256);
  digest_ = toHex(digest_node, sizeof(digest_node));
}

void
DigestTree::Node::int32ToLittleEndian(uint32_t value, uint8_t* result)
{
  for (size_t i = 0; i < 4; i++) {
    result[i] = value % 256;
    value = value / 256;
   }
}

}