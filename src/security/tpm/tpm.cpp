/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/tpm/tpm.cpp
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

#include <ndn-cpp/security/tpm/tpm-key-handle.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>

using namespace std;

namespace ndn {

Tpm::Tpm
  (const std::string& scheme, const std::string& location,
   const ptr_lib::shared_ptr<TpmBackEnd>& backEnd)
: scheme_(scheme),
  location_(location),
  backEnd_(backEnd)
{
}

string
Tpm::getTpmLocator() const { return scheme_ + ":" + location_; }

bool
Tpm::hasKey(const Name& keyName) const { return backEnd_->hasKey(keyName); }

Blob
Tpm::getPublicKey(const Name& keyName) const
{
  const TpmKeyHandle* key = findKey(keyName);

  if (!key)
    return Blob();
  else
    return key->derivePublicKey();
}

Blob
Tpm::sign
  (const uint8_t* data, size_t dataLength, const Name& keyName,
   DigestAlgorithm digestAlgorithm) const
{
  const TpmKeyHandle* key = findKey(keyName);

  if (!key)
    return Blob();
  else
    return key->sign(digestAlgorithm, data, dataLength);
}

Blob
Tpm::decrypt
  (const uint8_t* cipherText, size_t cipherTextLength, const Name& keyName) const
{
  const TpmKeyHandle* key = findKey(keyName);

  if (!key)
    return Blob();
  else
    return key->decrypt(cipherText, cipherTextLength);
}

bool
Tpm::isTerminalMode() const { return backEnd_->isTerminalMode(); }

void
Tpm::setTerminalMode(bool isTerminal) const
{ 
  backEnd_->setTerminalMode(isTerminal);
}

bool
Tpm::isTpmLocked() const { return backEnd_->isTpmLocked(); }

bool
Tpm::unlockTpm(const uint8_t* password, size_t passwordLength) const
{
  return backEnd_->unlockTpm(password, passwordLength);
}

Name
Tpm::createKey(const Name& identityName, const KeyParams& params)
{
  if (params.getKeyType() == KEY_TYPE_RSA ||
      params.getKeyType() == KEY_TYPE_EC) {
    ptr_lib::shared_ptr<TpmKeyHandle> keyHandle = backEnd_->createKey
      (identityName, params);
    Name keyName = keyHandle->getKeyName();
    keys_[keyName] = keyHandle;
    return keyName;
  }
  else
    throw Error("createKey: Unsupported key type");
}

void
Tpm::deleteKey(const Name& keyName)
{
  map<Name, ptr_lib::shared_ptr<TpmKeyHandle>>::const_iterator it =
    keys_.find(keyName);
  if (it != keys_.end())
    keys_.erase(it);

  backEnd_->deleteKey(keyName);
}

Blob
Tpm::exportPrivateKey
  (const Name& keyName, const uint8_t* password, size_t passwordLength)
{
  return backEnd_->exportKey(keyName, password, passwordLength);
}

void
Tpm::importPrivateKey
  (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
   const uint8_t* password, size_t passwordLength)
{
  backEnd_->importKey(keyName, pkcs8, pkcs8Length, password, passwordLength);
}

const TpmKeyHandle*
Tpm::findKey(const Name& keyName) const
{
  map<Name, ptr_lib::shared_ptr<TpmKeyHandle>>::const_iterator it =
    keys_.find(keyName);

  if (it != keys_.end())
    return it->second.get();

  ptr_lib::shared_ptr<TpmKeyHandle> handle = backEnd_->getKeyHandle(keyName);

  if (handle) {
    const_cast<Tpm*>(this)->keys_[keyName] = handle;
    return handle.get();
  }

  return 0;
}

}
