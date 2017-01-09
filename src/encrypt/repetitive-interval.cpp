/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_TIME_H
#include <time.h>
#endif
#if NDN_CPP_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <float.h>
#include <math.h>
#include <stdexcept>
#include <ndn-cpp/encrypt/repetitive-interval.hpp>

using namespace std;

namespace ndn {

RepetitiveInterval::RepetitiveInterval()
{
  startDate_ = -DBL_MAX;
  endDate_ = -DBL_MAX;
  intervalStartHour_ = 0;
  intervalEndHour_ = 24;
  nRepeats_ = 0;
  repeatUnit_ = RepeatUnit::NONE;
}

RepetitiveInterval::RepetitiveInterval
  (MillisecondsSince1970 startDate, MillisecondsSince1970 endDate,
   int intervalStartHour, int intervalEndHour, int nRepeats,
   RepeatUnit repeatUnit)
{
  startDate_ = toDateOnlyMilliseconds(startDate);
  endDate_ = toDateOnlyMilliseconds(endDate);
  intervalStartHour_ = intervalStartHour;
  intervalEndHour_ = intervalEndHour;
  nRepeats_ = nRepeats;
  repeatUnit_ = repeatUnit;

  if (!(intervalStartHour_ < intervalEndHour_))
    throw runtime_error("ReptitiveInterval: startHour must be less than endHour");
  if (!(startDate_ <= endDate_))
    throw runtime_error
      ("ReptitiveInterval: startDate must be earlier than or same as endDate");
  if (!(intervalStartHour_ >= 0))
    throw runtime_error("ReptitiveInterval: intervalStartHour must be non-negative");
  if (!(intervalEndHour_ >= 1 && intervalEndHour_ <= 24))
    throw runtime_error("ReptitiveInterval: intervalEndHour must be from 1 to 24");
  if (repeatUnit_ == RepeatUnit::NONE) {
    if (!(startDate_ == endDate_))
      throw runtime_error
        ("ReptitiveInterval: With RepeatUnit.NONE, startDate must equal endDate");
  }
}

RepetitiveInterval::Result
RepetitiveInterval::getInterval(MillisecondsSince1970 timePoint) const
{
  bool isPositive;
  double startTime;
  double endTime;

  if (!hasIntervalOnDate(timePoint)) {
    // There is no interval on the date of timePoint.
    startTime = toDateOnlyMilliseconds(timePoint);
    endTime = toDateOnlyMilliseconds(timePoint) + 24 * MILLISECONDS_IN_HOUR;
    isPositive = false;
  }
  else {
    // There is an interval on the date of timePoint.
    startTime =
      toDateOnlyMilliseconds(timePoint) + intervalStartHour_ * MILLISECONDS_IN_HOUR;
    endTime =
      toDateOnlyMilliseconds(timePoint) + intervalEndHour_ * MILLISECONDS_IN_HOUR;

    // Check if in the time duration.
    if (timePoint < startTime) {
      endTime = startTime;
      startTime = toDateOnlyMilliseconds(timePoint);
      isPositive = false;
    }
    else if (timePoint > endTime) {
      startTime = endTime;
      endTime = toDateOnlyMilliseconds(timePoint) + MILLISECONDS_IN_DAY;
      isPositive = false;
    }
    else
      isPositive = true;
  }

  return Result(isPositive, Interval(startTime, endTime));
}

int
RepetitiveInterval::compare(const RepetitiveInterval& other) const
{
  if (startDate_ < other.startDate_)
    return -1;
  if (startDate_ > other.startDate_)
    return 1;

  if (endDate_ < other.endDate_)
    return -1;
  if (endDate_ > other.endDate_)
    return 1;

  if (intervalStartHour_ < other.intervalStartHour_)
    return -1;
  if (intervalStartHour_ > other.intervalStartHour_)
    return 1;

  if (intervalEndHour_ < other.intervalEndHour_)
    return -1;
  if (intervalEndHour_ > other.intervalEndHour_)
    return 1;

  if (nRepeats_ < other.nRepeats_)
    return -1;
  if (nRepeats_ > other.nRepeats_)
    return 1;

  if (repeatUnit_ < other.repeatUnit_)
    return -1;
  if (repeatUnit_ > other.repeatUnit_)
    return 1;

  return 0;
}

bool
RepetitiveInterval::hasIntervalOnDate(MillisecondsSince1970 timePoint) const
{
  MillisecondsSince1970 timePointDateMilliseconds =
    toDateOnlyMilliseconds(timePoint);

  if (timePointDateMilliseconds < startDate_ ||
      timePointDateMilliseconds > endDate_)
    return false;

  if (repeatUnit_ == RepeatUnit::NONE)
    return true;
  else if (repeatUnit_ == RepeatUnit::DAY) {
    uint64_t durationDays = (uint64_t)(timePointDateMilliseconds - startDate_) /
                            MILLISECONDS_IN_DAY;
    if (durationDays % nRepeats_ == 0)
      return true;
  }
  else {
#if NDN_CPP_HAVE_GMTIME_SUPPORT
    time_t timePointDateSeconds = (time_t)::floor(timePointDateMilliseconds / 1000.0);
    struct tm timePointDateTm = *::gmtime(&timePointDateSeconds);
    time_t startDateSeconds = (time_t)::floor(startDate_ / 1000.0);
    struct tm startDateTm = *::gmtime(&startDateSeconds);

    if (repeatUnit_ == RepeatUnit::MONTH &&
        timePointDateTm.tm_mday == startDateTm.tm_mday) {
      int yearDifference = timePointDateTm.tm_year - startDateTm.tm_year;
      int monthDifference = 12 * yearDifference +
        timePointDateTm.tm_mon - startDateTm.tm_mon;
      if (monthDifference % nRepeats_ == 0)
        return true;
    }
    else if (repeatUnit_ == RepeatUnit::YEAR &&
             timePointDateTm.tm_mday == startDateTm.tm_mday &&
             timePointDateTm.tm_mon == startDateTm.tm_mon) {
      int difference = timePointDateTm.tm_year - startDateTm.tm_year;
      if (difference % nRepeats_ == 0)
        return true;
    }
#else
  throw runtime_error("Time functions are not supported by the standard library");
#endif
  }

  return false;
}

MillisecondsSince1970
RepetitiveInterval::toDateOnlyMilliseconds(MillisecondsSince1970 timePoint)
{
  uint64_t result = (uint64_t)::round(timePoint);
  result -= result % MILLISECONDS_IN_DAY;
  return result;
}

}
