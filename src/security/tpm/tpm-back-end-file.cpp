/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
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

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "../../encoding/base64.hpp"
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle-memory.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-file.hpp>

using namespace std;

namespace ndn {

TpmBackEndFile::TpmBackEndFile(const string& locationPath)
{
  if (locationPath != "") {
    keyStorePath_ = locationPath;
    if (keyStorePath_[keyStorePath_.size() - 1] == '/' ||
        keyStorePath_[keyStorePath_.size() - 1] == '\\')
      // Strip the ending path separator.
      keyStorePath_.erase(keyStorePath_.size() - 1);
  }
  else {
    // Note: We don't use <filesystem> support because it is not "header-only"
    // and requires linking to libraries.
    const char* home = getenv("HOME");
    if (!home || *home == '\0')
      // Don't expect this to happen;
      home = ".";
    string homeDir(home);
    if (homeDir[homeDir.size() - 1] == '/' || homeDir[homeDir.size() - 1] == '\\')
      // Strip the ending path separator.
      homeDir.erase(homeDir.size() - 1);

    // TODO: Handle non-unix file systems which don't use "/".
    keyStorePath_ = homeDir + "/.ndn/ndnsec-key-file";
  }

  // TODO: Handle non-unix file systems which don't have "mkdir -p".
  ::system(("mkdir -p \"" + keyStorePath_ + "\"").c_str());
}

bool
TpmBackEndFile::doHasKey(const Name& keyName) const
{
  {
    ifstream file(toFilePath(keyName).c_str());
    if (!file.good())
      return false;
  }

  try {
    loadKey(keyName);
    return true;
  }
  catch (const runtime_error&) {
    return false;
  }
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndFile::doGetKeyHandle(const Name& keyName) const
{
  if (!doHasKey(keyName))
    return ptr_lib::shared_ptr<TpmKeyHandle>();

  return ptr_lib::make_shared<TpmKeyHandleMemory>(loadKey(keyName));
}

ptr_lib::shared_ptr<TpmKeyHandle>
TpmBackEndFile::doCreateKey(const Name& identityName, const KeyParams& params)
{
  ptr_lib::shared_ptr<TpmPrivateKey> key = TpmPrivateKey::generatePrivateKey
    (params);
  ptr_lib::shared_ptr<TpmKeyHandle> keyHandle(new TpmKeyHandleMemory(key));

  setKeyName(*keyHandle, identityName, params);

  try {
    saveKey(keyHandle->getKeyName(), key);
    return keyHandle;
  }
  catch (const runtime_error& e) {
    throw Error(string("Cannot write the key to disk: ") + e.what());
  }
}

void
TpmBackEndFile::doDeleteKey(const Name& keyName)
{
  string keyPath = toFilePath(keyName);

  {
    ifstream file(keyPath.c_str());
    if (!file.good())
      // Already removed.
      return;
  }

  if (remove(keyPath.c_str()) != 0)
    throw Error("Cannot delete the key");
}

ptr_lib::shared_ptr<TpmPrivateKey>
TpmBackEndFile::loadKey(const Name& keyName) const
{
  ptr_lib::shared_ptr<TpmPrivateKey> key(new TpmPrivateKey());
  ifstream file(toFilePath(keyName).c_str());
  stringstream base64;
  base64 << file.rdbuf();
  vector<uint8_t> pkcs;
  fromBase64(base64.str(), pkcs);

  key->loadPkcs1(&pkcs.front(), pkcs.size());
  return key;
}

void
TpmBackEndFile::saveKey
  (const Name& keyName, const ptr_lib::shared_ptr<TpmPrivateKey>& key)
{
  string filePath = toFilePath(keyName);
  ofstream file(filePath.c_str());
  file << toBase64(*key->toPkcs1(), true);

  // Set the file permissions.
#if !defined(_WIN32) // Windows doesn't have Unix group permissions.
  ::chmod(filePath.c_str(), S_IRUSR);
#endif
}

string
TpmBackEndFile::toFilePath(const Name& keyName) const
{
  Blob keyEncoding = keyName.wireEncode();
  uint8_t digest[ndn_SHA256_DIGEST_SIZE];
  CryptoLite::digestSha256(keyEncoding, digest);

  return keyStorePath_ + "/" + Blob(digest, sizeof(digest)).toHex() + ".privkey";
}

}
