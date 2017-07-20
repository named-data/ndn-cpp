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
#include "../../c/util/crypto.h"
#include "../../encoding/base64.hpp"
#include "../../encoding/der/der-node.hpp"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/security/ec-private-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-private-key-lite.hpp>
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

  Blob publicKeyDer;
  Blob privateKeyDer;

#if NDN_CPP_HAVE_LIBCRYPTO
  if (params.getKeyType() == KEY_TYPE_RSA) {
    const RsaKeyParams& rsaParams = static_cast<const RsaKeyParams&>(params);
    RsaPrivateKeyLite privateKey;
    ndn_Error error;
    if ((error = privateKey.generate(rsaParams.getKeySize())))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));

    // Get the encoding length and encode the public key.
    size_t encodingLength;
    if ((error = privateKey.encodePublicKey(0, encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    publicKeyDer = Blob
      (ptr_lib::make_shared<vector<uint8_t> >(encodingLength), false);
    if ((error = privateKey.encodePublicKey
         (const_cast<uint8_t*>(publicKeyDer.buf()), encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));

    // Get the encoding length and encode the private key.
    if ((error = privateKey.encodePrivateKey(0, encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    vector<uint8_t> pkcs1PrivateKeyDer(encodingLength);
    if ((error = privateKey.encodePrivateKey
         (&pkcs1PrivateKeyDer[0], encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    privateKeyDer = encodePkcs8PrivateKey
      (pkcs1PrivateKeyDer, OID(RSA_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerNull>());
  }
  else if (params.getKeyType() == KEY_TYPE_ECDSA) {
    const EcdsaKeyParams& ecdsaParams = static_cast<const EcdsaKeyParams&>(params);

    EcPrivateKeyLite privateKey;
    ndn_Error error;
    if ((error = privateKey.generate(ecdsaParams.getKeySize())))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));

    // Find the entry in EC_KEY_INFO.
    OID parametersOid;
    for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
      const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
      if (info->keySize == ecdsaParams.getKeySize()) {
        parametersOid.setIntegerList
          (info->oidIntegerList, info->oidIntegerListLength);

        break;
      }
    }
    if (parametersOid.getIntegerList().size() == 0)
      // We don't expect this to happen since generate succeeded.
      throw SecurityException("Unsupported keySize for KEY_TYPE_ECDSA");

    // Get the encoding length and encode the public key.
    size_t encodingLength;
    if ((error = privateKey.encodePublicKey(true, 0, encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    vector<uint8_t> opensslPublicKeyDer(encodingLength);
    if ((error = privateKey.encodePublicKey
         (true, &opensslPublicKeyDer[0], encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    // Convert the openssl style to ndn-cxx which has the simple AlgorithmIdentifier.
    // Find the bit string which is the second child.
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (&opensslPublicKeyDer[0], 0);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
      parsedNode->getChildren();
    publicKeyDer = encodeSubjectPublicKeyInfo
      (OID(EC_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerOid>(parametersOid), children[1]);

    // Get the encoding length and encode the private key. Omit the EC parameters
    // since ndn-cxx doesn't use them.
    if ((error = privateKey.encodePrivateKey(false, 0, encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    vector<uint8_t> pkcs1PrivateKeyDer(encodingLength);
    if ((error = privateKey.encodePrivateKey
         (false, &pkcs1PrivateKeyDer[0], encodingLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage: ") + ndn_getErrorString(error));
    privateKeyDer = encodePkcs8PrivateKey
      (pkcs1PrivateKeyDer, OID(EC_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerOid>(parametersOid));
  }
  else
#endif
    throw SecurityException("Unsupported key type");

  string keyUri = keyName.toUri();
  string keyFilePathNoExtension = maintainMapping(keyUri);
  string publicKeyFilePath = keyFilePathNoExtension + ".pub";
  string privateKeyFilePath = keyFilePathNoExtension + ".pri";

  ofstream publicKeyFile(publicKeyFilePath.c_str());
  publicKeyFile << toBase64(publicKeyDer.buf(), publicKeyDer.size(), true);
  ofstream privateKeyFile(privateKeyFilePath.c_str());
  privateKeyFile << toBase64(privateKeyDer.buf(), privateKeyDer.size(), true);

#if !defined(_WIN32) // Windows doesn't have Unix group permissions.
  ::chmod(publicKeyFilePath.c_str(),  S_IRUSR | S_IRGRP | S_IROTH);
  ::chmod(privateKeyFilePath.c_str(), S_IRUSR);
#endif
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

  if (digestAlgorithm != DIGEST_ALGORITHM_SHA256)
    throw SecurityException
      ("FilePrivateKeyStorage::sign: Unsupported digest algorithm");

  // Read the private key.
  ifstream file(nameTransform(keyURI, ".pri").c_str());
  stringstream base64;
  base64 << file.rdbuf();
  vector<uint8_t> pkcs8Der;
  fromBase64(base64.str(), pkcs8Der);

  // The private key is generated by NFD which stores as PKCS #8. Decode it
  // to find the algorithm OID and the inner private key DER.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(&pkcs8Der[0], 0);
  const std::vector<ptr_lib::shared_ptr<DerNode> >& pkcs8Children =
    parsedNode->getChildren();
  // Get the algorithm OID and parameters.
  const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
    DerNode::getSequence(pkcs8Children, 1).getChildren();
  string oidString
    (dynamic_cast<DerNode::DerOid&>(*algorithmIdChildren[0]).toVal().toRawStr());
  ptr_lib::shared_ptr<DerNode> algorithmParameters = algorithmIdChildren[1];
  // Get the value of the 3rd child which is the octet string.
  Blob privateKeyDer = pkcs8Children[2]->toVal();

  // TODO: use RSA_size, etc. to get the proper size of the signature buffer.
  uint8_t signatureBits[1000];
  size_t signatureBitsLength;
  ndn_Error error;

  // Decode the private key and sign.
#if NDN_CPP_HAVE_LIBCRYPTO
  if (oidString == RSA_ENCRYPTION_OID) {
    RsaPrivateKeyLite privateKey;
    if ((error = privateKey.decode(privateKeyDer)))
      throw SecurityException
        (string("FilePrivateKeyStorage::sign RSA: ") + ndn_getErrorString(error));

    if ((error =  privateKey.signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage::sign RSA: ") + ndn_getErrorString(error));
  }
  else if (oidString == EC_ENCRYPTION_OID) {
    EcPrivateKeyLite privateKey;
    decodeEcPrivateKey(algorithmParameters, privateKeyDer, privateKey);

    if ((error =  privateKey.signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw SecurityException
        (string("FilePrivateKeyStorage::sign ECDSA: ") + ndn_getErrorString(error));
  }
  else
#endif
    throw SecurityException
      ("FilePrivateKeyStorage::sign: Unrecognized private key OID");

  return Blob(signatureBits, signatureBitsLength);
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
