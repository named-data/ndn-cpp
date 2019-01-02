/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/command-interest-signer.hpp
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

#include <math.h>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include "../c/util/time.h"
#include "../encoding/tlv-encoder.hpp"
#include <ndn-cpp/security/command-interest-preparer.hpp>

using namespace std;

namespace ndn {

CommandInterestPreparer::CommandInterestPreparer()
: lastUsedTimestamp_(::round(ndn_getNowMilliseconds())),
  nowOffsetMilliseconds_(0)
{}

void
CommandInterestPreparer::prepareCommandInterestName
  (Interest& interest, WireFormat& wireFormat)
{
  // nowOffsetMilliseconds_ is only used for testing.
  MillisecondsSince1970 now = ndn_getNowMilliseconds() + nowOffsetMilliseconds_;
  MillisecondsSince1970 timestamp = ::round(now);
  while (timestamp <= lastUsedTimestamp_)
    timestamp += 1.0;

  // Update the timestamp now. In the small chance that signing fails, it just
  // means that we have bumped the timestamp.
  lastUsedTimestamp_ = timestamp;

  // The timestamp is encoded as a TLV nonNegativeInteger.
  TlvEncoder encoder(8);
  encoder.writeNonNegativeInteger((uint64_t)timestamp);
  interest.getName().append(Blob(encoder.finish()));

  // The random value is a TLV nonNegativeInteger too, but we know it is 8 bytes,
  //   so we don't need to call the nonNegativeInteger encoder.
  uint8_t randomBuffer[8];
  ndn_Error error;
  if ((error = CryptoLite::generateRandomBytes(randomBuffer, sizeof(randomBuffer))))
    throw runtime_error(ndn_getErrorString(error));
  interest.getName().append(randomBuffer, sizeof(randomBuffer));
}

}
