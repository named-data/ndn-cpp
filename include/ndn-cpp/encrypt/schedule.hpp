/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/schedule https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_SCHEDULE_HPP
#define NDN_SCHEDULE_HPP

#include <vector>
#include <ndn-cpp/c/errors.h>
#include <ndn-cpp/util/blob.hpp>
#include "repetitive-interval.hpp"

struct ndn_TlvEncoder;
struct ndn_TlvDecoder;

namespace ndn {

/**
 * Schedule is used to manage the times when a member can access data using two
 * sets of RepetitiveInterval as follows. whiteIntervalList is an ordered
 * set for the times a member is allowed to access to data, and
 * blackIntervalList is for the times a member is not allowed.
 * @note This class is an experimental feature. The API may change.
 */
class Schedule {
public:
  class Result {
  public:
    Result(bool isPositive, const Interval& interval)
    {
      this->isPositive = isPositive;
      this->interval = interval;
    }

    bool isPositive;
    Interval interval;
  };

  /**
   * Create a Schedule with empty whiteIntervalList and blackIntervalList.
   */
  Schedule()
  {
  }

  // Note: RepetitiveInterval is immutable, so we can use the default shallow
  //   copy constructor.

  /**
   * Add the repetitiveInterval to the whiteIntervalList.
   * @param repetitiveInterval The RepetitiveInterval to add. If the list
   * already contains the same RepetitiveInterval, this does nothing.
   * @return This Schedule so you can chain calls to add.
   */
  Schedule&
  addWhiteInterval(const ptr_lib::shared_ptr<RepetitiveInterval>& repetitiveInterval)
  {
    // RepetitiveInterval is immutable, so we don't need to make a copy.
    sortedSetAdd(whiteIntervalList_, repetitiveInterval);
    return *this;
  }

  /**
   * Add the repetitiveInterval to the blackIntervalList.
   * @param repetitiveInterval The RepetitiveInterval to add. If the list
   * already contains the same RepetitiveInterval, this does nothing.
   * @return This Schedule so you can chain calls to add.
   */
  Schedule&
  addBlackInterval(const ptr_lib::shared_ptr<RepetitiveInterval>& repetitiveInterval)
  {
    // RepetitiveInterval is immutable, so we don't need to make a copy.
    sortedSetAdd(blackIntervalList_, repetitiveInterval);
    return *this;
  }

  /**
   * Get the interval that covers the time stamp. This iterates over the two
   * repetitive interval sets and find the shortest interval that allows a group
   * member to access the data. If there is no interval covering the time stamp,
   * this returns false for isPositive and returns a negative interval.
   * @param timeStamp The time stamp as milliseconds since Jan 1, 1970 UTC.
   * @return An object with fields (isPositive, interval) where isPositive is
   * true if the returned interval is positive or false if negative, and
   * interval is the Interval covering the time stamp, or a negative interval if
   * not found.
   */
  Result
  getCoveringInterval(MillisecondsSince1970 timeStamp) const;

  /**
   * Encode this Schedule.
   * @return The encoded byte array.
   */
  Blob
  wireEncode() const;

  /**
   * Decode the input and update this Schedule.
   * @param input The input byte array to be decoded.
   * @param inputLength The length of input.
   */
  void
  wireDecode(const uint8_t *input, size_t inputLength);

  /**
   * Decode the input and update this Schedule.
   * @param input The input byte array to be decoded.
   */
  void
  wireDecode(const std::vector<uint8_t>& input)
  {
    wireDecode(&input[0], input.size());
  }

  /**
   * Decode the input and update this Schedule.
   * @param input The input byte array to be decoded as an immutable Blob.
   */
  void
  wireDecode(const Blob& input)
  {
    wireDecode(input.buf(), input.size());
  }

  static MillisecondsSince1970
  fromIsoString(const std::string& dateString);

  static std::string
  toIsoString(MillisecondsSince1970 msSince1970);

private:
  /**
   * Insert element into the list, sorted using element.compare(). If it is a
   * duplicate of an existing list element, don't add it.
   * @param list The list to update.
   * @param element The element to add.
   */
  static void
  sortedSetAdd
    (std::vector<ptr_lib::shared_ptr<RepetitiveInterval> >& list,
     const ptr_lib::shared_ptr<RepetitiveInterval>& element);

  /**
   * This is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs in the body of
   * the RepetitiveInterval value.
   * @param context This is the RepetitiveInterval pointer which was passed to
   * writeTlv.
   * @param encoder the ndn_TlvEncoder which is calling this.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  encodeRepetitiveIntervalValue(const void *context, ndn_TlvEncoder *encoder);

  /**
   * This is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs in the body of
   * the RepetitiveInterval list value, such as WhiteIntervalList.
   * @param context This is the vector<ptr_lib::shared_ptr<RepetitiveInterval> >
   * pointer which was passed to writeTlv.
   * @param encoder the ndn_TlvEncoder which is calling this.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  encodeRepetitiveIntervalListValue(const void *context, ndn_TlvEncoder *encoder);

  /**
   * This is called by ndn_TlvEncoder_writeNestedTlv to write the TLVs in the body of
   * the Schedule value.
   * @param context This is the Schedule pointer which was passed to writeTlv.
   * @param encoder the ndn_TlvEncoder which is calling this.
   * @return 0 for success, else an error code.
   */
  static ndn_Error
  encodeScheduleValue(const void *context, ndn_TlvEncoder *encoder);

  /**
   * Decode the input as an NDN-TLV RepetitiveInterval.
   * @param decoder A pointer to the ndn_TlvDecoder struct.
   * @return A new RepetitiveInterval with the decoded result.
   * @throws runtime_error for decoding error.
   */
  static ptr_lib::shared_ptr<RepetitiveInterval>
  decodeRepetitiveInterval(ndn_TlvDecoder *decoder);

  /**
   * A helper function to calculate black interval results or white interval
   * results.
   * @param list The set of RepetitiveInterval, which can be the white list or
   * the black list.
   * @param timeStamp The time stamp as milliseconds since Jan 1, 1970 UTC.
   * @param positiveResult The positive result which is updated.
   * @param negativeResult The negative result which is updated.
   */
  static void
  calculateIntervalResult
    (const std::vector<ptr_lib::shared_ptr<RepetitiveInterval> >& list,
     MillisecondsSince1970 timeStamp, Interval& positiveResult,
     Interval& negativeResult);

  std::vector<ptr_lib::shared_ptr<RepetitiveInterval> > whiteIntervalList_;
  std::vector<ptr_lib::shared_ptr<RepetitiveInterval> > blackIntervalList_;
  static const uint64_t MILLISECONDS_IN_DAY = 24 * 3600 * 1000;
};

}

#endif
