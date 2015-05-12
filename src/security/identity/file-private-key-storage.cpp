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

#if 1
#include <stdexcept>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <algorithm>
#include <fstream>
#include "../../c/util/crypto.h"
#include "../../encoding/base64.hpp"
#include "../../encoding/der/der-node.hpp"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/file-private-key-storage.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;

static const char *WHITESPACE_CHARS = " \n\r\t";
static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";
static const char *EC_ENCRYPTION_OID = "1.2.840.10045.2.1";

static int CURVE_OID_224[] = { OBJ_secp224r1 };
static int CURVE_OID_256[] = { OBJ_X9_62_prime256v1 };
static int CURVE_OID_384[] = { OBJ_secp384r1 };
static int CURVE_OID_521[] = { OBJ_secp521r1 };

/**
 * The EcKeyInfo struct has fields used by the EC_KEY_INFO array to relate EC
 * key size and OIDs.
 */
struct EcKeyInfo {
  int keySize;
  int curveId;
  int *oidIntegerList;
  size_t oidIntegerListLength;
};

static struct EcKeyInfo EC_KEY_INFO[] {
  { 224, NID_secp224r1, CURVE_OID_224, sizeof(CURVE_OID_224) / sizeof(CURVE_OID_224[0]) },
  { 256, NID_X9_62_prime256v1, CURVE_OID_256, sizeof(CURVE_OID_256) / sizeof(CURVE_OID_256[0]) },
  { 384, NID_secp384r1, CURVE_OID_384, sizeof(CURVE_OID_384) / sizeof(CURVE_OID_384[0]) },
  { 521, NID_secp521r1, CURVE_OID_521, sizeof(CURVE_OID_521) / sizeof(CURVE_OID_521[0]) }
};

/**
 * Modify str in place to erase whitespace on the left.
 * @param str
 */
static __inline void
trimLeft(string& str)
{
  size_t found = str.find_first_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found > 0)
      str.erase(0, found);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the right.
 * @param str
 */
static __inline void
trimRight(string& str)
{
  size_t found = str.find_last_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found + 1 < str.size())
      str.erase(found + 1);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the left and right.
 * @param str
 */
static void
trim(string& str)
{
  trimLeft(str);
  trimRight(str);
}

FilePrivateKeyStorage::FilePrivateKeyStorage()
{
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

  if (params.getKeyType() == KEY_TYPE_RSA) {
    const RsaKeyParams& rsaParams = static_cast<const RsaKeyParams&>(params);

    BIGNUM* exponent = 0;
    RSA* rsa = 0;

    exponent = BN_new();
    if (BN_set_word(exponent, RSA_F4) == 1) {
      rsa = RSA_new();
      if (RSA_generate_key_ex(rsa, rsaParams.getKeySize(), exponent, NULL) == 1) {
        // Encode the public key.
        int length = i2d_RSA_PUBKEY(rsa, NULL);
        publicKeyDer = Blob(ptr_lib::make_shared<vector<uint8_t> >(length), false);
        uint8_t* derPointer = const_cast<uint8_t*>(publicKeyDer.buf());
        i2d_RSA_PUBKEY(rsa, &derPointer);

        // Encode the private key.
        length = i2d_RSAPrivateKey(rsa, NULL);
        vector<uint8_t> pkcs1PrivateKeyDer(length);
        derPointer = &pkcs1PrivateKeyDer[0];
        i2d_RSAPrivateKey(rsa, &derPointer);
        privateKeyDer = encodePkcs8PrivateKey
          (pkcs1PrivateKeyDer, OID(RSA_ENCRYPTION_OID),
           ptr_lib::make_shared<DerNode::DerNull>());
      }
    }

    BN_free(exponent);
    RSA_free(rsa);
  }
  else if (params.getKeyType() == KEY_TYPE_ECDSA) {
    const EcdsaKeyParams& ecdsaParams = static_cast<const EcdsaKeyParams&>(params);

    OID parametersOid;
    int curveId = -1;

    // Find the entry in EC_KEY_INFO.
    for (size_t i = 0 ; i < sizeof(EC_KEY_INFO) / sizeof(EC_KEY_INFO[0]); ++i) {
      if (EC_KEY_INFO[i].keySize == ecdsaParams.getKeySize()) {
        curveId = EC_KEY_INFO[i].curveId;
        parametersOid.setIntegerList
          (EC_KEY_INFO[i].oidIntegerList, EC_KEY_INFO[i].oidIntegerListLength);

        break;
      }
    }
    if (curveId == -1)
      throw SecurityException("Unsupported keySize for KEY_TYPE_ECDSA");

    EC_KEY* ecKey = EC_KEY_new_by_curve_name(curveId);
    if (ecKey != NULL) {
      if (EC_KEY_generate_key(ecKey) == 1) {
        // Encode the public key.
        int length = i2d_EC_PUBKEY(ecKey, NULL);
        vector<uint8_t> opensslPublicKeyDer(length);
        uint8_t* derPointer = &opensslPublicKeyDer[0];
        i2d_EC_PUBKEY(ecKey, &derPointer);
        // Convert the openssl style to ndn-cxx which has the simple AlgorithmIdentifier.
        // Find the bit string which is the second child.
        ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
          (&opensslPublicKeyDer[0], 0);
        const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
          parsedNode->getChildren();
        publicKeyDer = encodeSubjectPublicKeyInfo
          (OID(EC_ENCRYPTION_OID),
           ptr_lib::make_shared<DerNode::DerOid>(parametersOid), children[1]);

        // Encode the private key.
        EC_KEY_set_enc_flags(ecKey, EC_PKEY_NO_PARAMETERS | EC_PKEY_NO_PUBKEY);
        length = i2d_ECPrivateKey(ecKey, NULL);
        vector<uint8_t> pkcs1PrivateKeyDer(length);
        derPointer = &pkcs1PrivateKeyDer[0];
        i2d_ECPrivateKey(ecKey, &derPointer);
        privateKeyDer = encodePkcs8PrivateKey
          (pkcs1PrivateKeyDer, OID(EC_ENCRYPTION_OID),
           ptr_lib::make_shared<DerNode::DerOid>(parametersOid));
      }
    }

    EC_KEY_free(ecKey);
  }
  else
    throw SecurityException("Unsupported key type");

  string keyUri = keyName.toUri();
  string publicKeyFilePath = nameTransform(keyUri, ".pub");
  string privateKeyFilePath = nameTransform(keyUri, ".pri");

  ofstream publicKeyFile(publicKeyFilePath.c_str());
  publicKeyFile << toBase64(publicKeyDer.buf(), publicKeyDer.size(), true);
  ofstream privateKeyFile(privateKeyFilePath.c_str());
  privateKeyFile << toBase64(privateKeyDer.buf(), privateKeyDer.size(), true);

  ::chmod(publicKeyFilePath.c_str(),  S_IRUSR | S_IRGRP | S_IROTH);
  ::chmod(privateKeyFilePath.c_str(), S_IRUSR);
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

  // Get the digest to sign.
  uint8_t digest[SHA256_DIGEST_LENGTH];
  ndn_digestSha256(data, dataLength, digest);
  // TODO: use RSA_size, etc. to get the proper size of the signature buffer.
  uint8_t signatureBits[1000];
  unsigned int signatureBitsLength;

  // Decode the private key and sign.
  if (oidString == RSA_ENCRYPTION_OID) {
    // Use a temporary pointer since d2i updates it.
    const uint8_t* derPointer = privateKeyDer.buf();
    rsa_st* privateKey = d2i_RSAPrivateKey(NULL, &derPointer, privateKeyDer.size());
    if (!privateKey)
      throw SecurityException
        ("FilePrivateKeyStorage::sign: Error decoding the RSA private key DER");

    int success = RSA_sign
      (NID_sha256, digest, sizeof(digest), signatureBits, &signatureBitsLength,
       privateKey);
    // Free the private key before checking for success.
    RSA_free(privateKey);
    if (!success)
      throw SecurityException("FilePrivateKeyStorage::sign: Error in RSA_sign");
  }
  else if (oidString == EC_ENCRYPTION_OID) {
    ec_key_st* privateKey = decodeEcPrivateKey(algorithmParameters, privateKeyDer);
    int success = ECDSA_sign
      (NID_sha256, digest, sizeof(digest), signatureBits, &signatureBitsLength,
       privateKey);
    // Free the private key before checking for success.
    EC_KEY_free(privateKey);
    if (!success)
      throw SecurityException("FilePrivateKeyStorage::sign: Error in ECDSA_sign");
  }
  else
    throw SecurityException
      ("FilePrivateKeyStorage::sign: Unrecognized private key OID");

  return Blob(signatureBits, (size_t)signatureBitsLength);
}

Blob
FilePrivateKeyStorage::decrypt
  (const Name& keyName, const uint8_t* data, size_t dataLength,
   bool isSymmetric)
{
#if 1
  throw runtime_error("FilePrivateKeyStorage::decrypt not implemented");
#endif
}

Blob
FilePrivateKeyStorage::encrypt
  (const Name& keyName, const uint8_t* data, size_t dataLength,
   bool isSymmetric)
{
#if 1
  throw runtime_error("FilePrivateKeyStorage::encrypt not implemented");
#endif
}

void
FilePrivateKeyStorage::generateKey(const Name& keyName, const KeyParams& params)
{
#if 1
  throw runtime_error("FilePrivateKeyStorage::generateKey not implemented");
#endif
}

bool
FilePrivateKeyStorage::doesKeyExist(const Name& keyName, KeyClass keyClass)
{
  string keyURI = keyName.toUri();
  if (keyClass == KEY_CLASS_PUBLIC)
    return ::access(nameTransform(keyURI, ".pub").c_str(), R_OK) == 0;
  else if (keyClass == KEY_CLASS_PRIVATE)
    return ::access(nameTransform(keyURI, ".pri").c_str(), R_OK) == 0;
  else if (keyClass == KEY_CLASS_SYMMETRIC)
    return ::access(nameTransform(keyURI, ".key").c_str(), R_OK) == 0;
  else
    return false;
}

string
FilePrivateKeyStorage::nameTransform
  (const string& keyName, const string& extension)
{
  uint8_t hash[SHA256_DIGEST_LENGTH];
  ndn_digestSha256((uint8_t*)&keyName[0], keyName.size(), hash);

  string digest = toBase64(hash, sizeof(hash));
  trim(digest);
  std::replace(digest.begin(), digest.end(), '/', '%');

  return keyStorePath_ + "/" + digest + extension;
}

Blob
FilePrivateKeyStorage::encodePkcs8PrivateKey
  (const vector<uint8_t>& privateKeyDer, const OID& oid,
   const ptr_lib::shared_ptr<DerNode>& parameters)
{
  ptr_lib::shared_ptr<DerSequence> algorithmIdentifier(new DerSequence());
  algorithmIdentifier->addChild(ptr_lib::make_shared<DerNode::DerOid>(oid));
  algorithmIdentifier->addChild(parameters);

  DerSequence result;
  result.addChild(ptr_lib::make_shared<DerNode::DerInteger>(0));
  result.addChild(algorithmIdentifier);
  result.addChild(ptr_lib::make_shared<DerNode::DerOctetString>
    (&privateKeyDer[0], privateKeyDer.size()));

  return result.encode();
}

Blob
FilePrivateKeyStorage::encodeSubjectPublicKeyInfo
  (const OID& oid, const ptr_lib::shared_ptr<DerNode>& parameters,
   const ptr_lib::shared_ptr<DerNode>& bitString)
{
  ptr_lib::shared_ptr<DerSequence> algorithmIdentifier(new DerSequence());
  algorithmIdentifier->addChild(ptr_lib::make_shared<DerNode::DerOid>(oid));
  algorithmIdentifier->addChild(parameters);

  DerSequence result;
  result.addChild(algorithmIdentifier);
  result.addChild(bitString);

  return result.encode();
}

ec_key_st*
FilePrivateKeyStorage::decodeEcPrivateKey
  (const ptr_lib::shared_ptr<DerNode>& algorithmParameters,
   const Blob& privateKeyDer)
{
  // Find the curveId in EC_KEY_INFO.
  int curveId = -1;
  string oidString = algorithmParameters->toVal().toRawStr();
  for (size_t i = 0 ; i < sizeof(EC_KEY_INFO) / sizeof(EC_KEY_INFO[0]); ++i) {
    OID curveOid(EC_KEY_INFO[i].oidIntegerList, EC_KEY_INFO[i].oidIntegerListLength);
    if (curveOid.toString() == oidString) {
      curveId = EC_KEY_INFO[i].curveId;
      break;
    }
  }
  if (curveId == -1)
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Unrecognized EC algorithm parameters");

  // Get the value in the octet string.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(privateKeyDer.buf(), 0);
  DerNode::DerOctetString* octetString = dynamic_cast<DerNode::DerOctetString*>
    (parsedNode->getChildren()[1].get());
  if (!octetString)
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Can't get the private key octet string");
  Blob octetStringValue = octetString->toVal();

  BIGNUM* keyBignum = BN_bin2bn(octetStringValue.buf(), octetStringValue.size(), NULL);
  if (!keyBignum) {
    // We don't expect this to happen.
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Can't create a BIGNUM for the private key value");
  }
  EC_KEY* privateKey = EC_KEY_new_by_curve_name(curveId);
  if (!privateKey) {
    // We don't expect this to happen.
    BN_free(keyBignum);
    throw SecurityException
      ("FilePrivateKeyStorage::decodeEcPrivateKey: Can't create an EC key for the curve ID");
  }
  EC_KEY_set_private_key(privateKey, keyBignum);
  BN_free(keyBignum);

  return privateKey;
}

}
