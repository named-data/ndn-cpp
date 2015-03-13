/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx util/segment-fetcher https://github.com/named-data/ndn-cxx
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

#include "../c/util/ndn_memory.h"
#include <ndn-cpp/util/segment-fetcher.hpp>

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

bool
SegmentFetcher::DontVerifySegment(const ptr_lib::shared_ptr<Data>& data)
{
  return true;
}

void
SegmentFetcher::fetch
  (Face& face, const Interest &baseInterest, const VerifySegment& verifySegment,
   const OnComplete& onComplete, const OnError& onError)
{
  // Make a shared_ptr because we make callbacks with bind using
  //   shared_from_this() so the object remains allocated.
  ptr_lib::shared_ptr<SegmentFetcher> segmentFetcher
    (new SegmentFetcher(face, verifySegment, onComplete, onError));
  segmentFetcher->fetchFirstSegment(baseInterest);
}

void
SegmentFetcher::fetchFirstSegment(const Interest& baseInterest)
{
  Interest interest(baseInterest);
  interest.setChildSelector(1);
  interest.setMustBeFresh(true);

  face_.expressInterest
    (interest, 
     bind(&SegmentFetcher::onSegmentReceived, shared_from_this(), _1, _2),
     bind(&SegmentFetcher::onTimeout, shared_from_this(), _1));
}

void
SegmentFetcher::fetchNextSegment
  (const Interest& originalInterest, const Name& dataName, uint64_t segment)
{
  // Start with the original Interest to preserve any special selectors.
  Interest interest(originalInterest);
  // Changing a field clears the nonce so that the library will generate a new one.
  interest.setMustBeFresh(false);
  interest.setName(dataName.getPrefix(-1).appendSegment(segment));

  face_.expressInterest
    (interest, 
     bind(&SegmentFetcher::onSegmentReceived, shared_from_this(), _1, _2),
     bind(&SegmentFetcher::onTimeout, shared_from_this(), _1));
}

void
SegmentFetcher::onSegmentReceived
  (const ptr_lib::shared_ptr<const Interest>& originalInterest,
   const ptr_lib::shared_ptr<Data>& data)
{
  if (!verifySegment_(data)) {
    onError_(SEGMENT_VERIFICATION_FAILED, "Segment verification failed");
    return;
  }

  if (!endsWithSegmentNumber(data->getName()))
    // We don't expect a name without a segment number.  Treat it as a bad packet.
    onError_
      (DATA_HAS_NO_SEGMENT,
       string("Got an unexpected packet without a segment number: ") +
         data->getName().toUri());
  else {
    uint64_t currentSegment;
    try {
      currentSegment = data->getName().get(-1).toSegment();
    }
    catch (runtime_error& ex) {
      onError_
        (DATA_HAS_NO_SEGMENT,
         string("Error decoding the name segment number ") +
         data->getName().get(-1).toEscapedString() + ": " + ex.what());
      return;
    }

    uint64_t expectedSegmentNumber = contentParts_.size();
    if (currentSegment != expectedSegmentNumber)
      // Try again to get the expected segment.  This also includes the case
      //   where the first segment is not segment 0.
      fetchNextSegment(*originalInterest, data->getName(), expectedSegmentNumber);
    else {
      // Save the content and check if we are finished.
      contentParts_.push_back(data->getContent());

      if (data->getMetaInfo().getFinalBlockId().getValue().size() > 0) {
        uint64_t finalSegmentNumber;
        try {
          finalSegmentNumber = data->getMetaInfo().getFinalBlockId().toSegment();
        }
        catch (runtime_error& ex) {
          onError_
            (DATA_HAS_NO_SEGMENT,
             string("Error decoding the FinalBlockId segment number ") +
             data->getMetaInfo().getFinalBlockId().toEscapedString() + ": " + 
             ex.what());
          return;
        }

        if (currentSegment == finalSegmentNumber) {
          // We are finished.

          // Get the total size and concatenate to get the content.
          int totalSize = 0;
          for (int i = 0; i < contentParts_.size(); ++i)
            totalSize += contentParts_[i].size();
          ptr_lib::shared_ptr<vector<uint8_t> > content
            (new std::vector<uint8_t>(totalSize));
          size_t offset = 0;
          for (size_t i = 0; i < contentParts_.size(); ++i) {
            const Blob& part = contentParts_[i];
            ndn_memcpy(&(*content)[offset], part.buf(), part.size());
            offset += part.size();
          }

          onComplete_(Blob(content, false));
          return;
        }
      }

      // Fetch the next segment.
      fetchNextSegment
        (*originalInterest, data->getName(), expectedSegmentNumber + 1);
    }
  }
}

void
SegmentFetcher::onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
{
  onError_
    (INTEREST_TIMEOUT,
     string("Time out for interest ") + interest->getName().toUri());
}

}
