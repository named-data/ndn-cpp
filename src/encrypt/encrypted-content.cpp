/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt src/encrypted-content https://github.com/named-data/ndn-group-encrypt
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
#include "../c/encrypt/encrypted-content.h"
#include <ndn-cpp/lite/encrypt/encrypted-content-lite.hpp>
#include <ndn-cpp/encrypt/encrypted-content.hpp>

using namespace std;

namespace ndn {

const Name&
EncryptedContent::getKeyLocatorName() const
{
  if (keyLocator_.getType() != ndn_KeyLocatorType_KEYNAME)
    throw runtime_error("getKeyLocatorName: The KeyLocator type must be KEYNAME");

  return keyLocator_.getKeyName();
}

void
EncryptedContent::get(EncryptedContentLite& encryptedContentLite) const
{
  encryptedContentLite.setAlgorithmType(algorithmType_);
  keyLocator_.get(encryptedContentLite.getKeyLocator());
  encryptedContentLite.setInitialVector(initialVector_);
  encryptedContentLite.setPayload(payload_);
  encryptedContentLite.setPayloadKey(payloadKey_);
}

void
EncryptedContent::set(const EncryptedContentLite& encryptedContentLite)
{
  setAlgorithmType(encryptedContentLite.getAlgorithmType());
  keyLocator_.set(encryptedContentLite.getKeyLocator());
  setInitialVector(Blob(encryptedContentLite.getInitialVector()));
  setPayload(Blob(encryptedContentLite.getPayload()));
  setPayloadKey(Blob(encryptedContentLite.getPayloadKey()));
}

}
