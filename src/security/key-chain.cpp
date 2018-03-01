/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2018 Regents of the University of California.
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

#include <stdio.h>
#include <stdexcept>
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/util/crypto-lite.hpp>
#include <ndn-cpp/lite/security/ec-public-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/basic-identity-storage.hpp>
#include <ndn-cpp/security/policy/policy-manager.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>
#include <ndn-cpp/security/verification-helpers.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/digest-sha256-signature.hpp>
#include <ndn-cpp/hmac-with-sha256-signature.hpp>
#if NDN_CPP_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ndn-cpp/security/pib/pib-sqlite3.hpp>
#include <ndn-cpp/security/pib/pib-memory.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-osx.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-file.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>
#include "../util/config-file.hpp"
#include <ndn-cpp/security/key-chain.hpp>

INIT_LOGGER("ndn.KeyChain");

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

const RsaKeyParams KeyChain::DEFAULT_KEY_PARAMS;

string* KeyChain::defaultPibLocator_ = 0;
string* KeyChain::defaultTpmLocator_ = 0;
map<string, KeyChain::MakePibImpl>* KeyChain::pibFactories_ = 0;
map<string, KeyChain::MakeTpmBackEnd>* KeyChain::tpmFactories_ = 0;
SigningInfo* KeyChain::defaultSigningInfo_ = 0;
KeyParams* KeyChain::defaultKeyParams_ = 0;

#ifdef NDN_CPP_HAVE_SQLITE3
static ptr_lib::shared_ptr<PibImpl>
makePibSqlite3(const string& location)
{
  return ptr_lib::shared_ptr<PibImpl>(new PibSqlite3(location));
}
#endif

static ptr_lib::shared_ptr<PibImpl>
makePibMemory(const string& location)
{
  return ptr_lib::shared_ptr<PibImpl>(new PibMemory());
}

#if NDN_CPP_HAVE_OSX_SECURITY
static ptr_lib::shared_ptr<TpmBackEnd>
makeTpmBackEndOsx(const string& location)
{
  return ptr_lib::shared_ptr<TpmBackEnd>(new TpmBackEndOsx(location));
}
#endif

static ptr_lib::shared_ptr<TpmBackEnd>
makeTpmBackEndFile(const string& location)
{
  return ptr_lib::shared_ptr<TpmBackEnd>(new TpmBackEndFile(location));
}

static ptr_lib::shared_ptr<TpmBackEnd>
makeTpmBackEndMemory(const string& location)
{
  // Ignore the location.
  return ptr_lib::shared_ptr<TpmBackEnd>(new TpmBackEndMemory());
}

KeyChain::KeyChain
  (const string& pibLocator, const string& tpmLocator, bool allowReset)
: face_(0)
{
  isSecurityV1_ = false;
  construct(pibLocator, tpmLocator, allowReset);
}

KeyChain::KeyChain
  (const ptr_lib::shared_ptr<IdentityManager>& identityManager,
   const ptr_lib::shared_ptr<PolicyManager>& policyManager)
: identityManager_(identityManager), policyManager_(policyManager),
  face_(0)
{
  isSecurityV1_ = true;
}

KeyChain::KeyChain(const ptr_lib::shared_ptr<IdentityManager>& identityManager)
: identityManager_(identityManager),
  policyManager_(ptr_lib::make_shared<NoVerifyPolicyManager>()),
  face_(0)
{
  isSecurityV1_ = true;
}

KeyChain::KeyChain()
: face_(0)
{
  isSecurityV1_ = false;

// Only check for v1 files if we have Unix support and SQLite3.
#if NDN_CPP_HAVE_UNISTD_H
#ifdef NDN_CPP_HAVE_SQLITE3
  if (::access(BasicIdentityStorage::getDefaultDatabaseFilePath().c_str(), R_OK)
        == 0 &&
      ::access(PibSqlite3::getDefaultDatabaseFilePath().c_str(), R_OK) != 0) {
    // The security v1 SQLite file still exists and the security v2 does not yet.
    isSecurityV1_ = true;
    identityManager_.reset(new IdentityManager());
    policyManager_.reset(new NoVerifyPolicyManager());

    return;
  }
#endif
#endif

  construct("", "", true);
}

void
KeyChain::construct
  (const string& pibLocator, const string& tpmLocator, bool allowReset)
{
  // PIB locator.
  string pibScheme, pibLocation;
  parseAndCheckPibLocator(pibLocator, pibScheme, pibLocation);
  string canonicalPibLocator = pibScheme + ":" + pibLocation;

  // Create the PIB.
  pib_ = createPib(canonicalPibLocator);
  string oldTpmLocator = "";
  try {
    oldTpmLocator = pib_->getTpmLocator();
  }
  catch (const Pib::Error&) {
    // The TPM locator is not set in the PIB yet.
  }

  // TPM locator.
  string tpmScheme, tpmLocation;
  parseAndCheckTpmLocator(tpmLocator, tpmScheme, tpmLocation);
  string canonicalTpmLocator = tpmScheme + ":" + tpmLocation;

  ConfigFile config;
  if (canonicalPibLocator == getDefaultPibLocator(config)) {
    // The default PIB must use the default TPM.
    if (oldTpmLocator != "" && oldTpmLocator != getDefaultTpmLocator(config)) {
      pib_->reset();
      canonicalTpmLocator = getDefaultTpmLocator(config);
    }
  }
  else {
    // Check the consistency of the non-default PIB.
    if (oldTpmLocator != "" && oldTpmLocator != canonicalTpmLocator) {
      if (allowReset)
        pib_->reset();
      else
        throw LocatorMismatchError
          ("The supplied TPM locator does not match the TPM locator in the PIB: " +
           oldTpmLocator + " != " + canonicalTpmLocator);
    }
  }

  // Note that a key mismatch may still happen if the TPM locator is initially
  // set to a wrong one or if the PIB was shared by more than one TPM before.
  // This is due to the old PIB not having TPM info. The new PIB should not have
  // this problem.
  tpm_ = createTpm(canonicalTpmLocator);
  pib_->setTpmLocator(canonicalTpmLocator);
}

ptr_lib::shared_ptr<PibIdentity>
KeyChain::createIdentityV2(const Name& identityName, const KeyParams& params)
{
  ptr_lib::shared_ptr<PibIdentity> id = pib_->addIdentity(identityName);

  ptr_lib::shared_ptr<PibKey> key;
  try {
    key = id->getDefaultKey();
  }
  catch (const Pib::Error&) {
    key = createKey(*id, params);
  }

  try {
    key->getDefaultCertificate();
  }
  catch (const Pib::Error&) {
    _LOG_TRACE("No default cert for " << key->getName() << ", requesting self-signing");
    selfSign(key);
  }

  return id;
}

void
KeyChain::deleteIdentity(PibIdentity& identity)
{
  Name identityName = identity.getName();

  ptr_lib::shared_ptr<std::vector<Name> > keyNames =
    identity.getKeys().getKeyNames();
  for (size_t i = 0; i < keyNames->size(); ++i)
    tpm_->deleteKey(keyNames->at(i));

  pib_->removeIdentity(identityName);
  // TODO: Mark identity as invalid.
}

void
KeyChain::setDefaultIdentity(PibIdentity& identity)
{
  pib_->setDefaultIdentity(identity.getName());
}

ptr_lib::shared_ptr<PibKey>
KeyChain::createKey(PibIdentity& identity, const KeyParams& params)
{
  // Create the key in the TPM.
  Name keyName = tpm_->createKey(identity.getName(), params);

  // Set up the key info in the PIB.
  Blob publicKey = tpm_->getPublicKey(keyName);
  ptr_lib::shared_ptr<PibKey> key = identity.addKey
    (publicKey.buf(), publicKey.size(), keyName);

  _LOG_TRACE("Requesting self-signing for newly created key " << key->getName());
  selfSign(key);

  return key;
}

void
KeyChain::deleteKey(PibIdentity& identity, PibKey& key)
{
  Name keyName = key.getName();
  if (identity.getName() != key.getIdentityName())
    throw invalid_argument("Identity `" + identity.getName().toUri() +
      "` does not match key `" + keyName.toUri() + "`");

  identity.removeKey(keyName);
  tpm_->deleteKey(keyName);
}

void
KeyChain::setDefaultKey(PibIdentity& identity, PibKey& key)
{
  if (identity.getName() != key.getIdentityName())
    throw invalid_argument("Identity `" + identity.getName().toUri() +
      "` does not match key `" + key.getName().toUri() + "`");

  identity.setDefaultKey(key.getName());
}

void
KeyChain::addCertificate(PibKey& key, const CertificateV2& certificate)
{
  if (key.getName() != certificate.getKeyName() ||
      !certificate.getContent().equals(key.getPublicKey()))
    throw invalid_argument("Key `" + key.getName().toUri() +
      "` does not match certificate `" + certificate.getName().toUri() + "`");

  key.addCertificate(certificate);
}

void
KeyChain::deleteCertificate(PibKey& key, const Name& certificateName)
{
  if (!CertificateV2::isValidName(certificateName))
    throw invalid_argument("Wrong certificate name `" + certificateName.toUri() +
      "`");

  key.removeCertificate(certificateName);
}

void
KeyChain::setDefaultCertificate(PibKey& key, const CertificateV2& certificate)
{
  // This replaces the certificate it it exists.
  addCertificate(key, certificate);
  key.setDefaultCertificate(certificate.getName());
}

void
KeyChain::sign
  (Data& data, const SigningInfo& params, WireFormat& wireFormat)
{
  Name keyName;
  ptr_lib::shared_ptr<Signature> signatureInfo = prepareSignatureInfo
    (params, keyName);

  data.setSignature(*signatureInfo);

  // Encode once to get the signed portion.
  SignedBlob encoding = data.wireEncode(wireFormat);

  Blob signatureBytes = sign
    (encoding.signedBuf(), encoding.signedSize(), keyName,
     params.getDigestAlgorithm());
  data.getSignature()->setSignature(signatureBytes);

  // Encode again to include the signature.
  data.wireEncode(wireFormat);
}

void
KeyChain::sign
  (Interest& interest, const SigningInfo& params, WireFormat& wireFormat)
{
  Name keyName;
  ptr_lib::shared_ptr<Signature> signatureInfo = prepareSignatureInfo
    (params, keyName);

  // Append the encoded SignatureInfo.
  interest.getName().append(wireFormat.encodeSignatureInfo(*signatureInfo));

  // Append an empty signature so that the "signedPortion" is correct.
  interest.getName().append(Name::Component());
  // Encode once to get the signed portion, and sign.
  SignedBlob encoding = interest.wireEncode(wireFormat);
  Blob signatureBytes = sign
    (encoding.signedBuf(), encoding.signedSize(), keyName,
     params.getDigestAlgorithm());
  signatureInfo->setSignature(signatureBytes);

  // Remove the empty signature and append the real one.
  interest.setName(interest.getName().getPrefix(-1).append
    (wireFormat.encodeSignatureValue(*signatureInfo)));
}

Blob
KeyChain::sign
  (const uint8_t* buffer, size_t bufferLength, const SigningInfo& params)
{
  Name keyName;
  ptr_lib::shared_ptr<Signature> signatureInfo = prepareSignatureInfo
    (params, keyName);

  return sign(buffer, bufferLength, keyName, params.getDigestAlgorithm());
}

ptr_lib::shared_ptr<CertificateV2>
KeyChain::selfSign(ptr_lib::shared_ptr<PibKey>& key, WireFormat& wireFormat)
{
  ptr_lib::shared_ptr<CertificateV2> certificate(new CertificateV2());

  // Set the name.
  MillisecondsSince1970 now = ndn_getNowMilliseconds();
  Name certificateName = key->getName();
  certificateName.append("self").appendVersion((uint64_t)now);
  certificate->setName(certificateName);

  // Set the MetaInfo.
  certificate->getMetaInfo().setType(ndn_ContentType_KEY);
  // Set a one-hour freshness period.
  certificate->getMetaInfo().setFreshnessPeriod(3600 * 1000.0);

  // Set the content.
  certificate->setContent(key->getPublicKey());

  // Set the signature-info.
  SigningInfo signingInfo(key);
  // Set a 20-year validity period.
  signingInfo.setValidityPeriod
    (ValidityPeriod(now, now + 20 * 365 * 24 * 3600 * 1000.0));

  sign(*certificate, signingInfo, wireFormat);

  key->addCertificate(*certificate);
  return certificate;
}

void
KeyChain::importSafeBag
  (const SafeBag& safeBag, const uint8_t* password, size_t passwordLength)
{
  CertificateV2 certificate(*safeBag.getCertificate());
  Name identity = certificate.getIdentity();
  Name keyName = certificate.getKeyName();
  Blob publicKeyBits = certificate.getPublicKey();

  if (tpm_->hasKey(keyName))
    throw Error("Private key `" + keyName.toUri() + "` already exists");

  try {
    ptr_lib::shared_ptr<PibIdentity> existingId = pib_->getIdentity(identity);
    existingId->getKey(keyName);
    throw Error("Public key `" + keyName.toUri() + "` already exists");
  }
  catch (const Pib::Error& ex) {
    // Either the identity or the key doesn't exist, so OK to import.
  }

  try {
    tpm_->importPrivateKey
      (keyName, safeBag.getPrivateKeyBag().buf(),
       safeBag.getPrivateKeyBag().size(), password, passwordLength);
  }
  catch (const std::exception& ex) {
    throw Error("Failed to import private key `" + keyName.toUri() + "`: " +
                ex.what());
  }

  // Check the consistency of the private key and certificate.
  uint8_t content[] = {0x01, 0x02, 0x03, 0x04};
  Blob signatureBits;
  try {
    signatureBits = tpm_->sign
      (content, sizeof(content), keyName, DIGEST_ALGORITHM_SHA256);
  } catch (const std::exception& ex) {
    tpm_->deleteKey(keyName);
    throw Error("Invalid private key `" + keyName.toUri() + "`");
  }

  if (!VerificationHelpers::verifySignature
      (Blob(content, sizeof(content)), signatureBits, PublicKey(publicKeyBits))) {
    tpm_->deleteKey(keyName);
    throw Error("Certificate `" + certificate.getName().toUri() +
      "` and private key `" + keyName.toUri() + "` do not match");
  }

  // The consistency is verified. Add to the PIB.
  ptr_lib::shared_ptr<PibIdentity> id = pib_->addIdentity(identity);
  ptr_lib::shared_ptr<PibKey> key = id->addKey
    (certificate.getPublicKey().buf(), certificate.getPublicKey().size(),
     keyName);
  key->addCertificate(certificate);
}

// Security v1 methods

void
KeyChain::signByIdentity(Data& data, const Name& identityName, WireFormat& wireFormat)
{
  if (!isSecurityV1_) {
    SigningInfo signingInfo;
    signingInfo.setSigningIdentity(identityName);
    sign(data, signingInfo, wireFormat);
    return;
  }

  Name signingCertificateName;

  if (identityName.size() == 0) {
    Name inferredIdentity = policyManager_->inferSigningIdentity(data.getName());
    if (inferredIdentity.size() == 0)
      signingCertificateName = identityManager_->getDefaultCertificateName();
    else
      signingCertificateName = identityManager_->getDefaultCertificateNameForIdentity(inferredIdentity);
  }
  else
    signingCertificateName = identityManager_->getDefaultCertificateNameForIdentity(identityName);

  if (signingCertificateName.size() == 0)
    throw SecurityException("No qualified certificate name found!");

  if (!policyManager_->checkSigningPolicy(data.getName(), signingCertificateName))
    throw SecurityException("Signing Cert name does not comply with signing policy");

  identityManager_->signByCertificate(data, signingCertificateName, wireFormat);
}

ptr_lib::shared_ptr<Signature>
KeyChain::signByIdentity(const uint8_t* buffer, size_t bufferLength, const Name& identityName)
{
  if (!isSecurityV1_)
    throw Error
      ("signByIdentity(buffer, identityName) is not supported for security v2. Use sign with SigningInfo.");

  Name signingCertificateName = identityManager_->getDefaultCertificateNameForIdentity(identityName);

  if (signingCertificateName.size() == 0)
    throw SecurityException("No qualified certificate name found!");

  return identityManager_->signByCertificate(buffer, bufferLength, signingCertificateName);
}

void
KeyChain::verifyData
  (const ptr_lib::shared_ptr<Data>& data, const OnVerified& onVerified,
   const OnDataValidationFailed& onValidationFailed, int stepCount)
{
  _LOG_TRACE("Enter Verify");

  if (policyManager_->requireVerify(*data)) {
    ptr_lib::shared_ptr<ValidationRequest> nextStep = policyManager_->checkVerificationPolicy
      (data, stepCount, onVerified, onValidationFailed);
    if (nextStep)
      face_->expressInterest
        (*nextStep->interest_,
         bind(&KeyChain::onCertificateData, this, _1, _2, nextStep),
         bind(&KeyChain::onCertificateInterestTimeout, this, _1, 
              nextStep->retry_, onValidationFailed, data, nextStep));
  }
  else if (policyManager_->skipVerifyAndTrust(*data)) {
    try {
      onVerified(data);
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::verifyData: Error in onVerified: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::verifyData: Error in onVerified.");
    }
  }
  else {
    try {
      onValidationFailed
        (data,
         "The packet has no verify rule but skipVerifyAndTrust is false");
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::verifyData: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::verifyData: Error in onValidationFailed.");
    }
  }
}

static void
onDataValidationFailedWrapper
  (const ptr_lib::shared_ptr<Data>& data, const string& reason,
   const OnVerifyFailed& onVerifyFailed)
{
  onVerifyFailed(data);
}

void
KeyChain::verifyData
  (const ptr_lib::shared_ptr<Data>& data, const OnVerified& onVerified,
   const OnVerifyFailed& onVerifyFailed, int stepCount)
{
  verifyData
    (data, onVerified,
     // Cast to disambiguate from the deprecated OnVerifyFailed.
     (const OnDataValidationFailed)bind
       (&onDataValidationFailedWrapper, _1, _2, onVerifyFailed), stepCount);
}

void
KeyChain::verifyInterest
  (const ptr_lib::shared_ptr<Interest>& interest,
   const OnVerifiedInterest& onVerified,
   const OnInterestValidationFailed& onValidationFailed, int stepCount,
   WireFormat& wireFormat)
{
  _LOG_TRACE("Enter Verify");

  if (policyManager_->requireVerify(*interest)) {
    ptr_lib::shared_ptr<ValidationRequest> nextStep =
      policyManager_->checkVerificationPolicy
        (interest, stepCount, onVerified, onValidationFailed, wireFormat);
    if (nextStep)
      face_->expressInterest
        (*nextStep->interest_,
         bind(&KeyChain::onCertificateData, this, _1, _2, nextStep),
         bind(&KeyChain::onCertificateInterestTimeoutForVerifyInterest, this,
              _1, nextStep->retry_, onValidationFailed, interest, nextStep));
  }
  else if (policyManager_->skipVerifyAndTrust(*interest)) {
    try {
      onVerified(interest);
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::verifyInterest: Error in onVerified: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::verifyInterest: Error in onVerified.");
    }
  }
  else {
    try {
      onValidationFailed
        (interest,
         "The packet has no verify rule but skipVerifyAndTrust is false");
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::verifyInterest: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::verifyInterest: Error in onValidationFailed.");
    }
  }
}

static void
onInterestValidationFailedWrapper
  (const ptr_lib::shared_ptr<Interest>& interest, const string& reason,
   const OnVerifyInterestFailed& onVerifyFailed)
{
  onVerifyFailed(interest);
}

void
KeyChain::verifyInterest
  (const ptr_lib::shared_ptr<Interest>& interest,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, int stepCount,
   WireFormat& wireFormat)
{
  verifyInterest
    (interest, onVerified,
     // Cast to disambiguate from the deprecated OnVerifyInterestFailed.
     (const OnInterestValidationFailed)bind
       (&onInterestValidationFailedWrapper, _1, _2, onVerifyFailed),
     stepCount, wireFormat);
}

#if NDN_CPP_HAVE_LIBCRYPTO
void
KeyChain::signWithHmacWithSha256
  (Data& data, const Blob& key, WireFormat& wireFormat)
{
  // Encode once to get the signed portion.
  SignedBlob encoding = data.wireEncode(wireFormat);

  ptr_lib::shared_ptr<vector<uint8_t>> signatureBits
    (new vector<uint8_t>(ndn_SHA256_DIGEST_SIZE));
  CryptoLite::computeHmacWithSha256
    (key, encoding.getSignedPortionBlobLite(), &signatureBits->front());
  data.getSignature()->setSignature(Blob(signatureBits, false));

  // Encode again to include the signature.
  data.wireEncode(wireFormat);
}

void
KeyChain::signWithHmacWithSha256
  (Interest& interest, const Blob& key, const Name& keyName,
   WireFormat& wireFormat)
{
  HmacWithSha256Signature signature;
  signature.getKeyLocator().setType(ndn_KeyLocatorType_KEYNAME);
  signature.getKeyLocator().setKeyName(keyName);

  // Append the encoded SignatureInfo.
  interest.getName().append(wireFormat.encodeSignatureInfo(signature));
  // Append an empty signature so that the "signedPortion" is correct.
  interest.getName().append(Name::Component());

  // Encode once to get the signed portion and sign.
  SignedBlob encoding = interest.wireEncode(wireFormat);

  ptr_lib::shared_ptr<vector<uint8_t>> signatureBits
    (new vector<uint8_t>(ndn_SHA256_DIGEST_SIZE));
  CryptoLite::computeHmacWithSha256
    (key, encoding.getSignedPortionBlobLite(), &signatureBits->front());
  signature.setSignature(Blob(signatureBits, false));

  // Remove the empty signature and append the real one.
  interest.setName(interest.getName().getPrefix(-1).append
    (wireFormat.encodeSignatureValue(signature)));
}

bool
KeyChain::verifyDataWithHmacWithSha256
  (const Data& data, const Blob& key, WireFormat& wireFormat)
{
  // wireEncode returns the cached encoding if available.
  SignedBlob encoding = data.wireEncode(wireFormat);

  return CryptoLite::verifyHmacWithSha256Signature
    (key, data.getSignature()->getSignature(), encoding.getSignedPortionBlobLite());
}

bool
KeyChain::verifyInterestWithHmacWithSha256
  (const Interest& interest, const Blob& key, WireFormat& wireFormat)
{
  // Decode the last two name components of the signed interest.
  ptr_lib::shared_ptr<Signature> signature =
    wireFormat.decodeSignatureInfoAndValue
      (interest.getName().get(-2).getValue(),
       interest.getName().get(-1).getValue());

  // wireEncode returns the cached encoding if available.
  SignedBlob encoding = interest.wireEncode(wireFormat);

  vector<uint8_t> newSignatureBits(ndn_SHA256_DIGEST_SIZE);
  return CryptoLite::verifyHmacWithSha256Signature
    (key, signature->getSignature(), encoding.getSignedPortionBlobLite());
}
#endif

// Private security v2 methods

map<string, KeyChain::MakePibImpl>&
KeyChain::getPibFactories()
{
  if (!pibFactories_) {
    // Allocate because some C++ environments don't handle static constructors well.
    pibFactories_ = new map<string, MakePibImpl>();

    // Add the standard factories.
#ifdef NDN_CPP_HAVE_SQLITE3
    (*pibFactories_)[PibSqlite3::getScheme()] = &makePibSqlite3;
#endif
    (*pibFactories_)[PibMemory::getScheme()] = &makePibMemory;
  }

  return *pibFactories_;
}

map<string, KeyChain::MakeTpmBackEnd>&
KeyChain::getTpmFactories()
{
  if (!tpmFactories_) {
    // Allocate because some C++ environments don't handle static constructors well.
    tpmFactories_ = new map<string, MakeTpmBackEnd>();

    // Add the standard factories.
#if NDN_CPP_HAVE_OSX_SECURITY
    (*tpmFactories_)[TpmBackEndOsx::getScheme()] = &makeTpmBackEndOsx;
#endif
    (*tpmFactories_)[TpmBackEndFile::getScheme()] = &makeTpmBackEndFile;
    (*tpmFactories_)[TpmBackEndMemory::getScheme()] = &makeTpmBackEndMemory;
  }

  return *tpmFactories_;
}

void
KeyChain::parseLocatorUri(const string& uri, string& scheme, string& location)
{
  size_t iColon = uri.find(':');
  if (iColon != string::npos) {
    scheme = uri.substr(0, iColon);
    location = uri.substr(iColon + 1);
  }
  else {
    scheme = uri;
    location = "";
  }
}

void
KeyChain::parseAndCheckPibLocator
  (const string& pibLocator, string& pibScheme, string& pibLocation)
{
  parseLocatorUri(pibLocator, pibScheme, pibLocation);

  if (pibScheme == "")
    pibScheme = getDefaultPibScheme();

  map<string, MakePibImpl>::const_iterator pibFactory =
    getPibFactories().find(pibScheme);
  if (pibFactory == getPibFactories().end())
    throw Error("PIB scheme `" + pibScheme + "` is not supported");
}

void
KeyChain::parseAndCheckTpmLocator
  (const string& tpmLocator, string& tpmScheme, string& tpmLocation)
{
  parseLocatorUri(tpmLocator, tpmScheme, tpmLocation);

  if (tpmScheme == "")
    tpmScheme = getDefaultTpmScheme();

  map<string, MakeTpmBackEnd>::const_iterator tpmFactory =
    getTpmFactories().find(tpmScheme);
  if (tpmFactory == getTpmFactories().end())
    throw Error("TPM scheme `" + tpmScheme + "` is not supported");
}

string
KeyChain::getDefaultPibScheme()
{
  return NDN_PIB_SQLITE3_SCHEME;
}

string
KeyChain::getDefaultTpmScheme()
{
#if NDN_CPP_HAVE_OSX_SECURITY && NDN_CPP_WITH_OSX_KEYCHAIN
  return TpmBackEndOsx::getScheme();
#else
  return TpmBackEndFile::getScheme();
#endif
}

ptr_lib::shared_ptr<Pib>
KeyChain::createPib(const string& pibLocator)
{
  string pibScheme, pibLocation;
  parseAndCheckPibLocator(pibLocator, pibScheme, pibLocation);
  map<string, MakePibImpl>::const_iterator pibFactory =
    getPibFactories().find(pibScheme);
  return ptr_lib::shared_ptr<Pib>(new Pib
    (pibScheme, pibLocation, pibFactory->second(pibLocation)));
}

ptr_lib::shared_ptr<Tpm>
KeyChain::createTpm(const string& tpmLocator)
{
  string tpmScheme, tpmLocation;
  parseAndCheckTpmLocator(tpmLocator, tpmScheme, tpmLocation);
  map<string, MakeTpmBackEnd>::const_iterator tpmFactory =
    getTpmFactories().find(tpmScheme);
  return  ptr_lib::shared_ptr<Tpm>(new Tpm
    (tpmScheme, tpmLocation, tpmFactory->second(tpmLocation)));
}

string
KeyChain::getDefaultPibLocator(ConfigFile& config)
{
  if (!defaultPibLocator_)
    // Allocate because some C++ environments don't handle static constructors well.
    defaultPibLocator_ = new string();

  if (*defaultPibLocator_ != "")
    return *defaultPibLocator_;

  const char* clientPib = getenv("NDN_CLIENT_PIB");
  if (clientPib && *clientPib != '\0')
    *defaultPibLocator_ = clientPib;
  else
     *defaultPibLocator_ = config.get("pib", getDefaultPibScheme() + ":");

  return *defaultPibLocator_;
}

string
KeyChain::getDefaultTpmLocator(ConfigFile& config)
{
  if (!defaultTpmLocator_)
    // Allocate because some C++ environments don't handle static constructors well.
    defaultTpmLocator_ = new string();

  if (*defaultTpmLocator_ != "")
    return *defaultTpmLocator_;

  const char* clientTpm = getenv("NDN_CLIENT_TPM");
  if (clientTpm && *clientTpm != '\0')
    *defaultTpmLocator_ = clientTpm;
  else
     *defaultTpmLocator_ = config.get("tpm", getDefaultTpmScheme() + ":");

  return *defaultTpmLocator_;
}

ptr_lib::shared_ptr<Signature>
KeyChain::prepareSignatureInfo(const SigningInfo& params, Name& keyName)
{
  ptr_lib::shared_ptr<PibIdentity> identity;
  ptr_lib::shared_ptr<PibKey> key;

  if (params.getSignerType() == SigningInfo::SIGNER_TYPE_NULL) {
    try {
      identity = pib_->getDefaultIdentity();
    }
    catch (const Pib::Error&) {
      // There is no default identity, so use sha256 for signing.
      keyName = SigningInfo::getDigestSha256Identity();
      return ptr_lib::shared_ptr<Signature>(new DigestSha256Signature());
    }
  }
  else if (params.getSignerType() == SigningInfo::SIGNER_TYPE_ID) {
    identity = params.getPibIdentity();
    if (!identity) {
      try {
        identity = pib_->getIdentity(params.getSignerName());
      }
      catch (const Pib::Error&) {
        throw InvalidSigningInfoError
          ("Signing identity `" + params.getSignerName().toUri() +
           "` does not exist");
      }
    }
  }
  else if (params.getSignerType() == SigningInfo::SIGNER_TYPE_KEY) {
    key = params.getPibKey();
    if (!key) {
      Name identityName = PibKey::extractIdentityFromKeyName
        (params.getSignerName());

      try {
        identity = pib_->getIdentity(identityName);
        key = identity->getKey(params.getSignerName());
        // We will use the PIB key instance, so reset the identity.
        identity.reset();
      }
      catch (const Pib::Error&) {
        throw InvalidSigningInfoError
          ("Signing key `" + params.getSignerName().toUri() + "` does not exist");
      }
    }
  }
  else if (params.getSignerType() == SigningInfo::SIGNER_TYPE_CERT) {
    Name identityName = CertificateV2::extractIdentityFromCertName
      (params.getSignerName());

    try {
      identity = pib_->getIdentity(identityName);
      key = identity->getKey
        (CertificateV2::extractKeyNameFromCertName(params.getSignerName()));
    }
    catch (const Pib::Error&) {
      throw InvalidSigningInfoError
        ("Signing certificate `" + params.getSignerName().toUri() +
         "` does not exist");
    }
  }
  else if (params.getSignerType() == SigningInfo::SIGNER_TYPE_SHA256) {
    keyName = SigningInfo::getDigestSha256Identity();
    return ptr_lib::shared_ptr<Signature>(new DigestSha256Signature());
  }
  else
    // We don't expect this to happen.
    throw InvalidSigningInfoError("Unrecognized signer type");

  if (!identity && !key)
    throw InvalidSigningInfoError("Cannot determine signing parameters");

  if (identity && !key) {
    try {
      key = identity->getDefaultKey();
    }
    catch (const Pib::Error&) {
      throw InvalidSigningInfoError
        ("Signing identity `" + identity->getName().toUri() +
         "` does not have default certificate");
    }
  }

  ptr_lib::shared_ptr<Signature> signatureInfo;

  if (key->getKeyType() == KEY_TYPE_RSA &&
      params.getDigestAlgorithm() == DIGEST_ALGORITHM_SHA256)
    signatureInfo.reset(new Sha256WithRsaSignature());
  else if (key->getKeyType() == KEY_TYPE_EC &&
           params.getDigestAlgorithm() == DIGEST_ALGORITHM_SHA256)
    signatureInfo.reset(new Sha256WithEcdsaSignature());
  else
    throw Error("Unsupported key type");

  if (params.getValidityPeriod().hasPeriod() &&
      ValidityPeriod::canGetFromSignature(signatureInfo.get()))
    // Set the ValidityPeriod from the SigningInfo params.
    ValidityPeriod::getFromSignature(signatureInfo.get()).setPeriod
      (params.getValidityPeriod().getNotBefore(),
       params.getValidityPeriod().getNotAfter());

  KeyLocator& keyLocator = KeyLocator::getFromSignature(signatureInfo.get());
  keyLocator.setType(ndn_KeyLocatorType_KEYNAME);
  keyLocator.setKeyName(key->getName());

  keyName = key->getName();
  return signatureInfo;
}

Blob
KeyChain::sign
  (const uint8_t* buffer, size_t bufferLength, const Name& keyName,
   DigestAlgorithm digestAlgorithm) const
{
  if (keyName == SigningInfo::getDigestSha256Identity()) {
    uint8_t digest[ndn_SHA256_DIGEST_SIZE];
    CryptoLite::digestSha256(buffer, bufferLength, digest);
    return Blob(digest, sizeof(digest));
  }

  return tpm_->sign(buffer, bufferLength, keyName, digestAlgorithm);
}

const SigningInfo&
KeyChain::getDefaultSigningInfo()
{
  if (!defaultSigningInfo_)
    // Allocate because some C++ environments don't handle static constructors well.
    defaultSigningInfo_ = new SigningInfo();

  return *defaultSigningInfo_;
}

const KeyParams&
KeyChain::getDefaultKeyParams()
{
  if (!defaultKeyParams_)
    // Allocate because some C++ environments don't handle static constructors well.
    defaultKeyParams_ = new RsaKeyParams();

  return *defaultKeyParams_;
}

// Private security v1 methods

void
KeyChain::onCertificateData(const ptr_lib::shared_ptr<const Interest> &interest, const ptr_lib::shared_ptr<Data> &data, ptr_lib::shared_ptr<ValidationRequest> nextStep)
{
  // Try to verify the certificate (data) according to the parameters in nextStep.
  verifyData
    (data, nextStep->onVerified_, nextStep->onValidationFailed_,
     nextStep->stepCount_);
}

void
KeyChain::onCertificateInterestTimeout
  (const ptr_lib::shared_ptr<const Interest> &interest, int retry,
   const OnDataValidationFailed& onValidationFailed,
   const ptr_lib::shared_ptr<Data> &data,
   ptr_lib::shared_ptr<ValidationRequest> nextStep)
{
  if (retry > 0)
    // Issue the same expressInterest as in verifyData except decrement retry.
    face_->expressInterest
      (*interest,
       bind(&KeyChain::onCertificateData, this, _1, _2, nextStep),
       bind(&KeyChain::onCertificateInterestTimeout, this, _1, retry - 1, 
            onValidationFailed, data, nextStep));
  else {
    try {
      onValidationFailed
        (data,
         "The retry count is zero after timeout for fetching " +
         interest->getName().toUri());
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::onCertificateInterestTimeout: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::onCertificateInterestTimeout: Error in onValidationFailed.");
    }
  }
}

void
KeyChain::onCertificateInterestTimeoutForVerifyInterest
  (const ptr_lib::shared_ptr<const Interest> &interest, int retry,
   const OnInterestValidationFailed& onValidationFailed,
   const ptr_lib::shared_ptr<Interest>& originalInterest,
   ptr_lib::shared_ptr<ValidationRequest> nextStep)
{
  if (retry > 0)
    // Issue the same expressInterest as in verifyInterest except decrement retry.
    face_->expressInterest
      (*interest,
       bind(&KeyChain::onCertificateData, this, _1, _2, nextStep),
       bind(&KeyChain::onCertificateInterestTimeoutForVerifyInterest, this,
            _1, retry - 1, onValidationFailed, originalInterest, nextStep));
  else {
    try {
      onValidationFailed
        (originalInterest,
         "The retry count is zero after timeout for fetching " +
         interest->getName().toUri());
    } catch (const std::exception& ex) {
      _LOG_ERROR("KeyChain::onCertificateInterestTimeoutForVerifyInterest: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("KeyChain::onCertificateInterestTimeoutForVerifyInterest: Error in onValidationFailed.");
    }
  }
}

Name
KeyChain::prepareDefaultCertificateName()
{
  ptr_lib::shared_ptr<IdentityCertificate> signingCertificate =
    identityManager_->getDefaultCertificate();
  if (!signingCertificate) {
    setDefaultCertificate();
    signingCertificate = identityManager_->getDefaultCertificate();
  }

  return signingCertificate->getName();
}

void
KeyChain::setDefaultCertificate()
{
  if (!identityManager_->getDefaultCertificate()) {
    Name defaultIdentity;
    try {
      defaultIdentity = identityManager_->getDefaultIdentity();
    } catch (SecurityException&) {
      // Create a default identity name.
      uint8_t randomComponent[4];
      ndn_Error error;
      if ((error = CryptoLite::generateRandomBytes
           (randomComponent, sizeof(randomComponent))))
        throw runtime_error(ndn_getErrorString(error));
      defaultIdentity = Name();
      defaultIdentity.append("tmp-identity")
        .append(Blob(randomComponent, sizeof(randomComponent)));
    }

    createIdentityAndCertificate(defaultIdentity);
    identityManager_->setDefaultIdentity(defaultIdentity);
  }
}

}
