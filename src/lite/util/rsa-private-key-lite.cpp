/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include "../../c/util/rsa-private-key.h"
#include <ndn-cpp/lite/util/rsa-private-key-lite.hpp>

namespace ndn {

RsaPrivateKeyLite::RsaPrivateKeyLite()
{
  ndn_RsaPrivateKey_initialize(this);
}

RsaPrivateKeyLite::~RsaPrivateKeyLite()
{
  ndn_RsaPrivateKey_finalize(this);
}

ndn_Error
RsaPrivateKeyLite::decode
  (const uint8_t* privateKeyDer, size_t privateKeyDerLength)
{
  return ndn_RsaPrivateKey_decode(this, privateKeyDer, privateKeyDerLength);
}

ndn_Error
RsaPrivateKeyLite::signWithSha256
  (const uint8_t* data, size_t dataLength, const uint8_t* signature,
   size_t& signatureLength) const
{
  return ndn_RsaPrivateKey_signWithSha256
    (this, data, dataLength, signature, &signatureLength);
}

}
