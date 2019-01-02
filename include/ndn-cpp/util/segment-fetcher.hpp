/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015-2019 Regents of the University of California.
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

#ifndef NDN_SEGMENT_FETCHER_HPP
#define NDN_SEGMENT_FETCHER_HPP

#include "../face.hpp"
#include "../security/key-chain.hpp"

namespace ndn {

/**
 * SegmentFetcher is a utility class to the fetch latest version of segmented data.
 *
 * SegmentFetcher assumes that the data is named /<prefix>/<version>/<segment>,
 * where:
 * - <prefix> is the specified name prefix,
 * - <version> is an unknown version that needs to be discovered, and
 * - <segment> is a segment number. (The number of segments is unknown and is
 *   controlled by the `FinalBlockId` field in at least the last Data packet.
 *
 * The following logic is implemented in SegmentFetcher:
 *
 * 1. Express the first Interest to discover the version:
 *
 *    >> Interest: /<prefix>?ChildSelector=1&MustBeFresh=true
 *
 * 2. Infer the latest version of the Data: <version> = Data.getName().get(-2)
 *
 * 3. If the segment number in the retrieved packet == 0, go to step 5.
 *
 * 4. Send an Interest for segment 0:
 *
 *    >> Interest: /<prefix>/<version>/<segment=0>
 *
 * 5. Keep sending Interests for the next segment while the retrieved Data does
 *    not have a FinalBlockId or the FinalBlockId != Data.getName().get(-1).
 *
 *    >> Interest: /<prefix>/<version>/<segment=(N+1))>
 *
 * 6. Call the OnComplete callback with a blob that concatenates the content
 *    from all the segmented objects.
 *
 * If an error occurs during the fetching process, the OnError callback is called
 * with a proper error code.  The following errors are possible:
 *
 * - `INTEREST_TIMEOUT`: if any of the Interests times out
 * - `DATA_HAS_NO_SEGMENT`: if any of the retrieved Data packets don't have a segment
 *   as the last component of the name (not counting the implicit digest)
 * - `SEGMENT_VERIFICATION_FAILED`: if any retrieved segment fails
 *   the user-provided VerifySegment callback or KeyChain verifyData.
 *
 * In order to validate individual segments, a KeyChain needs to be supplied.
 * If verifyData fails, the fetching process is aborted with
 * SEGMENT_VERIFICATION_FAILED. If data validation is not required, pass a null
 * KeyChain.
 *
 * Example:
 *     void onComplete(const Blob& encodedMessage);
 *
 *     void onError(SegmentFetcher::ErrorCode errorCode, const string& message);
 *
 *     Interest interest(Name("/data/prefix"));
 *     interest.setInterestLifetimeMilliseconds(1000);
 *
 *     SegmentFetcher.fetch(face, interest, 0, onComplete, onError);
 */
class SegmentFetcher : public ptr_lib::enable_shared_from_this<SegmentFetcher> {
public:
  enum ErrorCode {
    INTEREST_TIMEOUT = 1,
    DATA_HAS_NO_SEGMENT = 2,
    SEGMENT_VERIFICATION_FAILED = 3
  };

  typedef func_lib::function<bool(const ptr_lib::shared_ptr<Data>& data)> VerifySegment;

  typedef func_lib::function<void(const Blob& content)> OnComplete;

  typedef func_lib::function<void
    (ErrorCode errorCode, const std::string& message)> OnError;

  /**
   * DontVerifySegment may be used in fetch to skip validation of Data packets.
   */
  static bool
  DontVerifySegment(const ptr_lib::shared_ptr<Data>& data);

  /**
   * Initiate segment fetching. For more details, see the documentation for
   * the class.
   * @param face This calls face.expressInterest to fetch more segments.
   * @param baseInterest An Interest for the initial segment of the requested
   * data, where baseInterest.getName() has the name prefix.
   * This interest may include a custom InterestLifetime and selectors that will
   * propagate to all subsequent Interests. The only exception is that the
   * initial Interest will be forced to include selectors "ChildSelector=1" and
   * "MustBeFresh=true" which will be turned off in subsequent Interests.
   * @param verifySegment When a Data packet is received this calls
   * verifySegment(data). If it returns false then abort fetching and call
   * onError with SEGMENT_VERIFICATION_FAILED. If data validation is not
   * required, use DontVerifySegment.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onComplete When all segments are received, call
   * onComplete(content) where content is the concatenation of the content of
   * all the segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError Call onError(errorCode, message) for timeout or an error
   * processing segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  static void
  fetch
    (Face& face, const Interest &baseInterest, const VerifySegment& verifySegment,
     const OnComplete& onComplete, const OnError& onError);

  /**
   * Initiate segment fetching. For more details, see the documentation for
   * the class.
   * @param face This calls face.expressInterest to fetch more segments.
   * @param baseInterest An Interest for the initial segment of the requested
   * data, where baseInterest.getName() has the name prefix.
   * This interest may include a custom InterestLifetime and selectors that will
   * propagate to all subsequent Interests. The only exception is that the
   * initial Interest will be forced to include selectors "ChildSelector=1" and
   * "MustBeFresh=true" which will be turned off in subsequent Interests.
   * @param validatorKeyChain When a Data packet is received this calls
   * validatorKeyChain->verifyData(data). If validation fails then abort
   * fetching and call onError with SEGMENT_VERIFICATION_FAILED. This does not
   * make a copy of the KeyChain; the object must remain valid while fetching.
   * If validatorKeyChain is null, this does not validate the data packet.
   * @param onComplete When all segments are received, call
   * onComplete(content) where content is the concatenation of the content of
   * all the segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onError Call onError(errorCode, message) for timeout or an error
   * processing segments.
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   */
  static void
  fetch
    (Face& face, const Interest &baseInterest, KeyChain* validatorKeyChain,
     const OnComplete& onComplete, const OnError& onError);

private:
  /**
   * Create a new SegmentFetcher to use the Face. See the static fetch method
   * for details. If validatorKeyChain is not null, use it and ignore
   * verifySegment. After creating the SegmentFetcher, call fetchFirstSegment.
   */
  SegmentFetcher
    (Face& face, KeyChain* validatorKeyChain, const VerifySegment& verifySegment,
     const OnComplete& onComplete, const OnError& onError)
  : face_(face), validatorKeyChain_(validatorKeyChain), verifySegment_(verifySegment),
    onComplete_(onComplete), onError_(onError)
  {
  }

  void
  fetchFirstSegment(const Interest& baseInterest);

  void
  fetchNextSegment
    (const Interest& originalInterest, const Name& dataName, uint64_t segment);

  void
  onSegmentReceived
    (const ptr_lib::shared_ptr<const Interest>& originalInterest,
     const ptr_lib::shared_ptr<Data>& data);

  void
  onVerified
    (const ptr_lib::shared_ptr<Data>& data,
     const ptr_lib::shared_ptr<const Interest>& originalInterest);

  void
  onValidationFailed
    (const ptr_lib::shared_ptr<Data>& data, const std::string& reason);

  void
  onTimeout(const ptr_lib::shared_ptr<const Interest>& interest);

  /**
   * Check if the last component in the name is a segment number.
   * @param name The name to check.
   * @return True if the name ends with a segment number, otherwise false.
   */
  static bool
  endsWithSegmentNumber(const Name& name)
  {
    return name.size() >= 1 && name.get(-1).isSegment();
  }

  std::vector<Blob> contentParts_;
  Face& face_;
  KeyChain* validatorKeyChain_;
  VerifySegment verifySegment_;
  OnComplete onComplete_;
  OnError onError_;
};

}

#endif
