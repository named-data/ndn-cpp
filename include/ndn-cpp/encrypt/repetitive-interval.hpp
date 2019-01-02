/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/repetitive-interval https://github.com/named-data/ndn-group-encrypt
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

#ifndef NDN_REPETITIVE_INTERVAL_HPP
#define NDN_REPETITIVE_INTERVAL_HPP

#include "interval.hpp"

namespace ndn {

/**
 * A RepetitiveInterval is an advanced interval which can repeat and can be used
 * to find a simple Interval that a time point falls in.
 * @note This class is an experimental feature. The API may change.
 */
class RepetitiveInterval {
public:
  enum RepeatUnit {
    NONE = 0,
    DAY = 1,
    MONTH = 2,
    YEAR = 3
  };

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
   * Create a default RepetitiveInterval with one day duration, non-repeating.
   */
  RepetitiveInterval();

  /**
   * Create a RepetitiveInterval with the given values. startDate must be
   * earlier than or same as endDate. intervalStartHour must be less than
   * intervalEndHour.
   * @param startDate The start date as milliseconds since Jan 1, 1970 UTC.
   * @param endDate The end date as milliseconds since Jan 1, 1970 UTC.
   * @param intervalStartHour The start hour in the day, from 0 to 23.
   * @param intervalEndHour The end hour in the day from 1 to 24.
   * @param nRepeats (optional) Repeat the interval nRepeats repetitions, every
   * unit, until endDate. If ommitted, use 0.
   * @param repeatUnit (optional) The unit of the repetition. If this is NONE or
   * omitted, then startDate must equal endDate. If omitted, use NONE.
   * @throws runtime_error if the above conditions are not met.
   */
  RepetitiveInterval
    (MillisecondsSince1970 startDate, MillisecondsSince1970 endDate,
     int intervalStartHour, int intervalEndHour, int nRepeats = 0,
     RepeatUnit repeatUnit = RepeatUnit::NONE);

  /**
   * Get an interval that covers the time point. If there is no interval
   * covering the time point, this returns false for isPositive and returns a
   * negative interval.
   * @param timePoint The time point as milliseconds since Jan 1, 1970 UTC.
   * @return An object with fields (isPositive, interval) where isPositive is
   * true if the returned interval is positive or false if negative, and
   * interval is the Interval covering the time point or a negative interval if
   * not found.
   */
  Result
  getInterval(MillisecondsSince1970 timePoint) const;

  /**
   * Compare this to the other RepetitiveInterval.
   * @param other The other RepetitiveInterval to compare to.
   * @return -1 if this is less than the other, 1 if greater and 0 if equal.
   */
  int
  compare(const RepetitiveInterval& other) const;

  /**
   * Get the start date.
   * @return The start date as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getStartDate() const { return startDate_; }

  /**
   * Get the end date.
   * @return The end date as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getEndDate() const { return endDate_; }

  /**
   * Get the interval start hour.
   * @return The interval start hour.
   */
  int
  getIntervalStartHour() const { return intervalStartHour_; }

  /**
   * Get the interval end hour.
   * @return The interval end hour.
   */
  int
  getIntervalEndHour() const { return intervalEndHour_; }

  /**
   * Get the number of repeats.
   * @return The number of repeats.
   */
  int
  getNRepeats() const { return nRepeats_; }

  /**
   * Get the repeat unit.
   * @return The repeat unit.
   */
  RepeatUnit
  getRepeatUnit() const { return repeatUnit_; }

private:
  friend class Schedule;

  /**
   * Check if the date of the time point is in any interval.
   * @param timePoint The time point as milliseconds since Jan 1, 1970 UTC.
   * @return True if the date of the time point is in any interval.
   */
  bool
  hasIntervalOnDate(MillisecondsSince1970 timePoint) const;

  /**
   * Return a time point on the beginning of the date (without hours, minutes, etc.)
   * @param timePoint The time point as milliseconds since Jan 1, 1970 UTC.
   * @return A time point as milliseconds since Jan 1, 1970 UTC.
   */
  static MillisecondsSince1970
  toDateOnlyMilliseconds(MillisecondsSince1970 timePoint);

  static const uint64_t MILLISECONDS_IN_HOUR = 3600 * 1000;
  static const uint64_t MILLISECONDS_IN_DAY = 24 * 3600 * 1000;
  MillisecondsSince1970 startDate_;
  MillisecondsSince1970 endDate_;
  int intervalStartHour_;
  int intervalEndHour_;
  int nRepeats_;
  RepeatUnit repeatUnit_;
};

}

#endif
