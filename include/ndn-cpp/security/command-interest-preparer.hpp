/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/command-interest-signer.hpp
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

#ifndef NDN_COMMAND_INTEREST_PREPARER_HPP
#define NDN_COMMAND_INTEREST_PREPARER_HPP

#include "../interest.hpp"

namespace ndn {

/**
 * A CommandInterestPreparer keeps track of a timestamp and prepares a command
 * interest by adding a timestamp and nonce to the name of an Interest. This
 * class is primarily designed to be used by the CommandInterestSigner, but can
 * also be using in an application that defines custom signing methods not
 * supported by the KeyChain (such as HMAC-SHA1). See the Command Interest
 * documentation:
 * https://redmine.named-data.net/projects/ndn-cxx/wiki/CommandInterest
 */
class CommandInterestPreparer {
public:
  /**
   * Create a CommandInterestPreparer and initialize the timestamp to now.
   */
  CommandInterestPreparer();

  /**
   * Append a timestamp component and a random nonce component to interest's
   * name. This ensures that the timestamp is greater than the timestamp used in
   * the previous call.
   * @param interest The interest whose name is append with components.
   * @param wireFormat (optional) A WireFormat object used to encode the
   * SignatureInfo. If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  prepareCommandInterestName
    (Interest& interest,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Set the offset for when prepareCommandInterestName() gets the current time,
   * which should only be used for testing.
   * @param nowOffsetMilliseconds The offset in milliseconds.
   */
  void
  setNowOffsetMilliseconds_(Milliseconds nowOffsetMilliseconds)
  {
    nowOffsetMilliseconds_ = nowOffsetMilliseconds;
  }

private:
  MillisecondsSince1970 lastUsedTimestamp_;
  Milliseconds nowOffsetMilliseconds_;
};

}

#endif
