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

#ifndef NDN_NFD_COMMAND_INTEREST_GENERATOR_HPP
#define NDN_NFD_COMMAND_INTEREST_GENERATOR_HPP

#include <ndn-cpp/interest.hpp>

namespace ndn {

class KeyChain;

/** An CommandInterestGenerator keeps track of a timestamp and generates
 * command interests according to the NFD Signed Command Interests protocol:
 * http://redmine.named-data.net/projects/nfd/wiki/Command_Interests
 */
class CommandInterestGenerator {
public:
  /**
   * Create a new CommandInterestGenerator and initialize the timestamp to
   * now.
   */
  CommandInterestGenerator();

  /**
   * Append a timestamp component and a random value component to interest's
   * name. This ensures that the timestamp is greater than the timestamp used in
   * the previous call. Then use keyChain to sign the interest which appends a
   * SignatureInfo component and a component with the signature bits. If the
   * interest lifetime is not set, this sets it.
   * @param interest The interest whose name is append with components.
   * @param keyChain The KeyChain for calling sign.
   * @param certificateName The certificate name of the key to use for signing.
   * @param wireFormat A WireFormat object used to encode the SignatureInfo and
   * to encode interest name for signing. If omitted, use
   * WireFormat getDefaultWireFormat().
   */
  void
  generate
    (Interest& interest, KeyChain& keyChain, const Name& certificateName,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

private:
  MillisecondsSince1970 lastTimestamp_;
};

}

#endif
