/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2018-2020 Regents of the University of California.
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

#ifndef NDN_COMMAND_INTEREST_SIGNER_HPP
#define NDN_COMMAND_INTEREST_SIGNER_HPP

#include "key-chain.hpp"
#include "command-interest-preparer.hpp"

namespace ndn {

/**
 * CommandInterestSigner is a helper class to create command interests. This
 * keeps track of a timestamp and generates command interests by adding name
 * components according to the NFD Signed Command Interests protocol.
 * See makeCommandInterest() for details.
 * https://redmine.named-data.net/projects/ndn-cxx/wiki/CommandInterest
 */
class CommandInterestSigner : public CommandInterestPreparer {
public:
  /**
   * Create a CommandInterestSigner to use the keyChain to sign.
   * @param keyChain The KeyChain used to sign.
   */
  CommandInterestSigner(KeyChain& keyChain);

  static const int POS_SIGNATURE_VALUE = -1;
  static const int POS_SIGNATURE_INFO =  -2;
  static const int POS_NONCE =           -3;
  static const int POS_TIMESTAMP =       -4;

  static const int MINIMUM_SIZE = 4;

  /**
   * Append the timestamp and nonce name components to the supplied name, create
   * an Interest object and signs it with the KeyChain given to the constructor.
   * This ensures that the timestamp is greater than the timestamp used in the
   * previous call.
   * @param name The Name for the Interest, which is copied.
   * @param params (optional) The signing parameters. If omitted, use a
   * default SigningInfo().
   * @param wireFormat (optional) A WireFormat object used to encode the
   * SignatureInfo and to encode interest name for signing. If omitted, use
   * WireFormat getDefaultWireFormat().
   * @return A new command Interest object.
   */
  ptr_lib::shared_ptr<Interest>
  makeCommandInterest
    (const Name& name,
     const SigningInfo& params = KeyChain::getDefaultSigningInfo(),
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

private:
  KeyChain& keyChain_;
};

}

#endif
