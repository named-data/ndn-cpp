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

#include <ndn-cpp/security/key-chain.hpp>
#include "command-interest-generator.hpp"

using namespace std;

namespace ndn {

void
CommandInterestGenerator::generate
  (Interest& interest, KeyChain& keyChain, const Name& certificateName,
   WireFormat& wireFormat)
{
  prepareCommandInterestName(interest, wireFormat);
  keyChain.sign(interest, certificateName, wireFormat);

  if (interest.getInterestLifetimeMilliseconds() < 0)
    // The caller has not set the interest lifetime, so set it here.
    interest.setInterestLifetimeMilliseconds(1000.0);
}

}
