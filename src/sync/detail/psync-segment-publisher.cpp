/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From the PSync library https://github.com/named-data/PSync/blob/master/PSync/segment-publisher.cpp
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

#include "psync-segment-publisher.hpp"

using namespace std;

namespace ndn {

void
PSyncSegmentPublisher::publish
  (const Name& interestName, const Name& dataName, Blob content,
   Milliseconds freshnessPeriod, const SigningInfo& signingInfo)
{
  uint64_t interestSegment = 0;
  if (interestName[-1].isSegment())
    interestSegment = interestName[-1].toSegment();

  const uint8_t* rawBuffer = content.buf();
  const uint8_t* segmentBegin = rawBuffer;
  const uint8_t* end = rawBuffer + content.size();

  size_t maxPacketSize = MAX_NDN_PACKET_SIZE / 2;

  uint64_t totalSegments = content.size() / maxPacketSize;
  Name::Component finalBlockId = Name::Component::fromSegment(totalSegments);

  Name segmentPrefix(dataName);
  segmentPrefix.appendVersion((uint64_t)ndn_getNowMilliseconds());

  uint64_t segmentNo = 0;
  do {
    const uint8_t* segmentEnd = segmentBegin + maxPacketSize;
    if (segmentEnd > end)
      segmentEnd = end;

    Name segmentName(segmentPrefix);
    segmentName.appendSegment(segmentNo);

    ptr_lib::shared_ptr<Data> data = ptr_lib::make_shared<Data>(segmentName);
    data->setContent(Blob(segmentBegin, segmentEnd - segmentBegin));
    data->getMetaInfo().setFreshnessPeriod(freshnessPeriod);
    data->getMetaInfo().setFinalBlockId(finalBlockId);

    segmentBegin = segmentEnd;

    keyChain_.sign(*data, signingInfo);

    // Only send the segment to the Face if it has a pending interest.
    // Otherwise, the segment is unsolicited.
    if (interestSegment == segmentNo)
      face_.putData(*data);

#if 0 // Until InMemoryStorageFifo implements an eviction policy, use InMemoryStorageRetaining.
    storage_.insert(*data, freshnessPeriod);
#else
    storage_.insert(*data);
#endif

    face_.callLater
      (freshnessPeriod,
       bind(&InMemoryStorageRetaining::remove, &storage_, segmentName));

    ++segmentNo;
  } while (segmentBegin < end);
}

bool
PSyncSegmentPublisher::replyFromStore(const Name& interestName)
{
  ptr_lib::shared_ptr<Data> data = storage_.find(interestName);

  if (data) {
    face_.putData(*data);
    return true;
  }

  return false;
}

}
