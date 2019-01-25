/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/transform/private-key.cpp
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

#include "../../encoding/der/der-node.hpp"
#include "../../encoding/der/der-exception.hpp"
#include "../../c/util/crypto.h"
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/encrypt/algo/des-algorithm-lite.hpp>
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>

using namespace std;

namespace ndn {

typedef DerNode::DerSequence DerSequence;

static const char *RSA_ENCRYPTION_OID = "1.2.840.113549.1.1.1";
static const char *EC_ENCRYPTION_OID = "1.2.840.10045.2.1";
static const char *PBES2_OID = "1.2.840.113549.1.5.13";
static const char *PBKDF2_OID = "1.2.840.113549.1.5.12";
static const char *DES_EDE3_CBC_OID = "1.2.840.113549.3.7";

void
TpmPrivateKey::loadPkcs1
  (const uint8_t* encoding, size_t encodingLength, KeyType keyType)
{
  ndn_Error error;

  if (keyType == (KeyType)-1) {
    // Try to determine the key type.
    try {
      ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
        (encoding, encodingLength);
      const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
        parsedNode->getChildren();

      // An RsaPrivateKey has integer version 0 and 8 integers.
      if (children.size() == 9 &&
          dynamic_cast<DerNode::DerInteger*>(children[0].get()) &&
          dynamic_cast<DerNode::DerInteger&>(*children[0]).toIntegerVal() == 0 &&
          dynamic_cast<DerNode::DerInteger*>(children[1].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[2].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[3].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[4].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[5].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[6].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[7].get()) &&
          dynamic_cast<DerNode::DerInteger*>(children[8].get()))
        keyType = KEY_TYPE_RSA;
      else
        // Assume it is an EC key. Try decoding it below.
        keyType = KEY_TYPE_EC;
    } catch (const DerDecodingException& ex) {
      // Assume it is an EC key. Try decoding it below.
      keyType = KEY_TYPE_EC;
    }
  }

  keyType_ = (KeyType)-1;
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType == KEY_TYPE_EC) {
    ecPrivateKey_.reset(new EcPrivateKeyLite());
    if ((error = ecPrivateKey_->decode(encoding, encodingLength)))
      throw Error
        (string("Error decoding PKCS #1 key: ") + ndn_getErrorString(error));
  }
  else if (keyType == KEY_TYPE_RSA) {
    rsaPrivateKey_.reset(new RsaPrivateKeyLite());
    if ((error = rsaPrivateKey_->decode(encoding, encodingLength)))
      throw Error
        (string("Error decoding PKCS #1 key: ") + ndn_getErrorString(error));
  }
  else
#endif
    throw Error("loadPkcs1: Unrecognized keyType");

  // Successfully decoded, so set the keyType_.
  keyType_ = keyType;
}

void
TpmPrivateKey::loadPkcs8(const uint8_t* encoding, size_t encodingLength)
{
  string oidString;
  ptr_lib::shared_ptr<DerNode> algorithmParameters;
  Blob privateKeyDer;

  try {
    // Decode to find the algorithm OID and the inner private key DER.
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (encoding, encodingLength);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& pkcs8Children =
      parsedNode->getChildren();

    // Get the algorithm OID and parameters.
    const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
      DerNode::getSequence(pkcs8Children, 1).getChildren();
    oidString =
      dynamic_cast<DerNode::DerOid&>(*algorithmIdChildren[0]).toVal().toRawStr();
    algorithmParameters = algorithmIdChildren[1];

    // Get the value of the 3rd child which is the octet string.
    privateKeyDer = pkcs8Children[2]->toVal();
  } catch (const DerDecodingException&) {
    // Error decoding as PKCS #8. Try PKCS #1 for backwards compatibility.
    try {
      loadPkcs1(encoding, encodingLength);
      return;
    } catch (const std::exception& ex) {
      throw Error(string("loadPkcs8: Error decoding private key: ") + ex.what());
    }
  }

  keyType_ = (KeyType)-1;
#if NDN_CPP_HAVE_LIBCRYPTO
  if (oidString == EC_ENCRYPTION_OID) {
    ecPrivateKey_.reset(new EcPrivateKeyLite());
    decodeEcPrivateKey(algorithmParameters, privateKeyDer, *ecPrivateKey_);

    // Successfully decoded, so set the keyType_.
    keyType_ = KEY_TYPE_EC;
  }
  else if (oidString == RSA_ENCRYPTION_OID) {
    rsaPrivateKey_.reset(new RsaPrivateKeyLite());
    ndn_Error error;
    if ((error = rsaPrivateKey_->decode(privateKeyDer)))
      throw Error(string("loadPkcs8: ") + ndn_getErrorString(error));

    // Successfully decoded, so set the keyType_.
    keyType_ = KEY_TYPE_RSA;
  }
  else
#endif
    throw Error("loadPkcs8: Unrecognized private key OID");
}

void
TpmPrivateKey::loadEncryptedPkcs8
  (const uint8_t* encoding, size_t encodingLength, const uint8_t* password,
   size_t passwordLength)
{
#if NDN_CPP_HAVE_LIBCRYPTO
  // Decode the PKCS #8 EncryptedPrivateKeyInfo.
  // See https://tools.ietf.org/html/rfc5208.
  string oidString;
  ptr_lib::shared_ptr<DerNode> parameters;
  Blob encryptedKey;
  try {
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (encoding, encodingLength);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& encryptedPkcs8Children =
      parsedNode->getChildren();
    const std::vector<ptr_lib::shared_ptr<DerNode> >& algorithmIdChildren =
      DerNode::getSequence(encryptedPkcs8Children, 0).getChildren();
    oidString =
      dynamic_cast<DerNode::DerOid&>(*algorithmIdChildren[0]).toVal().toRawStr();
    parameters = algorithmIdChildren[1];

    encryptedKey = dynamic_cast<DerNode::DerOctetString*>
      (encryptedPkcs8Children[1].get())->toVal();
  } catch (const std::exception& ex) {
    throw Error(string
      ("Cannot decode the PKCS #8 EncryptedPrivateKeyInfo: ") + ex.what());
  }

  // Use the password to get the unencrypted pkcs8Encoding.
  vector<uint8_t> pkcs8Encoding(encryptedKey.size());
  size_t pkcs8EncodingLength;

  if (oidString == PBES2_OID) {
    // Decode the PBES2 parameters. See https://www.ietf.org/rfc/rfc2898.txt .
    string keyDerivationOidString;
    ptr_lib::shared_ptr<DerNode> keyDerivationParameters;
    string encryptionSchemeOidString;
    ptr_lib::shared_ptr<DerNode> encryptionSchemeParameters;
    try {
      const std::vector<ptr_lib::shared_ptr<DerNode> >& parametersChildren =
        parameters->getChildren();

      const std::vector<ptr_lib::shared_ptr<DerNode> >& keyDerivationAlgorithmIdChildren =
        DerNode::getSequence(parametersChildren, 0).getChildren();
      keyDerivationOidString = dynamic_cast<DerNode::DerOid&>
        (*keyDerivationAlgorithmIdChildren[0]).toVal().toRawStr();
      keyDerivationParameters = keyDerivationAlgorithmIdChildren[1];

      const std::vector<ptr_lib::shared_ptr<DerNode> >& encryptionSchemeAlgorithmIdChildren =
        DerNode::getSequence(parametersChildren, 1).getChildren();
      encryptionSchemeOidString = dynamic_cast<DerNode::DerOid&>
        (*encryptionSchemeAlgorithmIdChildren[0]).toVal().toRawStr();
      encryptionSchemeParameters = encryptionSchemeAlgorithmIdChildren[1];
    } catch (const std::exception& ex) {
      throw Error(string
        ("Cannot decode the PBES2 parameters: ") + ex.what());
    }

    // Get the derived key from the password.
    vector<uint8_t> derivedKey;
    if (keyDerivationOidString == PBKDF2_OID) {
      // Decode the PBKDF2 parameters.
      Blob salt;
      int nIterations;
      try {
        const std::vector<ptr_lib::shared_ptr<DerNode> >& pbkdf2ParametersChildren =
          dynamic_cast<DerNode::DerSequence&>(*keyDerivationParameters).getChildren();
        salt = dynamic_cast<DerNode::DerOctetString*>
          (pbkdf2ParametersChildren[0].get())->toVal();
        nIterations = dynamic_cast<DerNode::DerInteger*>
          (pbkdf2ParametersChildren[1].get())->toIntegerVal();
      } catch (const std::exception& ex) {
        throw Error(string
          ("Cannot decode the PBES2 parameters: ") + ex.what());
      }

      // Check the encryption scheme here to get the needed result length.
      int resultLength;
      if (encryptionSchemeOidString == DES_EDE3_CBC_OID)
        resultLength = ndn_DES_EDE3_KEY_LENGTH;
      else
        throw Error("Unrecognized PBES2 encryption scheme OID: " +
          encryptionSchemeOidString);

      derivedKey.resize(resultLength);
      CryptoLite::computePbkdf2WithHmacSha1
        (password, passwordLength, salt.buf(), salt.size(), nIterations,
         resultLength, &derivedKey.front());
    }
    else
      throw Error
        ("Unrecognized PBES2 key derivation OID: " + keyDerivationOidString);

    // Use the derived key to get the unencrypted pkcs8Encoding.
    if (encryptionSchemeOidString == DES_EDE3_CBC_OID) {
      // Decode the DES-EDE3-CBC parameters.
      Blob initialVector;
      try {
        initialVector = dynamic_cast<DerNode::DerOctetString*>
          (encryptionSchemeParameters.get())->toVal();
      } catch (const std::exception& ex) {
        throw Error(string
          ("Cannot decode the DES-EDE3-CBC parameters: ") + ex.what());
      }

      ndn_Error error;
      if ((error = DesAlgorithmLite::decryptEdeCbcPkcs5Padding
           (&derivedKey.front(), derivedKey.size(), initialVector.buf(),
            initialVector.size(),
            encryptedKey.buf(), encryptedKey.size(), &pkcs8Encoding.front(),
            pkcs8EncodingLength)))
        throw Error(string("Error decrypting PKCS #8 key with DES-EDE3-CBC: ") +
                    ndn_getErrorString(error));
    }
    else
      throw Error("Unrecognized PBES2 encryption scheme OID: " +
        encryptionSchemeOidString);
  }
  else
    throw Error("Unrecognized PKCS #8 EncryptedPrivateKeyInfo OID: " + oidString);

  loadPkcs8(&pkcs8Encoding.front(), pkcs8Encoding.size());
#else
  throw Error
    ("loadEncryptedPkcs8: Password-based decrypting the private key is not supported");
#endif
}

Blob
TpmPrivateKey::derivePublicKey() const
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ == KEY_TYPE_EC) {
    // Get the encoding length and encode the public key.
    size_t encodingLength;
    ndn_Error error;
    if ((error = ecPrivateKey_->encodePublicKey(true, 0, encodingLength)))
      throw Error(string("generate: ") + ndn_getErrorString(error));
    vector<uint8_t> opensslPublicKeyDer(encodingLength);
    if ((error = ecPrivateKey_->encodePublicKey
         (true, &opensslPublicKeyDer[0], encodingLength)))
      throw Error(string("generate: ") + ndn_getErrorString(error));
    // Convert the openssl style to ndn-cxx which has the simple
    // AlgorithmIdentifier.
    // Find the bit string which is the second child.
    ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse
      (opensslPublicKeyDer);
    const std::vector<ptr_lib::shared_ptr<DerNode> >& children =
      parsedNode->getChildren();
    OID parametersOid = getEcOid(*ecPrivateKey_);
    return encodeSubjectPublicKeyInfo
      (OID(EC_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerOid>(parametersOid), children[1]);
  }
  else if (keyType_ == KEY_TYPE_RSA) {
    // Get the encoding length and then encode.
    size_t encodingLength;
    ndn_Error error;
    if ((error = rsaPrivateKey_->encodePublicKey(0, encodingLength)))
      throw Error
        (string("Error encoding public key: ") + ndn_getErrorString(error));

    // Put this vector in a shared_ptr so that the Blob constructor doesn't copy.
    ptr_lib::shared_ptr<vector<uint8_t> > encoding(new vector<uint8_t>());
    encoding->resize(encodingLength);
    if ((error = rsaPrivateKey_->encodePublicKey(&encoding->front(), encodingLength)))
      throw Error
        (string("Error encoding public key: ") + ndn_getErrorString(error));

    return Blob(encoding, false);
  }
  else
#endif
    throw Error("derivePublicKey: The private key is not loaded");
}

Blob
TpmPrivateKey::decrypt
  (const uint8_t* cipherText, size_t cipherTextLength,
   ndn_EncryptAlgorithmType algorithmType)
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ == KEY_TYPE_EC)
    throw Error("Decryption is not supported for EC keys");
  else if (keyType_ == KEY_TYPE_RSA) {
    // TODO: use RSA_size, etc. to get the proper size of the output buffer.
    ptr_lib::shared_ptr<vector<uint8_t> > plainData(new vector<uint8_t>(1000));
    size_t plainDataLength;
    ndn_Error error;
    if ((error = rsaPrivateKey_->decrypt
         (cipherText, cipherTextLength, algorithmType, &plainData->front(),
          plainDataLength))) {
      if (error == NDN_ERROR_Unsupported_algorithm_type)
        throw Error("RSA decrypt: Unsupported padding scheme");
      else
        throw Error(string("RSA decrypt: ") + ndn_getErrorString(error));
    }

    plainData->resize(plainDataLength);
    return Blob(plainData, false);
  }
  else
#endif
    throw Error("decrypt: The private key is not loaded");
}

Blob
TpmPrivateKey::sign
  (const uint8_t *data, size_t dataLength, DigestAlgorithm digestAlgorithm)
{
  if (digestAlgorithm != DIGEST_ALGORITHM_SHA256)
    throw Error("TpmPrivateKey::sign: Unsupported digest algorithm");

  // TODO: use RSA_size to get the proper size of the signature buffer.
  uint8_t signatureBits[1000];
  size_t signatureBitsLength;
  ndn_Error error;

#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ == KEY_TYPE_EC) {
    if ((error = ecPrivateKey_->signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw Error
        (string("Error signing with private key: ") + ndn_getErrorString(error));
  }
  else if (keyType_ == KEY_TYPE_RSA) {
    if ((error = rsaPrivateKey_->signWithSha256
         (data, dataLength, signatureBits, signatureBitsLength)))
      throw Error
        (string("Error signing with private key: ") + ndn_getErrorString(error));
  }
  else
#endif
    return Blob();

  return Blob(signatureBits, (size_t)signatureBitsLength);
}

Blob
TpmPrivateKey::toPkcs1(bool includeParameters)
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ == KEY_TYPE_EC) {
    // Get the encoding length and encode.
    size_t encodingLength;
    ndn_Error error;
    if ((error = ecPrivateKey_->encodePrivateKey
         (includeParameters, 0, encodingLength)))
      throw Error(string("toPkcs1: ") + ndn_getErrorString(error));
    ptr_lib::shared_ptr<vector<uint8_t> > encoding
      (new vector<uint8_t>(encodingLength));
    if ((error = ecPrivateKey_->encodePrivateKey
         (includeParameters, &encoding->front(), encodingLength)))
      throw Error(string("toPkcs1: ") + ndn_getErrorString(error));
    return Blob(encoding, false);
  }
  else if (keyType_ == KEY_TYPE_RSA) {
    // Get the encoding length and encode the private key.
    size_t encodingLength;
    ndn_Error error;
    if ((error = rsaPrivateKey_->encodePrivateKey(0, encodingLength)))
      throw Error(string("toPkcs8: ") + ndn_getErrorString(error));
    ptr_lib::shared_ptr<vector<uint8_t> > encoding
      (new vector<uint8_t>(encodingLength));
    if ((error = rsaPrivateKey_->encodePrivateKey
         (&encoding->front(), encodingLength)))
      throw Error(string("toPkcs8: ") + ndn_getErrorString(error));
    return Blob(encoding, false);
  }
  else
#endif
    throw Error("toPkcs1: The private key is not loaded");
}

Blob
TpmPrivateKey::toPkcs8(bool includeParameters)
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ == KEY_TYPE_EC) {
    OID parametersOid = getEcOid(*ecPrivateKey_);
    return encodePkcs8PrivateKey
      (*toPkcs1(includeParameters), OID(EC_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerOid>(parametersOid));
  }
  else if (keyType_ == KEY_TYPE_RSA) {
    return encodePkcs8PrivateKey
      (*toPkcs1(), OID(RSA_ENCRYPTION_OID),
       ptr_lib::make_shared<DerNode::DerNull>());
  }
  else
#endif
    throw Error("toPkcs8: The private key is not loaded");
}

Blob
TpmPrivateKey::toEncryptedPkcs8
  (const uint8_t* password, size_t passwordLength, bool includeParameters)
{
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyType_ < 0)
    throw Error("toEncryptedPkcs8: The private key is not loaded");

  // Create the derivedKey from the password.
  const int nIterations = 2048;
  uint8_t salt[8];
  CryptoLite::generateRandomBytes(salt, sizeof(salt));
  uint8_t derivedKey[ndn_DES_EDE3_KEY_LENGTH];
  CryptoLite::computePbkdf2WithHmacSha1
    (password, passwordLength, salt, sizeof(salt), nIterations,
     ndn_DES_EDE3_KEY_LENGTH, derivedKey);

  Blob pkcs8Encoding = toPkcs8(includeParameters);

  // Use the derived key to get the encrypted pkcs8Encoding.
  ptr_lib::shared_ptr<vector<uint8_t> > encryptedEncoding
    (new vector<uint8_t>(pkcs8Encoding.size() + ndn_DES_BLOCK_LENGTH));
  uint8_t initialVector[ndn_DES_BLOCK_LENGTH];
  CryptoLite::generateRandomBytes(initialVector, sizeof(initialVector));

  ndn_Error error;
  size_t encryptedEncodingLength;
  if ((error = DesAlgorithmLite::encryptEdeCbcPkcs5Padding
       (derivedKey, sizeof(derivedKey), initialVector, sizeof(initialVector),
        pkcs8Encoding.buf(), pkcs8Encoding.size(), &encryptedEncoding->front(),
        encryptedEncodingLength)))
    throw Error(string("Error encrypting PKCS #8 key with DES-EDE3-CBC: ") +
                ndn_getErrorString(error));

  // Encode the PBES2 parameters. See https://www.ietf.org/rfc/rfc2898.txt .
  ptr_lib::shared_ptr<DerSequence> keyDerivationParameters(new DerSequence());
  keyDerivationParameters->addChild(ptr_lib::make_shared<DerNode::DerOctetString>
    (salt, sizeof(salt)));
  keyDerivationParameters->addChild(ptr_lib::make_shared<DerNode::DerInteger>
    (nIterations));
  ptr_lib::shared_ptr<DerSequence> keyDerivationAlgorithmIdentifier
    (new DerSequence());
  keyDerivationAlgorithmIdentifier->addChild
    (ptr_lib::make_shared<DerNode::DerOid>(PBKDF2_OID));
  keyDerivationAlgorithmIdentifier->addChild(keyDerivationParameters);

  ptr_lib::shared_ptr<DerSequence> encryptionSchemeAlgorithmIdentifier
    (new DerSequence());
  encryptionSchemeAlgorithmIdentifier->addChild
    (ptr_lib::make_shared<DerNode::DerOid>(DES_EDE3_CBC_OID));
  encryptionSchemeAlgorithmIdentifier->addChild
    (ptr_lib::make_shared<DerNode::DerOctetString>
     (initialVector, sizeof(initialVector)));

  ptr_lib::shared_ptr<DerSequence> encryptedKeyParameters(new DerSequence());
  encryptedKeyParameters->addChild(keyDerivationAlgorithmIdentifier);
  encryptedKeyParameters->addChild(encryptionSchemeAlgorithmIdentifier);
  ptr_lib::shared_ptr<DerSequence> encryptedKeyAlgorithmIdentifier
    (new DerSequence());
  encryptedKeyAlgorithmIdentifier->addChild
    (ptr_lib::make_shared<DerNode::DerOid>(PBES2_OID));
  encryptedKeyAlgorithmIdentifier->addChild(encryptedKeyParameters);

  // Encode the PKCS #8 EncryptedPrivateKeyInfo.
  // See https://tools.ietf.org/html/rfc5208.
  ptr_lib::shared_ptr<DerSequence> encryptedKey(new DerSequence());
  encryptedKey->addChild(encryptedKeyAlgorithmIdentifier);
  encryptedKey->addChild(ptr_lib::make_shared<DerNode::DerOctetString>
    (&encryptedEncoding->front(), encryptedEncodingLength));

  return encryptedKey->encode();
#else
  throw Error
    ("toEncryptedPkcs8: Password-based encrypting the private key is not supported");
#endif
}

ptr_lib::shared_ptr<TpmPrivateKey>
TpmPrivateKey::generatePrivateKey(const KeyParams& keyParams)
{
  ptr_lib::shared_ptr<TpmPrivateKey> result(new TpmPrivateKey());
#if NDN_CPP_HAVE_LIBCRYPTO
  if (keyParams.getKeyType() == KEY_TYPE_EC) {
    const EcKeyParams& ecParams = static_cast<const EcKeyParams&>(keyParams);
    result->ecPrivateKey_.reset(new EcPrivateKeyLite());

    ndn_Error error;
    if ((error = result->ecPrivateKey_->generate(ecParams.getKeySize())))
      throw Error(string("generate: ") + ndn_getErrorString(error));

    result->keyType_ = KEY_TYPE_EC;
  }
  else if (keyParams.getKeyType() == KEY_TYPE_RSA) {
    const RsaKeyParams& rsaParams = static_cast<const RsaKeyParams&>(keyParams);

    result->rsaPrivateKey_.reset(new RsaPrivateKeyLite());
    ndn_Error error;
    if ((error = result->rsaPrivateKey_->generate(rsaParams.getKeySize())))
      throw Error(string("generate: ") + ndn_getErrorString(error));

    result->keyType_ = KEY_TYPE_RSA;
  }
  else
#endif
    throw invalid_argument("Unsupported key type");

  return result;
}

Blob
TpmPrivateKey::encodePkcs8PrivateKey
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
TpmPrivateKey::encodeSubjectPublicKeyInfo
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

#if NDN_CPP_HAVE_LIBCRYPTO
void
TpmPrivateKey::decodeEcPrivateKey
  (const ptr_lib::shared_ptr<DerNode>& algorithmParameters,
   const Blob& privateKeyDer, EcPrivateKeyLite& privateKey)
{
  // Find the curveId in EC_KEY_INFO.
  int curveId = -1;
  string oidString = algorithmParameters->toVal().toRawStr();
  for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
    const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
    OID curveOid(info->oidIntegerList, info->oidIntegerListLength);
    if (curveOid.toString() == oidString) {
      curveId = info->curveId;
      break;
    }
  }
  if (curveId == -1)
    throw Error("decodeEcPrivateKey: Unrecognized EC algorithm parameters");

  // Get the value in the octet string.
  ptr_lib::shared_ptr<DerNode> parsedNode = DerNode::parse(privateKeyDer);
  DerNode::DerOctetString* octetString = dynamic_cast<DerNode::DerOctetString*>
    (parsedNode->getChildren()[1].get());
  if (!octetString)
    throw Error("decodeEcPrivateKey: Can't get the private key octet string");
  Blob octetStringValue = octetString->toVal();

  ndn_Error error;
  if ((error = privateKey.setByCurve(curveId, octetStringValue)))
    throw Error(string("decodeEcPrivateKey ") + ndn_getErrorString(error));
}
#endif

OID
TpmPrivateKey::getEcOid(const EcPrivateKeyLite& ecPrivateKey)
{
  int curveId;
  ndn_Error error;
  if ((error = ecPrivateKey.getCurveId(curveId)))
    throw Error(string("Error getting EC curye ID: ") + ndn_getErrorString(error));

  OID oid;
  for (size_t i = 0 ; i < ndn_getEcKeyInfoCount(); ++i) {
    const struct ndn_EcKeyInfo *info = ndn_getEcKeyInfo(i);
    if (info->curveId == curveId) {
      oid.setIntegerList
        (info->oidIntegerList, info->oidIntegerListLength);
      return oid;
    }
  }

  throw Error("Unsupported keySize for KEY_TYPE_EC");
}

}
