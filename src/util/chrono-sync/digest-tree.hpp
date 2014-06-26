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

namespace google { namespace protobuf { template <typename Element> class RepeatedPtrField; } }
namespace Sync { class SyncState; }

namespace ndn {

class ChronoSync;

class DigestTree {
public:
  DigestTree()
  : root_("00")
  {}

  // Initialize after the first interest timeout.
  void
  initial(ChronoSync& self);

  // Add new comer to the tree.
  void
  newcomer
    (const std::string& name, int seqno_seq, int seqno_session, ChronoSync& self);

  // Update the digest_tree when we get some new data.
  void
  update
    (const google::protobuf::RepeatedPtrField<Sync::SyncState >& content,
     ChronoSync& self);

  int
  find(const std::string& name, int session) const;

  // TODO: Make private.

  class Node {
  public:
    Node
      (const std::string& prefix_name, int seqno_seq, int seqno_session,
       const std::string& digest)
    : prefix_name_(prefix_name), seqno_seq_(seqno_seq),
      seqno_session_(seqno_session), digest_(digest)
    {}

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

    std::string prefix_name_;
    int seqno_seq_;
    int seqno_session_;
    std::string digest_;
  };

  std::vector<ptr_lib::shared_ptr<DigestTree::Node> > digestnode_;
  std::string root_;

private:
  void
  static int32ToLittleEndian(uint32_t value, uint8_t* result);

#if 0
  void printTree() const;
#endif

  Node::Compare nodeCompare_;
};

}

#endif
