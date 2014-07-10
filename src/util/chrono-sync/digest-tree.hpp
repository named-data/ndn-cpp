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
     * @param prefix_name
     * @param seqno_seq
     * @param seqno_session
     */
    Node
      (const std::string& prefix_name, int seqno_seq, int seqno_session)
    : prefix_name_(prefix_name), seqno_seq_(seqno_seq),
      seqno_session_(seqno_session)
    {
      recomputeDigest();
    }

    const std::string&
    getPrefixName() const { return prefix_name_; }

    int
    getSequenceNo() const { return seqno_seq_; }

    int
    getSessionNo() const { return seqno_session_; }

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
      seqno_seq_ = sequenceNo;
      recomputeDigest();
    }

    /**
     * Compare shared_ptrs to Node based on prefix_name_ and seqno_session_.
     */
    class Compare {
    public:
      bool
      operator()
        (const ptr_lib::shared_ptr<const Node>& node1,
         const ptr_lib::shared_ptr<const Node>& node2) const
      {
        int nameComparison = node1->prefix_name_.compare(node2->prefix_name_);
        if (nameComparison != 0)
          return nameComparison;

        if (node1->seqno_session_ > node2->seqno_session_)
          return 1;
        else if (node1->seqno_session_ < node2->seqno_session_)
          return -1;
        else
          return 0;
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

    std::string prefix_name_;
    int seqno_seq_;
    int seqno_session_;
    std::string digest_;
  };

  /**
   * Update the digest tree and recompute the root digest.  If the combination
   * of prefixName and sessionNo already exists in the tree then update its
   * sequenceNo (only if the given sequenceNo is newer), otherwise add a new node.
   * @param prefixName The prefix name.
   * @param sessionNo The session number.
   * @param sequenceNo The new sequence number.
   * @return True if the digest tree is updated, false if not (because the
   * given sequenceNo is not newer than the existing sequence number).
   */
  bool
  update(const std::string& prefixName, int sessionNo, int sequenceNo);

  int
  find(const std::string& prefixName, int sessionNo) const;

  size_t
  size() const { return digestnode_.size(); }

  const DigestTree::Node&
  get(size_t i) const { return *digestnode_[i]; }

  /**
   * Get the root digest.
   * @return The root digest as a hex string.
   */
  const std::string&
  getRoot() const { return root_; }

private:
  /**
   * Set root_ to the digest of all digests in digestnode_. This sets root_
   * to the hex value of the digest.
   */
  void
  recomputeRoot();

#if 0
  void printTree() const;
#endif

  std::vector<ptr_lib::shared_ptr<DigestTree::Node> > digestnode_;
  std::string root_;
  Node::Compare nodeCompare_;
};

}

#endif

