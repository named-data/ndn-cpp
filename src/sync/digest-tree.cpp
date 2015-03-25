/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#include <algorithm>
#include <openssl/ssl.h>
#include "../util/logging.hpp"
#include "digest-tree.hpp"

INIT_LOGGER("ndn.DigestTree");

using namespace std;

namespace ndn {

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

bool
DigestTree::update(const std::string& dataPrefix, int sessionNo, int sequenceNo)
{
  int index = find(dataPrefix, sessionNo);
  _LOG_DEBUG(dataPrefix << ", " << sessionNo);
  _LOG_DEBUG("DigestTree::update session " << sessionNo << ", index " << index);
  if (index >= 0) {
    // only update the newer status
    if (digestNode_[index]->getSequenceNo() < sequenceNo)
      digestNode_[index]->setSequenceNo(sequenceNo);
    else
      return false;
  }
  else {
    _LOG_DEBUG("new comer " << dataPrefix << ", session " << sessionNo <<
               ", sequence " << sequenceNo);
    // Insert into digestnode_ sorted.
    ptr_lib::shared_ptr<Node> temp(new Node(dataPrefix, sessionNo, sequenceNo));
    digestNode_.insert
      (std::lower_bound(digestNode_.begin(), digestNode_.end(), temp, nodeCompare_),
       temp);
  }

  recomputeRoot();
  return true;
}

void
DigestTree::recomputeRoot()
{
  SHA256_CTX sha256;

  SHA256_Init(&sha256);
  for (size_t i = 0; i < digestNode_.size(); ++i)
    SHA256_UpdateHex(&sha256, digestNode_[i]->getDigest());
  uint8_t digestRoot[ndn_SHA256_DIGEST_SIZE];
  SHA256_Final(&digestRoot[0], &sha256);
  root_ = toHex(digestRoot, sizeof(digestRoot));
  _LOG_DEBUG("update root to: " + root_);
}

int
DigestTree::find(const string& dataPrefix, int sessionNo) const
{
  for (size_t i = 0; i < digestNode_.size(); ++i) {
    if (digestNode_[i]->getDataPrefix() == dataPrefix &&
        digestNode_[i]->getSessionNo() == sessionNo)
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
  int32ToLittleEndian(sessionNo_, number);
  SHA256_Update(&sha256, number, sizeof(number));
  int32ToLittleEndian(sequenceNo_, number);
  SHA256_Update(&sha256, number, sizeof(number));
  uint8_t sequenceDigest[ndn_SHA256_DIGEST_SIZE];
  SHA256_Final(sequenceDigest, &sha256);

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, &dataPrefix_[0], dataPrefix_.size());
  uint8_t nameDigest[ndn_SHA256_DIGEST_SIZE];
  SHA256_Final(nameDigest, &sha256);

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, nameDigest, sizeof(nameDigest));
  SHA256_Update(&sha256, sequenceDigest, sizeof(sequenceDigest));
  uint8_t nodeDigest[ndn_SHA256_DIGEST_SIZE];
  SHA256_Final(nodeDigest, &sha256);
  digest_ = toHex(nodeDigest, sizeof(nodeDigest));
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