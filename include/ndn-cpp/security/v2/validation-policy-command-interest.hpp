/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validation-policy-command-interest.hpp
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

#ifndef NDN_VALIDATION_POLICY_COMMAND_INTEREST_HPP
#define NDN_VALIDATION_POLICY_COMMAND_INTEREST_HPP

#include <map>
#include "validation-policy.hpp"

namespace ndn {

/**
 * ValidationPolicyCommandInterest extends ValidationPolicy as a policy for
 * stop-and-wait command Interests. See:
 * https://redmine.named-data.net/projects/ndn-cxx/wiki/CommandInterest
 *
 * This policy checks the timestamp field of a stop-and-wait command Interest.
 * Signed Interest validation and Data validation requests are delegated to an
 * inner policy.
 */
class ValidationPolicyCommandInterest : public ValidationPolicy
{
public:
  class Options {
  public:
    /**
     * Create a ValidationPolicyCommandInterest::Options with the optional
     * values.
     * @param gracePeriod (optional) See below for description. If omitted, use
     * 2 minutes.
     * @param maxRecords (optional) See below for description. If omitted, use
     * 1000 records.
     * @param recordLifetime (optional) See below for description. If omitted,
     * use 1 hour.
     */
    Options
      (Milliseconds gracePeriod = 2 * 60 * 1000.0, int maxRecords = 1000,
       Milliseconds recordLifetime = 3600 * 1000.0)
    : gracePeriod_(gracePeriod),
      maxRecords_(maxRecords),
      recordLifetime_(recordLifetime)
    {
    }

    /**
     * gracePeriod is the tolerance of the initial timestamp in milliseconds.
     *
     * A stop-and-wait command Interest is considered "initial" if the validator
     * has not recorded the last timestamp from the same public key, or when
     * such knowledge has been erased. For an initial command Interest, its
     * timestamp is compared to the current system clock, and the command
     * Interest is rejected if the absolute difference is greater than the grace
     * interval.
     *
     * This should be positive. Setting this option to 0 or negative causes the
     * validator to require exactly the same timestamp as the system clock,
     * which most likely rejects all command Interests.
     */
    Milliseconds gracePeriod_;

    /**
     * maxRecords is the maximum number of distinct public keys of which to
     * record the last timestamp.
     *
     * The validator records the last timestamps for every public key. For a
     * subsequent command Interest using the same public key, its timestamp is
     * compared to the last timestamp from that public key, and the command
     * Interest is rejected if its timestamp is less than or equal to the
     * recorded timestamp.
     *
     * This option limits the number of distinct public keys being tracked. If
     * the limit is exceeded, then the oldest record is deleted.
     *
     * Setting this option to -1 allows tracking unlimited public keys. Setting
     * this option to 0 disables using last timestamp records and causes every
     * command Interest to be processed as initial.
     */
    int maxRecords_;

    /**
     * recordLifetime is the maximum lifetime of a last timestamp record in
     * milliseconds.
     *
     * A last timestamp record expires and can be deleted if it has not been
     * refreshed within this duration. Setting this option to 0 or negative
     * makes last timestamp records expire immediately and causes every command
     * Interest to be processed as initial.
     */
    Milliseconds recordLifetime_;
  };

  /**
   * @param innerPolicy a ValidationPolicy for signed Interest signature
   * validation and Data validation. This must not be null.
   * @param options (optional) The stop-and-wait command Interest validation
   * options.
   * @throws std::invalid_argument if innerPolicy is null.
   */
  ValidationPolicyCommandInterest
    (const ptr_lib::shared_ptr<ValidationPolicy>& innerPolicy,
     const Options& options = Options());

  virtual void
  checkPolicy
    (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

  virtual void
  checkPolicy
    (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

  /**
   * Set the offset when insertNewRecord() and cleanUp() get the current time,
   * which should only be used for testing.
   * @param nowOffsetMilliseconds The offset in milliseconds.
   */
  void
  setNowOffsetMilliseconds_(Milliseconds nowOffsetMilliseconds)
  {
    nowOffsetMilliseconds_ = nowOffsetMilliseconds;
  }

private:
  class LastTimestampRecord
  {
  public:
    LastTimestampRecord
      (const Name& keyName, MillisecondsSince1970 timestamp,
       MillisecondsSince1970 lastRefreshed)
    : keyName_(keyName), timestamp_(timestamp), lastRefreshed_(lastRefreshed)
    {}

    Name keyName_;
    MillisecondsSince1970 timestamp_;
    MillisecondsSince1970 lastRefreshed_;
  };

  void
  cleanUp();

  /**
   * Get the keyLocatorName and timestamp from the command interest.
   * @param interest The Interest to parse.
   * @param state On error, this calls state.fail and returns false.
   * @param keyLocatorName Set this to the KeyLocator name.
   * @param timestamp Set this to the timestamp as milliseconds since Jan 1,
   * 1970 UTC.
   * @return On success, return true. On error, call state.fail and return false.
   */
  static bool
  parseCommandInterest
    (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
     Name& keyLocatorName, MillisecondsSince1970& timestamp);

  /**
   *
   * @param state On error, this calls state.fail and returns false.
   * @param keyName The key name.
   * @param timestamp The timestamp as milliseconds since Jan 1, 1970 UTC.
   * @return On success, return true. On error, call state.fail and return false.
   */
  bool
  checkTimestamp
    (const ptr_lib::shared_ptr<ValidationState>& state, const Name& keyName,
     MillisecondsSince1970 timestamp);

  void
  insertNewRecord
    (const Interest& interest, const Name& keyName,
     MillisecondsSince1970 timestamp);

  /**
   * Find the record in container_ which has the keyName.
   * @param keyName The key name to search for.
   * @return The index in container_ of the record, or -1 if not found.
   */
  int
  findByKeyName(const Name& keyName);

  Options options_;
  std::vector<ptr_lib::shared_ptr<LastTimestampRecord> > container_;
  Milliseconds nowOffsetMilliseconds_;
};

}

#endif
