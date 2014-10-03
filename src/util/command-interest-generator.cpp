/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from command-interest-generator.hpp by the NFD authors: https://github.com/named-data/NFD/blob/master/AUTHORS.md .
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
#include <ndn-cpp/security/key-chain.hpp>
#include "../c/util/time.h"
#include "../c/util/crypto.h"
#include "../encoding/tlv-encoder.hpp"
#include "command-interest-generator.hpp"

using namespace std;

namespace ndn {

CommandInterestGenerator::CommandInterestGenerator()
: lastTimestamp_(::round(ndn_getNowMilliseconds()))
{
}

void
CommandInterestGenerator::generate
  (Interest& interest, KeyChain& keyChain, const Name& certificateName,
   WireFormat& wireFormat)
{
  MillisecondsSince1970 timestamp = ::round(ndn_getNowMilliseconds());
  while (timestamp <= lastTimestamp_)
    timestamp += 1.0;

  // The timestamp is encoded as a TLV nonNegativeInteger.
  TlvEncoder encoder(8);
  encoder.writeNonNegativeInteger((uint64_t)timestamp);
  interest.getName().append(Blob(encoder.getOutput(), false));

  // The random value is a TLV nonNegativeInteger too, but we know it is 8 bytes,
  //   so we don't need to call the nonNegativeInteger encoder.
  uint8_t randomBuffer[8];
  ndn_generateRandomBytes(randomBuffer, sizeof(randomBuffer));
  interest.getName().append(randomBuffer, sizeof(randomBuffer));

  keyChain.sign(interest, certificateName, wireFormat);

  if (interest.getInterestLifetimeMilliseconds() < 0)
    // The caller has not set the interest lifetime, so set it here.
    interest.setInterestLifetimeMilliseconds(1000.0);

  // We successfully signed the interest, so update the timestamp.
  lastTimestamp_ = timestamp;
}

}
