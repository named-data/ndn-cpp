/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include "../../src/util/logging.hpp"
#include "content-fetcher.hpp"

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.ContentFetcher");

namespace ndn {

void
ContentFetcher::publish
  (MemoryContentCache& contentCache, const Name& prefix,
   Milliseconds freshnessPeriod, KeyChain* signingKeyChain,
   const Name& signingCertificateName, const ContentMetaInfo& metaInfo,
   const Blob& content, size_t contentSegmentSize)
{
  // Add the _meta Data packet to the contentCache.
  Data data(prefix);
  data.getName().append("_meta");
  data.getMetaInfo().setFreshnessPeriod(freshnessPeriod);
  data.setContent(metaInfo.wireEncode());
  if (signingKeyChain)
    signingKeyChain->sign(data);
  contentCache.add(data);

  if (metaInfo.getContentSize() > 0 && content.size() > 0) {
    // Add the segments of the content.
    // TODO: Implement the signature _manifest.
    int finalSegmentNumber = 0;
    for (size_t offset = 0; offset < content.size(); offset += contentSegmentSize)
      ++finalSegmentNumber;
    --finalSegmentNumber;
    Name::Component finalBlockId(Name().appendSegment(finalSegmentNumber).get(0));

    int segmentNumber = 0;
    for (size_t offset = 0; offset < content.size(); offset += contentSegmentSize) {
      size_t length = content.size() - offset;
      if (length > contentSegmentSize)
        length = contentSegmentSize;
      Blob segmentContent(content.buf() + offset, length);

      Data data(prefix);
      data.getName().appendSegment(segmentNumber);
      data.getMetaInfo().setFreshnessPeriod(freshnessPeriod);
      data.getMetaInfo().setFinalBlockId(finalBlockId);
      data.setContent(segmentContent);
      contentCache.add(data);

      ++segmentNumber;
    }
  }
}

void
ContentFetcher::fetch
  (Face& face, const Name& prefix, KeyChain* validatorKeyChain,
   const OnComplete& onComplete, const OnError& onError,
   Milliseconds interestLifetimeMilliseconds)
{
  // Make a shared_ptr because we make callbacks with bind using
  //   shared_from_this() so the object remains allocated.
  ptr_lib::shared_ptr<ContentFetcher> contentFetcher
    (new ContentFetcher
     (face, prefix, validatorKeyChain, onComplete, onError,
      interestLifetimeMilliseconds));
  contentFetcher->fetchMetaInfo();
}

void
ContentFetcher::fetchMetaInfo()
{
  Interest interest(prefix_);
  interest.getName().append("_meta");
  interest.setInterestLifetimeMilliseconds(interestLifetimeMilliseconds_);
  interest.setMustBeFresh(true);

  face_.expressInterest
    (interest,
     bind(&ContentFetcher::onMetaInfoReceived, shared_from_this(), _1, _2),
     bind(&ContentFetcher::onMetaInfoTimeout, shared_from_this(), _1));
}

void
ContentFetcher::onMetaInfoReceived
  (const ptr_lib::shared_ptr<const Interest>& originalInterest,
   const ptr_lib::shared_ptr<Data>& data)
{
  // Decode the _meta info.
  metaInfo_.reset(new ContentMetaInfo());
  try {
    metaInfo_->wireDecode(data->getContent());
  } catch (const std::exception& decodeException) {
    try {
      onError_
        (ErrorCode::META_INFO_DECODING_FAILED,
         string("Error decoding the _meta info: ") + decodeException.what());
    } catch (const std::exception& ex) {
      _LOG_ERROR("ContentFetcher::onMetaInfoReceived: Error in onError: " << ex.what());
    } catch (...) {
      _LOG_ERROR("ContentFetcher::onMetaInfoReceived: Error in onError.");
    }

    return;
  }

  if (metaInfo_->getContentSize() == 0) {
    // We're done. Report the _meta info.
    try {
      onComplete_(metaInfo_, Blob());
    } catch (const std::exception& ex) {
      _LOG_ERROR("ContentFetcher::onTimeout: Error in onComplete: " << ex.what());
    } catch (...) {
      _LOG_ERROR("ContentFetcher::onTimeout: Error in onComplete.");
    }
  }
  else {
    // Fetch the segments.
    Interest baseInterest(prefix_);
    baseInterest.getName().appendSegment(0);
    baseInterest.setInterestLifetimeMilliseconds(interestLifetimeMilliseconds_);
    SegmentFetcher::fetch
      (face_, baseInterest, validatorKeyChain_,
       bind(&ContentFetcher::onContentReceived, shared_from_this(), _1),
       bind(&ContentFetcher::onSegmentFetcherError, shared_from_this(), _1, _2));
  }
}

void
ContentFetcher::onMetaInfoTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest)
{
  try {
    onError_
      (ErrorCode::INTEREST_TIMEOUT,
       string("Time out fetching _meta info ") + interest->getName().toUri());
  } catch (const std::exception& ex) {
    _LOG_ERROR("ContentFetcher::onTimeout: Error in onError: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ContentFetcher::onTimeout: Error in onError.");
  }
}

void
ContentFetcher::onContentReceived(const Blob& content)
{
  try {
    onComplete_(metaInfo_, content);
  } catch (const std::exception& ex) {
    _LOG_ERROR("ContentFetcher::onTimeout: Error in onComplete: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ContentFetcher::onTimeout: Error in onComplete.");
  }
}

void
ContentFetcher::onSegmentFetcherError
  (SegmentFetcher::ErrorCode errorCode, const std::string& message)
{
  // Convert the SegmentFetcher error code to the ContentFetcher error code.
  ErrorCode contentFetcherErrorCode = (ErrorCode)-1;
  if (errorCode == SegmentFetcher::ErrorCode::INTEREST_TIMEOUT)
    contentFetcherErrorCode = ErrorCode::INTEREST_TIMEOUT;
  else if (errorCode == SegmentFetcher::ErrorCode::DATA_HAS_NO_SEGMENT)
    contentFetcherErrorCode = ErrorCode::DATA_HAS_NO_SEGMENT;
  else if (errorCode == SegmentFetcher::ErrorCode::SEGMENT_VERIFICATION_FAILED)
    contentFetcherErrorCode = ErrorCode::SEGMENT_VERIFICATION_FAILED;

  try {
    onError_(contentFetcherErrorCode, message);
  } catch (const std::exception& ex) {
    _LOG_ERROR("ContentFetcher::onSegmentFetcherError: Error in onError: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ContentFetcher::onSegmentFetcherError: Error in onError.");
  }
}

}
