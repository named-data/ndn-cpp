/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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

#include <float.h>
#include <stdexcept>
#include <ndn-cpp/encrypt/interval.hpp>

using namespace std;

namespace ndn {

Interval::Error::Error(const string& errorMessage) throw()
: errorMessage_(errorMessage)
{
}

Interval::Error::~Error() throw() {}

const char*
Interval::Error::what() const throw() { return errorMessage_.c_str(); }

Interval::Interval(bool isValid)
{
  isValid_ = isValid;
  startTime_ = -DBL_MAX;
  endTime_ = -DBL_MAX;
}

Interval::Interval(MillisecondsSince1970 startTime, MillisecondsSince1970 endTime)
{
  if (!(startTime < endTime))
    throw runtime_error("Interval start time must be less than the end time");

  isValid_ = true;
  startTime_ = startTime;
  endTime_ = endTime;
}

bool
Interval::covers(MillisecondsSince1970 timePoint) const
{
  if (!isValid_)
    throw runtime_error("Interval.covers: This Interval is invalid");

  if (isEmpty())
    return false;
  else
    return startTime_ <= timePoint && timePoint < endTime_;
}

Interval&
Interval::intersectWith(const Interval& interval)
{
  if (!isValid_)
    throw runtime_error("Interval.intersectWith: This Interval is invalid");
  if (!interval.isValid_)
    throw runtime_error("Interval.intersectWith: The other Interval is invalid");

  if (isEmpty() || interval.isEmpty()) {
    // If either is empty, the result is empty.
    startTime_ = endTime_;
    return *this;
  }

  if (startTime_ >= interval.endTime_ || endTime_ <= interval.startTime_) {
    // The two intervals don't have an intersection, so the result is empty.
    startTime_ = endTime_;
    return *this;
  }

  // Get the start time.
  if (startTime_ <= interval.startTime_)
    startTime_ = interval.startTime_;

  // Get the end time.
  if (endTime_ > interval.endTime_)
    endTime_ = interval.endTime_;

  return *this;
}

Interval&
Interval::unionWith(const Interval& interval)
{
  if (!isValid_)
    throw runtime_error("Interval.intersectWith: This Interval is invalid");
  if (!interval.isValid_)
    throw runtime_error("Interval.intersectWith: The other Interval is invalid");

  if (isEmpty()) {
    // This interval is empty, so use the other.
    startTime_ = interval.startTime_;
    endTime_ = interval.endTime_;
    return *this;
  }

  if (interval.isEmpty())
    // The other interval is empty, so keep using this one.
    return *this;

  if (startTime_ >= interval.endTime_ || endTime_ <= interval.startTime_)
    throw Interval::Error
      ("Interval.unionWith: The two intervals do not have an intersection");

  // Get the start time.
  if (startTime_ > interval.startTime_)
    startTime_ = interval.startTime_;

  // Get the end time.
  if (endTime_ < interval.endTime_)
    endTime_ = interval.endTime_;

  return *this;
}

MillisecondsSince1970
Interval::getStartTime() const
{
  if (!isValid_)
    throw runtime_error("Interval.getStartTime: This Interval is invalid");
  return startTime_;
}

MillisecondsSince1970
Interval::getEndTime() const
{
  if (!isValid_)
    throw runtime_error("Interval.getEndTime: This Interval is invalid");
  return endTime_;
}

bool
Interval::isEmpty() const
{
  if (!isValid_)
    throw runtime_error("Interval.isEmpty: This Interval is invalid");
  return startTime_ == endTime_;
}

}
