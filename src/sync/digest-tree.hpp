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

#ifndef NDN_DIGEST_TREE_HPP
#define NDN_DIGEST_TREE_HPP

#include <ndn-cpp/common.hpp>
#include <string>

namespace ndn {

class DigestTree {
public:
  DigestTree()
  : root_("00")
  {}

  class Node {
  public:
    /**
     * Create a new DigestTree::Node with the given fields and compute the digest.
     * @param dataPrefix The data prefix.
     * @param sessionNo The sequence number.
     * @param sequenceNo The session number.
     */
    Node(const std::string& dataPrefix, int sessionNo, int sequenceNo)
    : dataPrefix_(dataPrefix),
      sessionNo_(sessionNo),
      sequenceNo_(sequenceNo)
    {
      recomputeDigest();
    }

    const std::string&
    getDataPrefix() const { return dataPrefix_; }

    int
    getSessionNo() const { return sessionNo_; }

    int
    getSequenceNo() const { return sequenceNo_; }

    /**
     * Get the digest.
     * @return The digest as a hex string.
     */
    const std::string&
    getDigest() const { return digest_; }

    /**
     * Set the sequence number and recompute the digest.
     * @param sequenceNo The new sequence number.
     */
    void
    setSequenceNo(int sequenceNo)
    {
      sequenceNo_ = sequenceNo;
      recomputeDigest();
    }

    /**
     * Compare shared_ptrs to Node based on dataPrefix_ and seqno_session_.
     */
    class Compare {
    public:
      bool
      operator()
        (const ptr_lib::shared_ptr<const Node>& node1,
         const ptr_lib::shared_ptr<const Node>& node2) const
      {
        int nameComparison = node1->dataPrefix_.compare(node2->dataPrefix_);
        if (nameComparison != 0)
          return nameComparison < 0;

        return node1->sessionNo_ < node2->sessionNo_;
      }
    };

  private:
    /**
     * Digest the fields and set digest_ to the hex digest.
     */
    void
    recomputeDigest();

    static void
    int32ToLittleEndian(uint32_t value, uint8_t* result);

    std::string dataPrefix_;
    int sessionNo_;
    int sequenceNo_;
    std::string digest_;
  };

  /**
   * Update the digest tree and recompute the root digest.  If the combination
   * of dataPrefix and sessionNo already exists in the tree then update its
   * sequenceNo (only if the given sequenceNo is newer), otherwise add a new node.
   * @param dataPrefix The name prefix.
   * @param sessionNo The session number.
   * @param sequenceNo The new sequence number.
   * @return True if the digest tree is updated, false if not (because the
   * given sequenceNo is not newer than the existing sequence number).
   */
  bool
  update(const std::string& dataPrefix, int sessionNo, int sequenceNo);

  int
  find(const std::string& dataPrefix, int sessionNo) const;

  size_t
  size() const { return digestNode_.size(); }

  const DigestTree::Node&
  get(size_t i) const { return *digestNode_[i]; }

  /**
   * Get the root digest.
   * @return The root digest as a hex string.
   */
  const std::string&
  getRoot() const { return root_; }

private:
  /**
   * Set root_ to the digest of all digests in digestNode_. This sets root_
   * to the hex value of the digest.
   */
  void
  recomputeRoot();

  std::vector<ptr_lib::shared_ptr<DigestTree::Node> > digestNode_;
  std::string root_;
  Node::Compare nodeCompare_;
};

}

#endif

