/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
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

#ifndef NDN_GENERALIZED_CONTENT_HPP
#define NDN_GENERALIZED_CONTENT_HPP

#include <ndn-cpp/face.hpp>
#include <ndn-cpp/util/segment-fetcher.hpp>
#include <ndn-cpp/util/memory-content-cache.hpp>
#include "content-meta-info.hpp"

namespace ndntools {

/**
 * GeneralizedContent has the static publish and fetch methods which fetches meta
 * info and segmented content. See the methods for more detail.
 * @note The support for GeneralizedContent is experimental and the API is not finalized.
 */
class GeneralizedContent : public ndn::ptr_lib::enable_shared_from_this<GeneralizedContent> {
public:
  enum ErrorCode {
    // Repeat the error codes from SegmentFetcher.
    INTEREST_TIMEOUT = 1,
    DATA_HAS_NO_SEGMENT = 2,
    SEGMENT_VERIFICATION_FAILED = 3,

    META_INFO_DECODING_FAILED = 4
  };

  typedef ndn::func_lib::function<void
    (const ndn::ptr_lib::shared_ptr<ContentMetaInfo>& metaInfo,
     const ndn::Blob& content)> OnComplete;

  typedef ndn::func_lib::function<void
    (ErrorCode errorCode, const std::string& message)> OnError;

  /**
   * Use the contentCache to publish a Data packet named [prefix]/_meta whose
   * content is the encoded metaInfo. If metaInfo.getHasSegments() is true then
   * use the contentCache to publish the segments of the content.
   * @param contentCache This calls contentCache.add to add the Data packets.
   * After this call, the MemoryContentCache must remain valid long enough to
   * respond to Interest for the published Data packets.
   * @param prefix The Name prefix for the published Data packets.
   * @param freshnessPeriod The freshness period in milliseconds for the packets.
   * @param signingKeyChain This calls signingKeyChain.sign to sign the packets.
   * @param signingCertificateName The certificate name of the key used in
   * signingKeyChain.sign .
   * @param metaInfo The ContentMetaInfo for the _meta packet.
   * @param content The content which is segmented and published. If
   * metaInfo.getHasSegments() is false then this is ignored.
   * @param contentSegmentSize The the number of bytes for each segment of the
   * content. (This is is the size of the content in the segment Data packet,
   * not the size of the entire segment Data packet with overhead.) The final
   * segment may be smaller than this. If metaInfo.getHasSegments() is false
   * then this is ignored.
   */
  static void
  publish
    (ndn::MemoryContentCache& contentCache, const ndn::Name& prefix,
     ndn::Milliseconds freshnessPeriod, ndn::KeyChain* signingKeyChain,
     const ndn::Name& signingCertificateName, const ContentMetaInfo& metaInfo,
     const ndn::Blob& content, size_t contentSegmentSize);

  /**
   * Initiate meta info and segmented content fetching. This first fetches and
   * decodes <prefix>/_meta . If the ContentSize in the _meta info is not zero
   * then also fetch segments such as <prefix>/%00 .
   * @param face This calls face.expressInterest to fetch the _meta info and
   * segments.
   * @param prefix The prefix of the Data packets before the _meta or segment
   * number components.
   * @param validatorKeyChain When a Data packet is received this calls
   * validatorKeyChain->verifyData(data). If validation fails then abort
   * fetching and call onError with SEGMENT_VERIFICATION_FAILED. This does not
   * make a copy of the KeyChain; the object must remain valid while fetching.
   * If validatorKeyChain is null, this does not validate the data packet.
   * @param onComplete When all segments are received, call
   * onComplete(metaInfo, content) where metaInfo is the decoded ContentMetaInfo
   * object and content is the concatenation of the content of all the segments.
   * However, if the metaInfo content size is zero, then this does not fetch
   * segments and the content is null.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError Call onError(errorCode, message) for timeout or an error
   * processing segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param interestLifetimeMilliseconds (optional) The Interest lifetime in
   * milliseconds for fetching the _meta info and segments. If omitted, use the
   * default value from the default Interest object.
   */
  static void
  fetch
    (ndn::Face& face, const ndn::Name& prefix, ndn::KeyChain* validatorKeyChain,
     const OnComplete& onComplete, const OnError& onError,
     ndn::Milliseconds interestLifetimeMilliseconds = 4000.0);

private:
  /**
   * Create a new GeneralizedContent to use the Face. See the static fetch method
   * for details. After creating the SegmentFetcher, call fetchMetaInfo.
   */
  GeneralizedContent
    (ndn::Face& face, const ndn::Name& prefix, ndn::KeyChain* validatorKeyChain,
     const OnComplete& onComplete, const OnError& onError,
     ndn::Milliseconds interestLifetimeMilliseconds)
  : face_(face), prefix_(prefix), validatorKeyChain_(validatorKeyChain),
    onComplete_(onComplete), onError_(onError),
    interestLifetimeMilliseconds_(interestLifetimeMilliseconds)
  {
  }

  void
  fetchMetaInfo();

  void
  onMetaInfoReceived
    (const ndn::ptr_lib::shared_ptr<const ndn::Interest>& originalInterest,
     const ndn::ptr_lib::shared_ptr<ndn::Data>& data);

  void
  onMetaInfoTimeout(const ndn::ptr_lib::shared_ptr<const ndn::Interest>& interest);

  void
  onContentReceived(const ndn::Blob& content);

  void
  onSegmentFetcherError
    (ndn::SegmentFetcher::ErrorCode errorCode, const std::string& message);

  ndn::Face& face_;
  ndn::Name prefix_;
  ndn::KeyChain* validatorKeyChain_;
  OnComplete onComplete_;
  OnError onError_;
  ndn::Milliseconds interestLifetimeMilliseconds_;
  ndn::ptr_lib::shared_ptr<ContentMetaInfo> metaInfo_;
};

}

#endif
