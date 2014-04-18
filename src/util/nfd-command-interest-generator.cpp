/**
 * @author: Jeff Thompson
 * Derived from command-interest-generator.hpp by the NFD authors: https://github.com/named-data/NFD/blob/master/AUTHORS.md .
 * See COPYING for copyright and distribution information.
 */

#include <math.h>
#include <ndn-cpp/security/key-chain.hpp>
#include "../c/util/time.h"
#include "../c/util/crypto.h"
#include "nfd-command-interest-generator.hpp"

using namespace std;

namespace ndn {

NfdCommandInterestGenerator::NfdCommandInterestGenerator()
: lastTimestamp_(::round(ndn_getNowMilliseconds()))
{
}

void
NfdCommandInterestGenerator::generate
  (Interest& interest, KeyChain& keyChain, const Name& certificateName, 
   WireFormat& wireFormat)
{
  MillisecondsSince1970 timestamp = ::round(ndn_getNowMilliseconds());
  while (timestamp <= lastTimestamp_)
    timestamp += 1.0;

  interest.getName().append
    (Name::Component::fromNumber(ndn_getNowMilliseconds()));
  uint8_t randomBuffer[8];
  ndn_generateRandomBytes(randomBuffer, sizeof(randomBuffer));
  interest.getName().append(randomBuffer, sizeof(randomBuffer));

  keyChain.sign(interest, certificateName);

  // We successfully signed the interest, so update the timestamp.
  lastTimestamp_ = timestamp;
}

}
