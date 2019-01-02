/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/interval https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_INTERVAL_HPP
#define NDN_INTERVAL_HPP

#include <exception>
#include "../common.hpp"

namespace ndn {

/**
 * An Interval defines a time duration which contains a start timestamp and an
 * end timestamp.
 * @note This class is an experimental feature. The API may change.
 */
class Interval {
public:
  /**
   * Interval.Error extends std::exception for errors using Interval methods.
   */
  class Error : public std::exception {
  public:
    Error(const std::string& errorMessage) throw();

    virtual ~Error() throw();

    std::string
    Msg() const { return errorMessage_; }

    virtual const char*
    what() const throw();

  private:
    const std::string errorMessage_;
  };

  /**
   * Create an Interval with the given validity value.
   * @param isValid (optional) True to create a valid empty interval, false to
   * create an invalid interval. If omitted, create an invalid interval.
   */
  Interval(bool isValid = false);


  /**
   * Create a valid Interval with the given start and end times. The start
   * time must be less than the end time. To create an empty interval (start
   * time equals end time), use the constructor Interval(true).
   * @param startTime The start time as milliseconds since Jan 1, 1970 UTC.
   * @param endTime The end time as milliseconds since Jan 1, 1970 UTC.
   */
  Interval(MillisecondsSince1970 startTime, MillisecondsSince1970 endTime);

  /**
   * Set this interval to have the same values as the other interval.
   * @param interval The other Interval with values to copy.
   */
  void
  set(const Interval& interval)
  {
    startTime_ = interval.startTime_;
    endTime_ = interval.endTime_;
    isValid_ = interval.isValid_;
  }

  /**
   * Check if the time point is in this interval.
   * @param timePoint The time point to check as milliseconds since Jan 1, 1970 UTC.
   * @return True if timePoint is in this interval.
   */
  bool
  covers(MillisecondsSince1970 timePoint) const;

  /**
   * Set this Interval to the intersection of this and the other interval.
   * This and the other interval should be valid but either can be empty.
   * @param interval The other Interval to intersect with.
   * @return This Interval.
   */
  Interval&
  intersectWith(const Interval& interval);

  /**
   * Set this Interval to the union of this and the other interval.
   * This and the other interval should be valid but either can be empty.
   * This and the other interval should have an intersection. (Contiguous
   * intervals are not allowed.)
   * @param interval The other Interval to union with.
   * @return This Interval.
   * @throws Interval::Error if the two intervals do not have an intersection.
   */
  Interval&
  unionWith(const Interval& interval);

  /**
   * Get the start time.
   * @return The start time as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getStartTime() const;

  /**
   * Get the end time.
   * @return The end time as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getEndTime() const;

  /**
   * Check if this Interval is valid.
   * @return True if this interval is valid, false if invalid.
   */
  bool
  isValid() const { return isValid_; }

  /**
   * Check if this Interval is empty.
   * @return True if this Interval is empty (start time equals end time), false
   * if not.
   */
  bool
  isEmpty() const;

private:
  MillisecondsSince1970 startTime_;
  MillisecondsSince1970 endTime_;
  bool isValid_;
};

}

#endif
