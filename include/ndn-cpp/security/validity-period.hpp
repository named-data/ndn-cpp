/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx src/security https://github.com/named-data/ndn-cxx
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

#ifndef NDN_VALIDITY_PERIOD_HPP
#define NDN_VALIDITY_PERIOD_HPP

#include "../common.hpp"
#include "../lite/security/validity-period-lite.hpp"

namespace ndn {

/**
 * A ValidityPeriod is used in a Data packet's SignatureInfo and represents the
 * begin and end times of a certificate's validity period.
 */
class ValidityPeriod {
public:
  /** Create a default ValidityPeriod where the period is not specified.
   */
  ValidityPeriod() {}

  /**
   * Check if the period has been set.
   * @return True if the period has been set, false if the period is not
   * specified (after calling the default constructor or clear).
   */
  bool
  hasPeriod() const { return validityPeriod_.hasPeriod(); }

  /**
   * Get the beginning of the validity period range.
   * @return The time as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getNotBefore() const { return validityPeriod_.getNotBefore(); }

  /**
   * Get the end of the validity period range.
   * @return The time as milliseconds since Jan 1, 1970 UTC.
   */
  MillisecondsSince1970
  getNotAfter() const { return validityPeriod_.getNotAfter(); }

  /** Reset to a default ValidityPeriod where the period is not specified.
   */
  void
  clear() 
  {
    validityPeriod_.clear();
    ++changeCount_;
  }

  /**
   * Set the validity period.
   * @param notBefore The beginning of the validity period range as milliseconds
   * since Jan 1, 1970 UTC. Note that this is rounded up to the nearest whole
   * second.
   * @param notAfter The end of the validity period range as milliseconds
   * since Jan 1, 1970 UTC. Note that this is rounded down to the nearest whole
   * second.
   * @return This ValidityPeriod so that you can chain calls to update values.
   */
  ValidityPeriod&
  setPeriod
    (MillisecondsSince1970 notBefore,
     MillisecondsSince1970 notAfter)
  {
    validityPeriod_.setPeriod(notBefore, notAfter);
    ++changeCount_;
    return *this;
  }

  /**
   * Check if this is the same validity period as other.
   * @param other The other ValidityPeriod to compare with.
   * @return True if the validity periods are equal.
   */
  bool
  equals(const ValidityPeriod& other) const
  {
    return validityPeriod_.equals(other.validityPeriod_);
  }

  /**
   * Check if the time falls within the validity period.
   * @param time The time to check as milliseconds since Jan 1, 1970 UTC.
   * @return True if the beginning of the validity period is less than or equal
   * to time and time is less than or equal to the end of the validity period.
   */
  bool
  isValid(const MillisecondsSince1970& time) const
  {
    return validityPeriod_.isValid(time);
  }

  /**
   * Get the change count, which is incremented each time this object is changed.
   * @return The change count.
   */
  uint64_t
  getChangeCount() const { return changeCount_; }

  /**
   * Set validityPeriodLite to point to the values in this meta info object,
   * without copying any memory.
   * WARNING: The resulting pointers in validityPeriodLite are invalid after a
   * further use of this object which could reallocate memory.
   * @param validityPeriodLite The ValidityPeriodLite object which receives the
   * values.
   */
  void
  get(ValidityPeriodLite& validityPeriodLite) const
  {
    validityPeriodLite = validityPeriod_;
  }

  /**
   * Clear this meta info, and set the values by copying from validityPeriodLite.
   * @param validityPeriodLite A ValidityPeriodLite object.
   */
  void
  set(const ValidityPeriodLite& validityPeriodLite)
  {
    validityPeriod_ = validityPeriodLite;
    ++changeCount_;
  }

private:
  ValidityPeriodLite validityPeriod_;
  uint64_t changeCount_;
};

}

#endif
