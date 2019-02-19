/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/producer-base.hpp
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

#ifndef NDN_PSYNC_PRODUCER_BASE_HPP
#define NDN_PSYNC_PRODUCER_BASE_HPP

#include <map>
#include "../face.hpp"
#include "../security/key-chain.hpp"

namespace ndn {

class InvertibleBloomLookupTable;
class PSyncSegmentPublisher;

/**
 * PSyncProducerBase is a base class for PsyncPartialProducer::Impl and
 * PsyncFullProducer::Impl.
 */
class PSyncProducerBase : public ptr_lib::enable_shared_from_this<PSyncProducerBase> {
public:
  /**
   * Return the current sequence number of the given prefix.
   * @param prefix The prefix for the sequence number.
   * @return The sequence number for the prefix, or -1 if not found.
   */
  int
  getSequenceNo(const Name& prefix) const;

  /**
   * Add a user node for synchronization based on the prefix Name, and
   * initialize the sequence number to zero. However, if the prefix Name already
   * exists, then do nothing and return false. This does not add sequence number
   * zero to the IBLT because, if a large number of user nodes are added, then
   * decoding the difference between our own IBLT and the other IBLT will not be
   * possible.
   * @param prefix The prefix Name of the user node to be added.
   * @return True if the user node with the prefix Name was added, false if the
   * prefix Name already exists.
   */
  bool
  addUserNode(const Name& prefix);

  /**
   * Remove the user node from the synchronization. This erases the prefix from
   * the IBLT and other tables.
   * @param prefix The prefix Name of the user node to be removed. If there is
   * no user node with this prefix, do nothing.
   */
  void
  removeUserNode(const Name& prefix);

protected:
  /**
   * Create a PSyncProducerBase.
   * @param expectedNEntries The expected number of entries in the IBLT.
   * @param face The application's Face.
   * @param syncPrefix The prefix Name of the sync group, which is copied.
   * @param userPrefix The prefix Name of the first user in the group, which is
   * copied.
   * @param keyChain The KeyChain for signing Data packets.
   * @param syncReplyFreshnessPeriod The freshness period of the sync
   * Data packet, in milliseconds.
   * @param helloReplyFreshness The freshness period of the hello Data packet,
   * in milliseconds.
   * @param signingInfo (optional) The SigningInfo for signing Data packets,
   * which is copied. If omitted, use the default SigningInfo().
   */
  PSyncProducerBase
    (size_t expectedNEntries, Face& face, const Name& syncPrefix,
     const Name& userPrefix, KeyChain& keyChain,
     Milliseconds syncReplyFreshnessPeriod,
     Milliseconds helloReplyFreshnessPeriod,
     const SigningInfo& signingInfo = SigningInfo());

  /**
   * Update prefixes_ and iblt_ with the given prefix and sequence number.
   * Whoever calls this needs to make sure that prefix is in prefixes_.
   * We remove an already-existing prefix/sequence number from iblt_ (unless
   * sequenceNo is zero because we don't insert a zero sequence number into the
   * IBLT.) Then we update prefix_, prefixTohash_, hashToprefix_, and iblt_ .
   * @param prefix The prefix of the sequence number to update.
   * @param sequenceNumber The new sequence number.
   */
  void
  updateSequenceNo(const Name& prefix, int sequenceNo);

  /**
   * Send a Data packet with content type NACK to the Face. The Data packet is
   * signed with the keyChain and signingInfo given to the constructor. The
   * producer sends a Nack to the consumer if the consumer has a very old IBLT
   * whose differences with the latest IBLT can't be decoded successfully.
   * @param name The base Name for the application Nack Data packet, which is
   * copied. The Data Name will include the encoded IBLT and segment number 0.
   */
  void
  sendApplicationNack(const Name& name);

  /**
   * This is called when registerPrefix fails to log an error message.
   */
  void
  onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix);

  ptr_lib::shared_ptr<InvertibleBloomLookupTable> iblt_;
  size_t expectedNEntries_;
  // threshold_ is used to check if an IBLT difference is greater than the
  // threshold, and whether we need to update the other IBLT.
  size_t threshold_;

  // The key is the prefix Name. The value is the sequence number for the prefix.
  std::map<Name, int> prefixes_;
  // prefixToHash_ and hashToPrefix_ are just for looking up the hash more
  // quickly (instead of calculating it again). These are only used in
  // updateSequenceNo().
  // The key is the prefix/sequenceNo. The value is the hash.
  std::map<Name, uint32_t> prefixToHash_;
  // The key is the hash. The value is the prefix/sequenceNo.
  std::map<uint32_t, Name> hashToPrefix_;

  Face& face_;
  KeyChain keyChain_;
  SigningInfo signingInfo_;

  Name syncPrefix_;

  Milliseconds syncReplyFreshnessPeriod_;
  Milliseconds helloReplyFreshnessPeriod_;

  ptr_lib::shared_ptr<PSyncSegmentPublisher> segmentPublisher_;
};

}

#endif
