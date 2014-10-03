/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#ifndef NDN_SHA256_RSA_HANDLER_HPP
#define NDN_SHA256_RSA_HANDLER_HPP

#include "../../data.hpp"
#include "../certificate/public-key.hpp"

namespace ndn{

class Sha256WithRsaHandler {
public:
  Sha256WithRsaHandler() {}

  virtual
  ~Sha256WithRsaHandler() {}

 /**
  * Verify the signature on the data packet using the given public key. If there is no data.getDefaultWireEncoding(),
  * this calls data.wireEncode() to set it.
  * @param data The data packet with the signed portion and the signature to verify. The data packet must have a
  * Sha256WithRsaSignature.
  * @param publicKey The public key used to verify the signature.
  * @return true if the signature verifies, false if not.
  * @throw SecurityException if data does not have a Sha256WithRsaSignature.
  */
  static bool
  verifySignature(const Data& data, const PublicKey& publicKey);

};

}
#endif
