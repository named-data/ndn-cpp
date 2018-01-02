/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/tpm/back-end-mem.cpp
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

#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle-memory.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>

using namespace std;

namespace ndn {

TpmBackEndMemory::TpmBackEndMemory()
{
}

bool
TpmBackEndMemory::doHasKey(const Name& keyName) const
{
  return (keys_.count(keyName) > 0);
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndMemory::doGetKeyHandle(const Name& keyName) const
{
  map<Name, ptr_lib::shared_ptr<TpmPrivateKey>>::const_iterator it =
    keys_.find(keyName);
  if (it == keys_.end())
    return ptr_lib::shared_ptr<TpmKeyHandle>();
  return ptr_lib::make_shared<TpmKeyHandleMemory>(it->second);
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndMemory::doCreateKey(const Name& identityName, const KeyParams& params)
{
  ptr_lib::shared_ptr<TpmPrivateKey> key =
    TpmPrivateKey::generatePrivateKey(params);
  ptr_lib::shared_ptr<TpmKeyHandle> keyHandle(new TpmKeyHandleMemory(key));

  setKeyName(*keyHandle, identityName, params);

  keys_[keyHandle->getKeyName()] = key;
  return keyHandle;
}

void
TpmBackEndMemory::doDeleteKey(const Name& keyName) { keys_.erase(keyName); }

Blob
TpmBackEndMemory::doExportKey
  (const Name& keyName, const uint8_t* password, size_t passwordLength)
{
  if (password)
    throw Error("Private key password-encryption is not implemented");
  else {
    if (!hasKey(keyName))
      throw Error("exportKey: The key does not exist");

    return keys_[keyName]->toPkcs8();
  }
}

void
TpmBackEndMemory::doImportKey
  (const Name& keyName, const uint8_t* pkcs8, size_t pkcs8Length,
   const uint8_t* password, size_t passwordLength)
{
  try {
    if (password)
      throw Error("Private key password-encryption is not implemented");
    else {
      ptr_lib::shared_ptr<TpmPrivateKey> key(new TpmPrivateKey());
      key->loadPkcs8(pkcs8, pkcs8Length);
      keys_[keyName] = key;
    }
  } catch (const TpmPrivateKey::Error& ex) {
    throw Error(string("Cannot import private key: ") + ex.what());
  }
}

}
