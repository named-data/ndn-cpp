/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/tpm/back-end.cpp
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
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/security/pib/pib-key.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end.hpp>

using namespace std;

namespace ndn {

TpmBackEnd::~TpmBackEnd() {}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEnd::createKey(const Name& identityName, const KeyParams& params)
{
  // Do key name checking.
  if (params.getKeyIdType() == KEY_ID_TYPE_USER_SPECIFIED) {
    // The keyId is pre-set.
    Name keyName = PibKey::constructKeyName(identityName, params.getKeyId());
    if (hasKey(keyName))
      throw Tpm::Error("Key `" + keyName.toUri() + "` already exists");
  }
  else if (params.getKeyIdType() == KEY_ID_TYPE_SHA256) {
    // The key name will be assigned in setKeyName after the key is generated.
  }
  else if (params.getKeyIdType() == KEY_ID_TYPE_RANDOM) {
    Name keyName;
    Name::Component keyId;
    uint8_t random[8];
    do {
      ndn_Error error;
      if ((error = CryptoLite::generateRandomBytes(random, sizeof(random))))
        throw Error
          (string("Error generating random key ID: ") + ndn_getErrorString(error));
      keyId = Name::Component(random, sizeof(random));
      keyName = PibKey::constructKeyName(identityName, keyId);
    } while (hasKey(keyName));

    const_cast<KeyParams&>(params).setKeyId(keyId);
  }
  else
    throw Error("Unsupported key id type");

  return doCreateKey(identityName, params);
}

Blob
TpmBackEnd::exportKey
  (const Name& keyName, const uint8_t* password, size_t passwordLength)
{
  if (!hasKey(keyName))
    throw Error("Key `" + keyName.toUri() + "` does not exist");

  return doExportKey(keyName, password, passwordLength);
}

void
TpmBackEnd::importKey
  (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
   const uint8_t* password, size_t passwordLength)
{
  if (hasKey(keyName))
    throw Error("Key `" + keyName.toUri() + "` already exists");

  doImportKey(keyName, pkcs8, pkcs8Length, password, passwordLength);
}

void
TpmBackEnd::setKeyName
  (TpmKeyHandle& keyHandle, const Name& identityName, const KeyParams& params)
{
  Name::Component keyId;
  if (params.getKeyIdType() == KEY_ID_TYPE_USER_SPECIFIED)
    keyId = params.getKeyId();
  else if (params.getKeyIdType() == KEY_ID_TYPE_SHA256) {
    uint8_t digest[ndn_SHA256_DIGEST_SIZE];
    CryptoLite::digestSha256(keyHandle.derivePublicKey(), digest);
    keyId = Name::Component(digest, sizeof(digest));
  }
  else if (params.getKeyIdType() == KEY_ID_TYPE_RANDOM) {
    if (params.getKeyId().getValue().size() == 0)
      throw runtime_error("setKeyName: The keyId is empty for type RANDOM");
    keyId = params.getKeyId();
  }
  else
    throw runtime_error("setKeyName: unrecognized params.getKeyIdType()");

  keyHandle.setKeyName(PibKey::constructKeyName(identityName, keyId));
}

bool
TpmBackEnd::isTerminalMode() const { return true; }

void
TpmBackEnd::setTerminalMode(bool isTerminal) const {}

bool
TpmBackEnd::isTpmLocked() const { return false; }

bool
TpmBackEnd::unlockTpm(const uint8_t* password, size_t passwordLength) const
{
  return !isTpmLocked();
}

Blob
TpmBackEnd::doExportKey
  (const Name& keyName, const uint8_t* password, size_t passwordLength)
{
  throw runtime_error("TpmBackEnd doExportKey is not implemented");
}

void
TpmBackEnd::doImportKey
  (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
   const uint8_t* password, size_t passwordLength)
{
  throw runtime_error("TpmBackEnd doImportKey is not implemented");
}

}
