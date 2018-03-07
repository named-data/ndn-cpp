/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/tpm/key-handle-mem.cpp
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

#include <stdexcept>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle-memory.hpp>

using namespace std;

namespace ndn {

TpmKeyHandleMemory::TpmKeyHandleMemory
  (const ptr_lib::shared_ptr<TpmPrivateKey>& key)
: key_(key)
{
  if (!key)
    throw runtime_error("The key is null");
}

Blob
TpmKeyHandleMemory::doSign
  (DigestAlgorithm digestAlgorithm, const uint8_t* data, size_t dataLength) const
{
  if (digestAlgorithm == DIGEST_ALGORITHM_SHA256)
    return key_->sign(data, dataLength, digestAlgorithm);
  else
    return Blob();
}

Blob
TpmKeyHandleMemory::doDecrypt
  (const uint8_t* cipherText, size_t cipherTextLength) const
{
  return key_->decrypt(cipherText, cipherTextLength);
}

Blob
TpmKeyHandleMemory::doDerivePublicKey() const
{
  return key_->derivePublicKey();
}

}
