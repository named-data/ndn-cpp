/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_TIME_H
#include <time.h>
#endif
#if NDN_CPP_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <ctime>
#include <fstream>
#include <math.h>
#include <ndn-cpp/key-locator.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/basic-identity-storage.hpp>
#include <ndn-cpp/security/identity/file-private-key-storage.hpp>
#include <ndn-cpp/security/identity/osx-private-key-storage.hpp>
#include "../../c/util/time.h"
#include "../../c/util/crypto.h"
#include <ndn-cpp/security/identity/identity-manager.hpp>

using namespace std;

namespace ndn {

IdentityManager::IdentityManager
  (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage,
   const ptr_lib::shared_ptr<PrivateKeyStorage>& privateKeyStorage)
: identityStorage_(identityStorage), privateKeyStorage_(privateKeyStorage)
{
}

#ifdef NDN_CPP_HAVE_SQLITE3

#if NDN_CPP_HAVE_OSX_SECURITY && NDN_CPP_WITH_OSX_KEYCHAIN
IdentityManager::IdentityManager
  (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage)
: identityStorage_(identityStorage),
  privateKeyStorage_(ptr_lib::make_shared<OSXPrivateKeyStorage>())
{
}

IdentityManager::IdentityManager()
: identityStorage_(ptr_lib::make_shared<BasicIdentityStorage>()),
  privateKeyStorage_(ptr_lib::make_shared<OSXPrivateKeyStorage>())
{
}
#else
IdentityManager::IdentityManager
  (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage)
: identityStorage_(identityStorage),
  privateKeyStorage_(ptr_lib::make_shared<FilePrivateKeyStorage>())
{
}

IdentityManager::IdentityManager()
: identityStorage_(ptr_lib::make_shared<BasicIdentityStorage>()),
  privateKeyStorage_(ptr_lib::make_shared<FilePrivateKeyStorage>())
{
}
#endif

#else
// No SQLite, so we can't use BasicIdentityStorage.

#if NDN_CPP_HAVE_OSX_SECURITY && NDN_CPP_WITH_OSX_KEYCHAIN
IdentityManager::IdentityManager
  (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage)
: identityStorage_(identityStorage),
  privateKeyStorage_(ptr_lib::make_shared<OSXPrivateKeyStorage>())
{
}

IdentityManager::IdentityManager()
{
  throw SecurityException
    ("Can't create the default IdentityManager without an identityStorage. Try installing libsqlite3 and ./configure again.");
}
#else
IdentityManager::IdentityManager
  (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage)
: identityStorage_(identityStorage),
  privateKeyStorage_(ptr_lib::make_shared<FilePrivateKeyStorage>())
{
}

IdentityManager::IdentityManager()
{
  throw SecurityException
    ("Can't create the default IdentityManager without an identityStorage. Try installing libsqlite3 and ./configure again.");
}
#endif

#endif

Name
IdentityManager::createIdentity(const Name& identityName, const KeyParams& params)
{
  if (!identityStorage_->doesIdentityExist(identityName)) {
    identityStorage_->addIdentity(identityName);
    Name keyName = generateKeyPair(identityName, true, params);
    identityStorage_->setDefaultKeyNameForIdentity(keyName, identityName);
    ptr_lib::shared_ptr<IdentityCertificate> selfCert = selfSign(keyName);
    addCertificateAsDefault(*selfCert);

    return keyName;
  }
  else
    throw SecurityException("Identity has already been created!");
}

void
IdentityManager::deleteIdentity(const Name& identityName)
{
  try {
    if (identityStorage_->getDefaultIdentity() == identityName)
      // Don't delete the default identity!
      return;
  }
  catch (SecurityException& ex) {
    // There is no default identity to check.
  }

  vector<Name> keysToDelete;
  identityStorage_->getAllKeyNamesOfIdentity(identityName, keysToDelete, true);
  identityStorage_->getAllKeyNamesOfIdentity(identityName, keysToDelete, false);

  identityStorage_->deleteIdentityInfo(identityName);

  for (size_t i = 0; i < keysToDelete.size(); ++i)
    privateKeyStorage_->deleteKeyPair(keysToDelete[i]);
}

Name
IdentityManager::generateKeyPair
  (const Name& identityName, bool isKsk, const KeyParams& params)
{
  Name keyName = identityStorage_->getNewKeyName(identityName, isKsk);
  privateKeyStorage_->generateKeyPair(keyName, params);
  ptr_lib::shared_ptr<PublicKey> pubKey = privateKeyStorage_->getPublicKey(keyName);
  identityStorage_->addKey(keyName, params.getKeyType(), pubKey->getKeyDer());

  return keyName;
}

Name
IdentityManager::generateRSAKeyPair(const Name& identityName, bool isKsk, int keySize)
{
  Name keyName = generateKeyPair(identityName, isKsk, RsaKeyParams(keySize));

  return keyName;
}

Name
IdentityManager::generateEcdsaKeyPair(const Name& identityName, bool isKsk, int keySize)
{
  Name keyName = generateKeyPair(identityName, isKsk, EcdsaKeyParams(keySize));
  return keyName;
}

Name
IdentityManager::generateRSAKeyPairAsDefault(const Name& identityName, bool isKsk, int keySize)
{
  Name keyName = generateKeyPair(identityName, isKsk, RsaKeyParams(keySize));

  identityStorage_->setDefaultKeyNameForIdentity(keyName, identityName);

  return keyName;
}

Name
IdentityManager::generateEcdsaKeyPairAsDefault(const Name& identityName, bool isKsk, int keySize)
{
  Name keyName = generateKeyPair(identityName, isKsk, EcdsaKeyParams(keySize));
  identityStorage_->setDefaultKeyNameForIdentity(keyName, identityName);
  return keyName;
}

Name
IdentityManager::createIdentityCertificate(const Name& certificatePrefix,
                                           const Name& signerCertificateName,
                                           const MillisecondsSince1970& notBefore,
                                           const MillisecondsSince1970& notAfter)
{
  Name keyName = getKeyNameFromCertificatePrefix(certificatePrefix);

  Blob keyBlob = identityStorage_->getKey(keyName);
  ptr_lib::shared_ptr<PublicKey> publicKey(new PublicKey(keyBlob));

  ptr_lib::shared_ptr<IdentityCertificate> certificate = createIdentityCertificate
    (certificatePrefix, *publicKey,  signerCertificateName, notBefore, notAfter);

  identityStorage_->addCertificate(*certificate);

  return certificate->getName();
}

ptr_lib::shared_ptr<IdentityCertificate>
IdentityManager::createIdentityCertificate(const Name& certificatePrefix,
                                           const PublicKey& publicKey,
                                           const Name& signerCertificateName,
                                           const MillisecondsSince1970& notBefore,
                                           const MillisecondsSince1970& notAfter)
{
  ptr_lib::shared_ptr<IdentityCertificate> certificate(new IdentityCertificate());
  Name keyName = getKeyNameFromCertificatePrefix(certificatePrefix);

  Name certificateName = certificatePrefix;
  MillisecondsSince1970 ti = ::ndn_getNowMilliseconds();
  // Get the number of seconds.
  ostringstream oss;
  oss << floor(ti / 1000.0);

  certificateName.append("ID-CERT").append(oss.str());

  certificate->setName(certificateName);
  certificate->setNotBefore(notBefore);
  certificate->setNotAfter(notAfter);
  certificate->setPublicKeyInfo(publicKey);
  certificate->addSubjectDescription(CertificateSubjectDescription("2.5.4.41", keyName.toUri()));
  certificate->encode();

  ptr_lib::shared_ptr<Sha256WithRsaSignature> sha256Sig(new Sha256WithRsaSignature());

  KeyLocator keyLocator;
  keyLocator.setType(ndn_KeyLocatorType_KEYNAME);
  keyLocator.setKeyName(signerCertificateName);

  sha256Sig->setKeyLocator(keyLocator);
  sha256Sig->getPublisherPublicKeyDigest().setPublisherPublicKeyDigest(publicKey.getDigest());

  certificate->setSignature(*sha256Sig);

  SignedBlob unsignedData = certificate->wireEncode();

  ptr_lib::shared_ptr<IdentityCertificate> signerCertificate = getCertificate(signerCertificateName);
  Name signerkeyName = signerCertificate->getPublicKeyName();

  Blob sigBits = privateKeyStorage_->sign(unsignedData, signerkeyName);

  sha256Sig->setSignature(sigBits);

  return certificate;
}

void
IdentityManager::addCertificateAsDefault(const IdentityCertificate& certificate)
{
  identityStorage_->addCertificate(certificate);

  setDefaultCertificateForKey(certificate);
}

void
IdentityManager::addCertificateAsIdentityDefault(const IdentityCertificate& certificate)
{
  identityStorage_->addCertificate(certificate);

  Name keyName = certificate.getPublicKeyName();

  setDefaultKeyForIdentity(keyName);

  setDefaultCertificateForKey(certificate);
}

void
IdentityManager::setDefaultCertificateForKey(const IdentityCertificate& certificate)
{
  Name keyName = certificate.getPublicKeyName();

  if(!identityStorage_->doesKeyExist(keyName))
    throw SecurityException("No corresponding Key record for certificate!");

  identityStorage_->setDefaultCertificateNameForKey(keyName, certificate.getName());
}

ptr_lib::shared_ptr<Signature>
IdentityManager::signByCertificate(const uint8_t* buffer, size_t bufferLength, const Name& certificateName)
{
  DigestAlgorithm digestAlgorithm;
  ptr_lib::shared_ptr<Signature> signature = makeSignatureByCertificate
    (certificateName, digestAlgorithm);

  signature->setSignature
    (privateKeyStorage_->sign(buffer, bufferLength,
     IdentityCertificate::certificateNameToPublicKeyName(certificateName),
     digestAlgorithm));

  return signature;
}

void
IdentityManager::signByCertificate(Data &data, const Name &certificateName, WireFormat& wireFormat)
{
  DigestAlgorithm digestAlgorithm;
  ptr_lib::shared_ptr<Signature> signature = makeSignatureByCertificate
    (certificateName, digestAlgorithm);

  data.setSignature(*signature);
  // Encode once to get the signed portion.
  SignedBlob encoding = data.wireEncode(wireFormat);

  data.getSignature()->setSignature
    (privateKeyStorage_->sign(encoding.signedBuf(), encoding.signedSize(),
     IdentityCertificate::certificateNameToPublicKeyName(certificateName),
     digestAlgorithm));

  // Encode again to include the signature.
  data.wireEncode(wireFormat);
}

void
IdentityManager::signInterestByCertificate
  (Interest& interest, const Name& certificateName, WireFormat& wireFormat)
{
  DigestAlgorithm digestAlgorithm;
  ptr_lib::shared_ptr<Signature> signature = makeSignatureByCertificate
    (certificateName, digestAlgorithm);

  // Append the encoded SignatureInfo.
  interest.getName().append(wireFormat.encodeSignatureInfo(*signature));

  // Append an empty signature so that the "signedPortion" is correct.
  interest.getName().append(Name::Component());
  // Encode once to get the signed portion, and sign.
  SignedBlob encoding = interest.wireEncode(wireFormat);
  signature->setSignature
    (privateKeyStorage_->sign(encoding.signedBuf(), encoding.signedSize(),
     IdentityCertificate::certificateNameToPublicKeyName(certificateName),
     digestAlgorithm));

  // Remove the empty signature and append the real one.
  interest.setName(interest.getName().getPrefix(-1).append
    (wireFormat.encodeSignatureValue(*signature)));
}

void
IdentityManager::signWithSha256(Data &data, WireFormat& wireFormat)
{
  data.setSignature(DigestSha256Signature());

  // Encode once to get the signed portion.
  SignedBlob encoding = data.wireEncode(wireFormat);

  // Digest and set the signature.
  uint8_t signedPortionDigest[ndn_SHA256_DIGEST_SIZE];
  ndn_digestSha256
    (encoding.signedBuf(), encoding.signedSize(), signedPortionDigest);
  data.getSignature()->setSignature
    (Blob(signedPortionDigest, sizeof(signedPortionDigest)));

  // Encode again to include the signature.
  data.wireEncode(wireFormat);
}

void
IdentityManager::signInterestWithSha256
  (Interest& interest, WireFormat& wireFormat)
{
  DigestSha256Signature signature;
  // Append the encoded SignatureInfo.
  interest.getName().append(wireFormat.encodeSignatureInfo(signature));

  // Append an empty signature so that the "signedPortion" is correct.
  interest.getName().append(Name::Component());
  // Encode once to get the signed portion.
  SignedBlob encoding = interest.wireEncode(wireFormat);

  // Digest and set the signature.
  uint8_t signedPortionDigest[ndn_SHA256_DIGEST_SIZE];
  ndn_digestSha256
    (encoding.signedBuf(), encoding.signedSize(), signedPortionDigest);
  signature.setSignature(Blob(signedPortionDigest, sizeof(signedPortionDigest)));

  // Remove the empty signature and append the real one.
  interest.setName(interest.getName().getPrefix(-1).append
    (wireFormat.encodeSignatureValue(signature)));
}

ptr_lib::shared_ptr<IdentityCertificate>
IdentityManager::selfSign(const Name& keyName)
{
  ptr_lib::shared_ptr<IdentityCertificate> certificate(new IdentityCertificate());

  Blob keyBlob = identityStorage_->getKey(keyName);
  ptr_lib::shared_ptr<PublicKey> publicKey(new PublicKey(keyBlob));

#if NDN_CPP_HAVE_GMTIME_SUPPORT
  time_t nowSeconds = time(NULL);
  struct tm current = *gmtime(&nowSeconds);
  current.tm_hour = 0;
  current.tm_min  = 0;
  current.tm_sec  = 0;
  MillisecondsSince1970 notBefore = timegm(&current) * 1000.0;
  current.tm_year = current.tm_year + 2;
  MillisecondsSince1970 notAfter = timegm(&current) * 1000.0;

  certificate->setNotBefore(notBefore);
  certificate->setNotAfter(notAfter);
#else
  // Don't really expect this to happen.
  throw SecurityException("selfSign: Can't set certificate validity because time functions are not supported by the standard library.");
#endif

  Name certificateName = keyName.getPrefix(-1).append("KEY").append
    (keyName.get(-1)).append("ID-CERT").append
    (Name::Component::fromNumber((uint64_t)ndn_getNowMilliseconds()));
  certificate->setName(certificateName);

  certificate->setPublicKeyInfo(*publicKey);
  certificate->addSubjectDescription(CertificateSubjectDescription("2.5.4.41", keyName.toUri()));
  certificate->encode();

  signByCertificate(*certificate, certificate->getName());

  return certificate;
}

Name
IdentityManager::getKeyNameFromCertificatePrefix(const Name & certificatePrefix)
{
  Name result;

  string keyString("KEY");
  int i = 0;
  for(; i < certificatePrefix.size(); i++) {
    if (certificatePrefix.get(i).toEscapedString() == keyString)
      break;
  }

  if (i >= certificatePrefix.size())
    throw SecurityException("Identity Certificate Prefix does not have a KEY component");

  result.append(certificatePrefix.getSubName(0, i));
  result.append(certificatePrefix.getSubName(i + 1, certificatePrefix.size()-i-1));

  return result;
}

ptr_lib::shared_ptr<Signature>
IdentityManager::makeSignatureByCertificate
  (const Name& certificateName, DigestAlgorithm& digestAlgorithm)
{
  Name keyName = IdentityCertificate::certificateNameToPublicKeyName
    (certificateName);
  ptr_lib::shared_ptr<PublicKey> publicKey = privateKeyStorage_->getPublicKey
    (keyName);
  KeyType keyType = publicKey->getKeyType();

  if (keyType == KEY_TYPE_RSA) {
    ptr_lib::shared_ptr<Sha256WithRsaSignature> signature
      (new Sha256WithRsaSignature());
    digestAlgorithm = DIGEST_ALGORITHM_SHA256;

    signature->getKeyLocator().setType(ndn_KeyLocatorType_KEYNAME);
    signature->getKeyLocator().setKeyName(certificateName.getPrefix(-1));
    // Ignore witness and leave the digestAlgorithm as the default.
    signature->getPublisherPublicKeyDigest().setPublisherPublicKeyDigest
      (publicKey->getDigest());

    return signature;
  }
  else if (keyType == KEY_TYPE_ECDSA) {
    ptr_lib::shared_ptr<Sha256WithEcdsaSignature> signature
      (new Sha256WithEcdsaSignature());
    digestAlgorithm = DIGEST_ALGORITHM_SHA256;

    signature->getKeyLocator().setType(ndn_KeyLocatorType_KEYNAME);
    signature->getKeyLocator().setKeyName(certificateName.getPrefix(-1));

    return signature;
  }
  else
    throw SecurityException("Key type is not recognized");
}

}
