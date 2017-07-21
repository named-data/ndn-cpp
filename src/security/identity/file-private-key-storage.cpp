/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "../../encoding/base64.hpp"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/identity/file-private-key-storage.hpp>

using namespace std;

namespace ndn {

static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";
static const char *EC_ENCRYPTION_OID = "1.2.840.10045.2.1";

FilePrivateKeyStorage::FilePrivateKeyStorage(const string& keyStoreDirectoryPath)
{
  if (keyStoreDirectoryPath != "") {
    keyStorePath_ = keyStoreDirectoryPath;
    if (keyStorePath_[keyStorePath_.size() - 1] == '/' ||
        keyStorePath_[keyStorePath_.size() - 1] == '\\')
      // Strip the ending path separator.
      keyStorePath_.erase(keyStorePath_.size() - 1);
  }
  else {
    // Note: We don't use <filesystem> support because it is not "header-only" and
    // require linking to libraries.
    // TODO: Handle non-unix file system paths which don't use '/'.
    const char* home = getenv("HOME");
    if (!home || *home == '\0')
      // Don't expect this to happen;
      home = ".";
    string homeDir(home);
    if (homeDir[homeDir.size() - 1] == '/' || homeDir[homeDir.size() - 1] == '\\')
      // Strip the ending path separator.
      homeDir.erase(homeDir.size() - 1);

    keyStorePath_ = homeDir + '/' + ".ndn/ndnsec-tpm-file";
  }
  
  // TODO: Handle non-unix file systems which don't have "mkdir -p".
  ::system(("mkdir -p " + keyStorePath_).c_str());
}

FilePrivateKeyStorage::~FilePrivateKeyStorage()
{
}

void
FilePrivateKeyStorage::generateKeyPair
  (const Name& keyName, const KeyParams& params)
{
  if (doesKeyExist(keyName, KEY_CLASS_PUBLIC))
    throw SecurityException("Public Key already exists");
  if (doesKeyExist(keyName, KEY_CLASS_PRIVATE))
    throw SecurityException("Private Key already exists");

  try {
    ptr_lib::shared_ptr<TpmPrivateKey> privateKey =
      TpmPrivateKey::generatePrivateKey(params);

    string keyUri = keyName.toUri();
    string keyFilePathNoExtension = maintainMapping(keyUri);
    string publicKeyFilePath = keyFilePathNoExtension + ".pub";
    string privateKeyFilePath = keyFilePathNoExtension + ".pri";

    ofstream publicKeyFile(publicKeyFilePath.c_str());
    publicKeyFile << toBase64(*privateKey->derivePublicKey(), true);
    ofstream privateKeyFile(privateKeyFilePath.c_str());
    // Set includeParameters false because the legacy ndn-cxx stores the EC key
    // without parameters.
    privateKeyFile << toBase64(*privateKey->toPkcs8(false), true);

#if !defined(_WIN32) // Windows doesn't have Unix group permissions.
    ::chmod(publicKeyFilePath.c_str(),  S_IRUSR | S_IRGRP | S_IROTH);
    ::chmod(privateKeyFilePath.c_str(), S_IRUSR);
#endif
  } catch (TpmPrivateKey::Error& ex) {
    throw SecurityException(ex.what());
  }
}

void
FilePrivateKeyStorage::deleteKeyPair(const Name& keyName)
{
  string keyUri = keyName.toUri();

  remove(nameTransform(keyUri, ".pub").c_str());
  remove(nameTransform(keyUri, ".pri").c_str());
}

ptr_lib::shared_ptr<PublicKey>
FilePrivateKeyStorage::getPublicKey(const Name& keyName)
{
  string keyURI = keyName.toUri();

  if (!doesKeyExist(keyName, KEY_CLASS_PUBLIC))
    throw SecurityException("Public Key does not exist.");

  ifstream file(nameTransform(keyURI, ".pub").c_str());
  stringstream base64;
  base64 << file.rdbuf();

  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t> > der(new vector<uint8_t>());
  fromBase64(base64.str(), *der);

  Blob derBlob(der, false);
  return ptr_lib::shared_ptr<PublicKey>(new PublicKey(derBlob));
}

Blob
FilePrivateKeyStorage::sign
  (const uint8_t *data, size_t dataLength, const Name& keyName,
   DigestAlgorithm digestAlgorithm)
{
  string keyURI = keyName.toUri();

  if (!doesKeyExist(keyName, KEY_CLASS_PRIVATE))
    throw SecurityException
      ("FilePrivateKeyStorage::sign: private key doesn't exist");

  // Read the private key.
  ifstream file(nameTransform(keyURI, ".pri").c_str());
  stringstream base64;
  base64 << file.rdbuf();
  vector<uint8_t> pkcs8Der;
  fromBase64(base64.str(), pkcs8Der);

  TpmPrivateKey privateKey;
  try {
    privateKey.loadPkcs8(&pkcs8Der.front(), pkcs8Der.size());
    return privateKey.sign(data, dataLength, digestAlgorithm);
  } catch (TpmPrivateKey::Error& ex) {
    throw SecurityException(ex.what());
  }
}

Blob
FilePrivateKeyStorage::decrypt
  (const Name& keyName, const uint8_t* data, size_t dataLength,
   bool isSymmetric)
{
  throw runtime_error("FilePrivateKeyStorage::decrypt not implemented");
}

Blob
FilePrivateKeyStorage::encrypt
  (const Name& keyName, const uint8_t* data, size_t dataLength,
   bool isSymmetric)
{
  throw runtime_error("FilePrivateKeyStorage::encrypt not implemented");
}

void
FilePrivateKeyStorage::generateKey(const Name& keyName, const KeyParams& params)
{
  throw runtime_error("FilePrivateKeyStorage::generateKey not implemented");
}

bool
FilePrivateKeyStorage::doesKeyExist(const Name& keyName, KeyClass keyClass)
{
  string keyURI = keyName.toUri();
  string filePath;
  if (keyClass == KEY_CLASS_PUBLIC)
    filePath = nameTransform(keyURI, ".pub");
  else if (keyClass == KEY_CLASS_PRIVATE)
    filePath = nameTransform(keyURI, ".pri");
  else if (keyClass == KEY_CLASS_SYMMETRIC)
    filePath = nameTransform(keyURI, ".key").c_str();
  else
    return false;

  ifstream file(filePath.c_str());
  return file.good();
}

string
FilePrivateKeyStorage::nameTransform
  (const string& keyName, const string& extension)
{
  uint8_t hash[ndn_SHA256_DIGEST_SIZE];
  CryptoLite::digestSha256((uint8_t*)&keyName[0], keyName.size(), hash);

  string digest = toBase64(hash, sizeof(hash));
  ndn_trim(digest);
  // TODO: Handle non-unix file system paths which don't use '/'.
  std::replace(digest.begin(), digest.end(), '/', '%');

  return keyStorePath_ + "/" + digest + extension;
}

string
FilePrivateKeyStorage::maintainMapping(const string& keyName)
{
  string keyFilePathNoExtension = nameTransform(keyName, "");

  // TODO: Handle non-unix file system paths which don't use '/'.
  string mappingFilePath = keyStorePath_ + "/" + "mapping.txt";

  ofstream outFile;
  outFile.open(mappingFilePath.c_str(), ios_base::app);
  outFile << keyName << ' ' << keyFilePathNoExtension << endl;
  outFile.close();

  return keyFilePathNoExtension;
}

}
