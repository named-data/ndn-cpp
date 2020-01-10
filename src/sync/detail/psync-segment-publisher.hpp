/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/segment-publisher.hpp
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

#ifndef NDN_PSYNC_SEGMENT_PUBLISHER_HPP
#define NDN_PSYNC_SEGMENT_PUBLISHER_HPP

#include <ndn-cpp/face.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/in-memory-storage/in-memory-storage-retaining.hpp>

namespace ndn {

/**
 * The PSyncSegmentPublisher class has methods to publish segmented data used by
 * PSync.
 */
class PSyncSegmentPublisher {
public:
  /**
   * Create a PSyncSegmentPublisher.
   * @param face The application's Face.
   * @param keyChain The KeyChain for signing Data packets.
   * @param inMemoryStorageLimit (optional) The limit for the in-memory storage.
   * If omitted, use MAX_SEGMENTS_STORED.
   */
  PSyncSegmentPublisher
    (Face& face, KeyChain& keyChain, 
     size_t inMemoryStorageLimit = MAX_SEGMENTS_STORED)
  : face_(face), keyChain_(keyChain)
#if 0 // Until InMemoryStorageFifo implements an eviction policy, use InMemoryStorageRetaining.
    , storage_(inMemoryStorageLimit)
#endif
  {
  }

  /**
   * Put all the segments in the memory store.
   * @param interestName If the Interest name ends in a segment, immediately
   * send the Data packet for the segment to the Face.
   * @param dataName The Data name, which has components after the Interest name.
   * @param content The content of the data to be segmented.
   * @param freshnessPeriod The freshness period of the segments, in milliseconds.
   * @param signingInfo (optional) The SigningInfo for signing segment Data
   * packets. If omitted, use the default SigningInfo().
   */
  void
  publish
    (const Name& interestName, const Name& dataName, Blob content,
     Milliseconds freshnessPeriod, const SigningInfo& signingInfo = SigningInfo());

  /**
   * Try to reply to the Interest name from the memory store.
   * @param interestName The Interest name for looking up in the memory store.
   * @return True if sent the segment Data packet to the Face, or false if we
   * cannot find the segment, in which case the caller is expected to publish
   * the segment.
   */
  bool
  replyFromStore(const Name& interestName);

  static const size_t MAX_SEGMENTS_STORED = 100;

private:
  Face& face_;
  KeyChain& keyChain_;
#if 0 // Until InMemoryStorageFifo implements an eviction policy, use InMemoryStorageRetaining.
  InMemoryStorageFifo storage_;
#else
  InMemoryStorageRetaining storage_;
#endif
};

}

#endif
