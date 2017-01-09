/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/algo/encrypt-params https://github.com/named-data/ndn-group-encrypt
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

#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/encrypt/algo/encrypt-params.hpp>

using namespace std;

namespace ndn {

EncryptParams::EncryptParams
  (ndn_EncryptAlgorithmType algorithmType, size_t initialVectorLength)
{
  algorithmType_ = algorithmType;

  if (initialVectorLength > 0) {
    ptr_lib::shared_ptr<vector<uint8_t> > initialVector
      (new vector<uint8_t>(initialVectorLength));
    ndn_Error error;
    if ((error = CryptoLite::generateRandomBytes
         (&initialVector->front(), initialVector->size())))
      throw runtime_error(ndn_getErrorString(error));
    initialVector_ = Blob(initialVector, false);
  }
}

}
