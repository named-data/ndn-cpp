/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN config_policy_manager.py by Adeola Bannis.
 * Originally from Yingdi Yu <http://irl.cs.ucla.edu/~yingdi/>.
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

#include "../../util/regex/ndn-regex-top-matcher.hpp"
// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher-base.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/certificate/identity-certificate.hpp>
#include "../../util/boost-info-parser.hpp"
#include "../../c/util/time.h"
#include "../../encoding/base64.hpp"
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/security/policy/config-policy-manager.hpp>

using namespace std;
using namespace ndn::func_lib;

INIT_LOGGER("ndn.ConfigPolicyManager");

namespace ndn {

using namespace regex_lib;

/**
 * Ignore data and call onValidationFailed(interest, reason). This is so that an
 * OnInterestValidationFailed can be passed as an OnDataValidationFailed.
 */
static void
onInterestValidationFailedWrapper
  (const ptr_lib::shared_ptr<Data>& data, const string& reason,
   const OnInterestValidationFailed& onValidationFailed,
   const ptr_lib::shared_ptr<Interest>& interest)
{
  try {
    onValidationFailed(interest, reason);
  } catch (const std::exception& ex) {
    _LOG_ERROR("ConfigPolicyManager::onInterestValidationFailedWrapper: Error in onValidationFailed: " << ex.what());
  } catch (...) {
    _LOG_ERROR("ConfigPolicyManager::onInterestValidationFailedWrapper: Error in onValidationFailed.");
  }
}

ConfigPolicyManager::ConfigPolicyManager
  (const string& configFileName,
   const ptr_lib::shared_ptr<CertificateCache>& certificateCache,
   int searchDepth, Milliseconds graceInterval, Milliseconds keyTimestampTtl,
   int maxTrackedKeys)
  : maxDepth_(searchDepth),
    keyGraceInterval_(graceInterval),
    keyTimestampTtl_(keyTimestampTtl),
    maxTrackedKeys_(maxTrackedKeys)
{
  if (!certificateCache)
    certificateCache_.reset(new CertificateCache());
  else
    certificateCache_ = certificateCache;

  reset();

  if (configFileName != "")
    load(configFileName);
}

ConfigPolicyManager::~ConfigPolicyManager()
{
}

void
ConfigPolicyManager::reset()
{
  certificateCache_->reset();
  fixedCertificateCache_.clear();
  keyTimestamps_.clear();
  requiresVerification_ = true;
  config_.reset(new BoostInfoParser());
  refreshManager_.reset(new TrustAnchorRefreshManager());
}

void
ConfigPolicyManager::load(const std::string& configFileName)
{
  reset();
  config_->read(configFileName);
  loadTrustAnchorCertificates();
}

void
ConfigPolicyManager::load(const std::string& input, const std::string& inputName)
{
  reset();
  config_->read(input, inputName);
  loadTrustAnchorCertificates();
}

bool
ConfigPolicyManager::skipVerifyAndTrust(const Data& data)
{
  return !requiresVerification_;
}

bool
ConfigPolicyManager::skipVerifyAndTrust(const Interest& interest)
{
  return !requiresVerification_;
}

bool
ConfigPolicyManager::requireVerify(const Data& data)
{
  return requiresVerification_;
}

bool
ConfigPolicyManager::requireVerify(const Interest& interest)
{
  return requiresVerification_;
}

ptr_lib::shared_ptr<ValidationRequest>
ConfigPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Data>& data, int stepCount,
   const OnVerified& onVerified, 
   const OnDataValidationFailed& onValidationFailed)
{
  string failureReason = "unknown";
  ptr_lib::shared_ptr<Interest> certificateInterest = getCertificateInterest
    (stepCount, "data", data->getName(), data->getSignature(), failureReason);
  if (!certificateInterest) {
    try {
      onValidationFailed(data, failureReason);
    } catch (const std::exception& ex) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  if (certificateInterest->getName().size() > 0)
    return ptr_lib::make_shared<ValidationRequest>
      (certificateInterest,
       bind(&ConfigPolicyManager::onCertificateDownloadComplete, this, _1,
            data, stepCount, onVerified, onValidationFailed),
       onValidationFailed, 2, stepCount + 1);
  else {
    // Certificate is known. Verify the signature.
    // wireEncode returns the cached encoding if available.
    if (verify(data->getSignature(), data->wireEncode(), failureReason)) {
      try {
        onVerified(data);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onVerified: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onVerified.");
      }
    }
    else {
      try {
        onValidationFailed(data, failureReason);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
      }
    }

    return ptr_lib::shared_ptr<ValidationRequest>();
  }
}

ptr_lib::shared_ptr<ValidationRequest>
ConfigPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnInterestValidationFailed& onValidationFailed,
   WireFormat& wireFormat)
{
  string failureReason = "unknown";
  ptr_lib::shared_ptr<Signature> signature = extractSignature
    (*interest, wireFormat, failureReason);
  if (!signature) {
    // Can't get the signature from the interest name.
    try {
      onValidationFailed(interest, failureReason);
    } catch (const std::exception& ex) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  // For command interests, we need to ignore the last 4 components when
  //   matching the name.
  ptr_lib::shared_ptr<Interest> certificateInterest = getCertificateInterest
    (stepCount, "interest", interest->getName().getPrefix(-4), 
     signature.get(), failureReason);
  if (!certificateInterest) {
    try {
      onValidationFailed(interest, failureReason);
    } catch (const std::exception& ex) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
    } catch (...) {
      _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
    }
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  if (certificateInterest->getName().size() > 0)
    return ptr_lib::make_shared<ValidationRequest>
      (certificateInterest,
       bind(&ConfigPolicyManager::onCertificateDownloadCompleteForInterest, this, _1,
            interest, stepCount, onVerified, onValidationFailed, wireFormat),
       bind(&onInterestValidationFailedWrapper, _1, _2, onValidationFailed, interest),
       2, stepCount + 1);
  else {
    // For interests, we must check that the timestamp is fresh enough.
    // This is done after (possibly) downloading the certificate to avoid filling
    // the cache with bad keys.
    const Name& signatureName = KeyLocator::getFromSignature(signature.get()).getKeyName();
    Name keyName = IdentityCertificate::certificateNameToPublicKeyName(signatureName);
    MillisecondsSince1970 timestamp = interest->getName().get(-4).toNumber();

    if (!interestTimestampIsFresh(keyName, timestamp, failureReason)) {
      try {
        onValidationFailed(interest, failureReason);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
      }
      return ptr_lib::shared_ptr<ValidationRequest>();
    }

    // Certificate is known. Verify the signature.
    // wireEncode returns the cached encoding if available.
    if (verify(signature.get(), interest->wireEncode(), failureReason)) {
      try {
        onVerified(interest);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onVerified: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onVerified.");
      }
      updateTimestampForKey(keyName, timestamp);
    }
    else {
      try {
        onValidationFailed(interest, failureReason);
      } catch (const std::exception& ex) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed: " << ex.what());
      } catch (...) {
        _LOG_ERROR("ConfigPolicyManager::checkVerificationPolicy: Error in onValidationFailed.");
      }
    }

    return ptr_lib::shared_ptr<ValidationRequest>();
  }
}

ptr_lib::shared_ptr<Interest>
ConfigPolicyManager::getCertificateInterest
  (int stepCount, const string& matchType, const Name& objectName,
   const Signature* signature, string& failureReason)
{
  if (stepCount > maxDepth_) {
    ostringstream message;
    message << "The verification stepCount " << stepCount <<
      " exceeded the maxDepth " << maxDepth_;
    failureReason = message.str();
    return ptr_lib::shared_ptr<Interest>();
  }

  // first see if we can find a rule to match this packet
  const BoostInfoTree* matchedRule = findMatchingRule(objectName, matchType);

  // No matching rule -> fail.
  if (!matchedRule) {
    failureReason = "No matching rule found for " + objectName.toUri();
    return ptr_lib::shared_ptr<Interest>();
  }

  // Do a quick check if this is sig-type sha256.
  const BoostInfoTree& checker = *(*matchedRule)["checker"][0];
  const string& checkerType = checker["type"][0]->getValue();
  if (checkerType == "customized" &&
      checker["sig-type"][0]->getValue() == "sha256")
    // The signature is a simple DigestSha256 so we don't fetch certificates.
    return ptr_lib::make_shared<Interest>();

  if (!KeyLocator::canGetFromSignature(signature)) {
    // We only support signature types with key locators.
    failureReason = "The signature type does not support a KeyLocator";
    return ptr_lib::shared_ptr<Interest>();
  }

  const KeyLocator* keyLocator = &KeyLocator::getFromSignature(signature);

  const Name& signatureName = keyLocator->getKeyName();
  // No key name in KeyLocator -> fail.
  if (signatureName.size() == 0) {
    failureReason = "The signature KeyLocator doesn't have a key name";
    return ptr_lib::shared_ptr<Interest>();
  }

  bool signatureMatches = checkSignatureMatch
    (signatureName, objectName, *matchedRule, failureReason);
  if (!signatureMatches)
    return ptr_lib::shared_ptr<Interest>();

  // Before we look up keys, refresh any certificate directories.
  refreshManager_->refreshAnchors();

  // If we don't actually have the certificate yet, return a certificateInterest
  //   for it.
  ptr_lib::shared_ptr<IdentityCertificate> foundCert =
    refreshManager_->getCertificate(signatureName);
  if (!foundCert)
    foundCert = certificateCache_->getCertificate(signatureName);
  if (!foundCert)
    return ptr_lib::make_shared<Interest>(signatureName);
  else
    return ptr_lib::make_shared<Interest>();
}

void
ConfigPolicyManager::onCertificateDownloadComplete
  (const ptr_lib::shared_ptr<Data> &data,
   const ptr_lib::shared_ptr<Data> &originalData, int stepCount,
   const OnVerified& onVerified, 
   const OnDataValidationFailed& onValidationFailed)
{
  IdentityCertificate certificate(*data);
  certificateCache_->insertCertificate(certificate);

  // Now that we stored the needed certificate, increment stepCount and try again
  //   to verify the originalData.
  checkVerificationPolicy
    (originalData, stepCount + 1, onVerified, onValidationFailed);
}

void
ConfigPolicyManager::onCertificateDownloadCompleteForInterest
  (const ptr_lib::shared_ptr<Data> &data,
   const ptr_lib::shared_ptr<Interest> &originalInterest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnInterestValidationFailed& onValidationFailed, WireFormat& wireFormat)
{
  IdentityCertificate certificate(*data);
  certificateCache_->insertCertificate(certificate);

  // Now that we stored the needed certificate, increment stepCount and try again
  //   to verify the originalData.
  checkVerificationPolicy
    (originalInterest, stepCount + 1, onVerified, onValidationFailed, wireFormat);
}

bool
ConfigPolicyManager::checkSigningPolicy
  (const Name& dataName, const Name& certificateName)
{
  return true;
}

Name
ConfigPolicyManager::inferSigningIdentity(const Name& dataName)
{
  throw runtime_error("ConfigPolicyManager::inferSigningIdentity is not implemented");
}

void
ConfigPolicyManager::loadTrustAnchorCertificates()
{
  vector<const BoostInfoTree*> anchors =
    config_->getRoot()["validator/trust-anchor"];

  for (size_t i = 0; i < anchors.size(); ++i) {
    const BoostInfoTree& anchor = *anchors[i];

    const string& typeName = anchor["type"][0]->getValue();
    bool isPath = false;
    string certID;
    if (typeName == "file") {
      certID = anchor["file-name"][0]->getValue();
      isPath = true;
    }
    else if (typeName == "base64") {
      certID = anchor["base64-string"][0]->getValue();
      isPath = false;
    }
    else if (typeName == "dir") {
      const string& dirName = anchor["dir"][0]->getValue();

      double refreshPeriod = 0;
      vector<const BoostInfoTree*> refreshTrees = anchor["refresh"];
      if (refreshTrees.size() >= 1) {
        const string& refreshPeriodStr = refreshTrees[0]->getValue();

        regex regex1("(\\d+)([hms])");
        smatch refreshMatch;
        if (!regex_match(refreshPeriodStr, refreshMatch, regex1))
          refreshPeriod = 0;
        else {
          refreshPeriod = ::atoi(refreshMatch[1].str().c_str());
          if (refreshMatch[2].str() != "s") {
            refreshPeriod *= 60;
            if (refreshMatch[2].str() != "m")
              refreshPeriod *= 60;
          }
        }
      }

      // Convert refreshPeriod from seconds to milliseconds.
      refreshManager_->addDirectory(dirName, refreshPeriod * 1000);
      continue;
    }
    else if (typeName == "any") {
      // This disables all security!
      requiresVerification_ = false;
      break;
    }

    lookupCertificate(certID, isPath);
  }
}

bool
ConfigPolicyManager::checkSignatureMatch
  (const Name& signatureName, const Name& objectName, const BoostInfoTree& rule,
   string& failureReason)
{
  const BoostInfoTree& checker = *rule["checker"][0];
  const string& checkerType = checker["type"][0]->getValue();
  if (checkerType == "fixed-signer") {
    const BoostInfoTree& signerInfo = *checker["signer"][0];
    const string& signerType = signerInfo["type"][0]->getValue();

    ptr_lib::shared_ptr<Certificate> cert;
    if (signerType == "file") {
      cert = lookupCertificate(signerInfo["file-name"][0]->getValue(), true);
      if (!cert) {
        failureReason = "Can't find fixed-signer certificate file: " +
          signerInfo["file-name"][0]->getValue();
        return false;
      }
    }
    else if (signerType == "base64") {
      cert = lookupCertificate(signerInfo["base64-string"][0]->getValue(), false);
      if (!cert) {
        failureReason = "Can't find fixed-signer certificate base64: " +
          signerInfo["base64-string"][0]->getValue();
        return false;
      }
    }
    else {
      failureReason = "Unrecognized fixed-signer signerType: " + signerType;
      return false;
    }

    if (cert->getName().equals(signatureName))
      return true;
    else {
      failureReason = "fixed-signer cert name \"" + cert->getName().toUri() +
        "\" does not equal signatureName \"" + signatureName.toUri() + "\"";
      return false;
    }
  }
  else if (checkerType == "hierarchical") {
    // This just means the data/interest name has the signing identity as a prefix.
    // That means everything before "ksk-?" in the key name.
    string identityRegex = "^([^<KEY>]*)<KEY>(<>*)<ksk-.+><ID-CERT>";
    NdnRegexTopMatcher identityMatch(identityRegex);
    if (identityMatch.match(signatureName)) {
      Name identityPrefix = identityMatch.expand("\\1")
        .append(identityMatch.expand("\\2"));
      if (matchesRelation(objectName, identityPrefix, "is-prefix-of"))
        return true;
      else {
        failureReason = "The hierarchical objectName \"" + objectName.toUri() +
          "\" is not a prefix of \"" + identityPrefix.toUri() + "\"";
        return false;
      }
    }
    else {
      failureReason = "The hierarchical identityRegex \"" + identityRegex +
        "\" does not match signatureName \"" + signatureName.toUri() + "\"";
      return false;
    }
  }
  else if (checkerType == "customized") {
    const BoostInfoTree& keyLocatorInfo = *checker["key-locator"][0];
    // Not checking type - only name is supported.

    // Is this a simple relation?
    const string* relationType = keyLocatorInfo.getFirstValue("relation");
    if (relationType) {
      Name matchName(keyLocatorInfo["name"][0]->getValue());
      if (matchesRelation(signatureName, matchName, *relationType))
        return true;
      else {
        failureReason = "The custom signatureName \"" + signatureName.toUri() +
          "\" does not match matchName \"" + matchName.toUri() +
          "\" using relation " + *relationType;
        return false;
      }
    }

    // Is this a simple regex?
    const string* keyRegex = keyLocatorInfo.getFirstValue("regex");
    if (keyRegex) {
      if (NdnRegexTopMatcher(*keyRegex).match(signatureName))
        return true;
      else {
        failureReason = "The custom signatureName \"" + signatureName.toUri() +
          "\" does not regex match keyRegex \"" + *keyRegex + "\"";
        return false;
      }
    }

    // Is this a hyper-relation?
    vector<const BoostInfoTree*> hyperRelationList = keyLocatorInfo["hyper-relation"];
    if (hyperRelationList.size() >= 1) {
      const BoostInfoTree& hyperRelation = *hyperRelationList[0];

      const string* keyRegex = hyperRelation.getFirstValue("k-regex");
      const string* keyExpansion = hyperRelation.getFirstValue("k-expand");
      const string* nameRegex = hyperRelation.getFirstValue("p-regex");
      const string* nameExpansion = hyperRelation.getFirstValue("p-expand");
      const string* relationType = hyperRelation.getFirstValue("h-relation");
      if (keyRegex && keyExpansion && nameRegex && nameExpansion && relationType) {
        NdnRegexTopMatcher keyMatch(*keyRegex);
        if (!keyMatch.match(signatureName)) {
          failureReason = "The custom hyper-relation signatureName \"" +
            signatureName.toUri() + "\" does not match the keyRegex \"" +
            *keyRegex + "\"";
          return false;
        }
        Name keyMatchPrefix = keyMatch.expand(*keyExpansion);

        NdnRegexTopMatcher nameMatch(*nameRegex);
        if (!nameMatch.match(objectName)) {
          failureReason = "The custom hyper-relation objectName \"" +
            objectName.toUri() + "\" does not match the nameRegex \"" +
            *nameRegex + "\"";
          return false;
        }
        Name nameMatchExpansion = nameMatch.expand(*nameExpansion);

        if (matchesRelation(nameMatchExpansion, keyMatchPrefix, *relationType))
          return true;
        else {
          failureReason = "The custom hyper-relation nameMatch \"" +
            nameMatchExpansion.toUri() + "\" does not match the keyMatchPrefix \"" +
            keyMatchPrefix.toUri() + "\" using relation " + *relationType;
          return false;
        }
      }
    }
  }

  failureReason = "Unrecognized checkerType: " + checkerType;
  return false;
}

ptr_lib::shared_ptr<IdentityCertificate>
ConfigPolicyManager::lookupCertificate(const string& certID, bool isPath)
{
  ptr_lib::shared_ptr<IdentityCertificate> cert;

  map<string, string>::iterator iCertUri = fixedCertificateCache_.find(certID);
  if (iCertUri == fixedCertificateCache_.end()) {
    if (isPath)
      // Load the certificate data (base64 encoded IdentityCertificate)
      cert = TrustAnchorRefreshManager::loadIdentityCertificateFromFile(certID);
    else {
      vector<uint8_t> certData;
      fromBase64(certID.c_str(), certData);
      cert.reset(new IdentityCertificate());
      cert->wireDecode(certData);
    }

    string certUri = cert->getName().getPrefix(-1).toUri();
    fixedCertificateCache_[certID] = certUri;
    certificateCache_->insertCertificate(*cert);
  }
  else
    cert = certificateCache_->getCertificate(Name(iCertUri->second));

  return cert;
}

const BoostInfoTree*
ConfigPolicyManager::findMatchingRule
  (const Name& objName, const string& matchType) const
{
  vector<const BoostInfoTree*> rules = config_->getRoot()["validator/rule"];
  for (size_t iRule = 0; iRule < rules.size(); ++iRule) {
    const BoostInfoTree& r = *rules[iRule];

    if (r["for"][0]->getValue() == matchType) {
      bool passed = true;
      vector<const BoostInfoTree*> filters = r["filter"];
      if (filters.size() == 0)
        // no filters means we pass!
        return &r;
      else {
        for (size_t iFilter = 0; iFilter < filters.size(); ++iFilter) {
          const BoostInfoTree& f = *filters[iFilter];

          // Don't check the type - it can only be name for now.
          // We need to see if this is a regex or a relation.
          const string* regexPattern = f.getFirstValue("regex");
          if (!regexPattern) {
            const string& matchRelation = f["relation"][0]->getValue();
            const string& matchUri = f["name"][0]->getValue();
            Name matchName(matchUri);
            passed = matchesRelation(objName, matchName, matchRelation);
          }
          else
            passed = NdnRegexTopMatcher(*regexPattern).match(objName);

          if (!passed)
            break;
        }

        if (passed)
          return &r;
      }
    }
  }

  return 0;
}

bool
ConfigPolicyManager::matchesRelation
  (const Name& name, const Name& matchName, const string& matchRelation)
{
  bool passed = false;
  if (matchRelation == "is-strict-prefix-of") {
    if (matchName.size() == name.size())
      passed = false;
    else if (matchName.match(name))
      passed = true;
  }
  else if (matchRelation == "is-prefix-of") {
    if (matchName.match(name))
      passed = true;
  }
  else if (matchRelation == "equal") {
    if (matchName.equals(name))
      passed = true;
  }

  return passed;
}

ptr_lib::shared_ptr<Signature>
ConfigPolicyManager::extractSignature
  (const Interest& interest, WireFormat& wireFormat, string& failureReason)
{
  if (interest.getName().size() < 2) {
    failureReason = "The signed interest has less than 2 components: " +
      interest.getName().toUri();
    return ptr_lib::shared_ptr<Signature>();
  }

  try {
    return wireFormat.decodeSignatureInfoAndValue
      (interest.getName().get(-2).getValue(),
       interest.getName().get(-1).getValue());
  } catch (std::exception& e) {
    failureReason = string("Error decoding the signed interest signature: ") +
      e.what();
    return ptr_lib::shared_ptr<Signature>();
  }
}

bool
ConfigPolicyManager::interestTimestampIsFresh
  (const Name& keyName, MillisecondsSince1970 timestamp,
   string& failureReason) const
{
  map<string, MillisecondsSince1970>::const_iterator lastTimestamp =
    keyTimestamps_.find(keyName.toUri());
  if (lastTimestamp == keyTimestamps_.end()) {
    MillisecondsSince1970 now = ndn_getNowMilliseconds();
    MillisecondsSince1970 notBefore = now - keyGraceInterval_;
    MillisecondsSince1970 notAfter = now + keyGraceInterval_;

    if (!(timestamp > notBefore && timestamp < notAfter)) {
      ostringstream message;
      message <<
        "The command interest timestamp is not within the first use grace period of " <<
        keyGraceInterval_ << " milliseconds.";
      failureReason = message.str();
      return false;
    }
    else
      return true;
  }
  else {
    if (timestamp <= lastTimestamp->second) {
      failureReason =
        "The command interest timestamp is not newer than the previous timestamp";
      return false;
    }
    else
      return true;
  }
}

void
ConfigPolicyManager::updateTimestampForKey
  (const Name& keyName, MillisecondsSince1970 timestamp)
{
  keyTimestamps_[keyName.toUri()] = timestamp;

  if (keyTimestamps_.size() >= maxTrackedKeys_) {
    MillisecondsSince1970 now = ndn_getNowMilliseconds();
    MillisecondsSince1970 oldestTimestamp = now;
    string oldestKey;

    // Get the keys to erase without disturbing the map.
    vector<string> keysToErase;

    for (map<string, MillisecondsSince1970>::iterator entry = keyTimestamps_.begin();
         entry != keyTimestamps_.end(); ++entry) {
      const string& keyUri = entry->first;
      MillisecondsSince1970 ts = entry->second;
      if (now - ts > keyTimestampTtl_)
        keysToErase.push_back(keyUri);
      else if (ts < oldestTimestamp) {
        oldestTimestamp = ts;
        oldestKey = keyUri;
      }
    }

    // Now erase.
    for (size_t i = 0; i < keysToErase.size(); ++i)
      keyTimestamps_.erase(keysToErase[i]);

    if (keyTimestamps_.size() > maxTrackedKeys_ && oldestKey != "")
      // have not removed enough
      keyTimestamps_.erase(oldestKey);
  }
}

bool
ConfigPolicyManager::verify
  (const Signature* signatureInfo, const SignedBlob& signedBlob,
   string& failureReason) const
{
  // We have already checked once if there should be a key locator.
  if (!KeyLocator::canGetFromSignature(signatureInfo)) {
    // There is no KeyLocator, so assume it is not needed (such as DigestSha256).
    if (verifySignature(signatureInfo, signedBlob, Blob()))
      return true;
    else {
      failureReason = "The non-KeyLocator signature did not verify";
      return false;
    }
  }

  const KeyLocator& keyLocator = KeyLocator::getFromSignature(signatureInfo);

  if (keyLocator.getType() == ndn_KeyLocatorType_KEYNAME) {
    // Assume the key name is a certificate name.
    Name signatureName = keyLocator.getKeyName();
    ptr_lib::shared_ptr<IdentityCertificate> certificate =
      refreshManager_->getCertificate(signatureName);
    if (!certificate)
      certificate = certificateCache_->getCertificate(signatureName);
    if (!certificate) {
      failureReason = "Cannot find a certificate with name " +
        signatureName.toUri();
      return false;
    }

    Blob publicKeyDer = certificate->getPublicKeyInfo().getKeyDer();
    if (publicKeyDer.isNull()) {
      // We don't expect this to happen.
      failureReason = "There is no public key in the certificate with name " +
        certificate->getName().toUri();
      return false;
    }

    if (verifySignature(signatureInfo, signedBlob, publicKeyDer))
      return true;
    else {
      failureReason = "The signature did not verify with the given public key";
      return false;
    }
  }
  else {
    failureReason = "The KeyLocator does not have a key name";
    return false;
  }
}

ptr_lib::shared_ptr<IdentityCertificate>
ConfigPolicyManager::TrustAnchorRefreshManager::loadIdentityCertificateFromFile
  (const string& filename)
{
  ifstream certFile(filename.c_str());

  stringstream encodedData;
  encodedData << certFile.rdbuf();

  // Use a vector in a shared_ptr so we can make it a Blob without copying.
  ptr_lib::shared_ptr<vector<uint8_t> > decodedData(new vector<uint8_t>());
  fromBase64(encodedData.str(), *decodedData);

  ptr_lib::shared_ptr<IdentityCertificate> cert(new IdentityCertificate());
  cert->wireDecode(Blob(decodedData, false));
  return cert;
}

void
ConfigPolicyManager::TrustAnchorRefreshManager::addDirectory
  (const string& directoryName, Milliseconds refreshPeriod)
{
  DIR *directory = ::opendir(directoryName.c_str());
  if (directory == NULL)
    throw SecurityException
      ("ConfigPolicyManager::TrustAnchorRefreshManager::addDirectory: Cannot open the directory.");

  vector<string> certificateNames;
  struct dirent *entry;
  while ((entry = ::readdir(directory)) != NULL) {
    // TODO: Handle non-unix file systems which don't have stat.
    string fullPath = directoryName + '/' + entry->d_name;
    struct stat fileStat;
    if (::stat(fullPath.c_str(), &fileStat) == -1)
      throw SecurityException
        ("ConfigPolicyManager::TrustAnchorRefreshManager::addDirectory: Cannot stat the file.");
    if (!S_ISREG(fileStat.st_mode))
      continue;

    ptr_lib::shared_ptr<IdentityCertificate> cert;
    try {
      cert = loadIdentityCertificateFromFile(fullPath);
    }
    catch (SecurityException& ex) {
      // Allow files that are not certificates.
      continue;
    }

    // Cut off the timestamp so it matches KeyLocator Name format.
    string certUri = cert->getName().getPrefix(-1).toUri();
    certificateCache_.insertCertificate(*cert);
    certificateNames.push_back(certUri);
  }

  ::closedir(directory);

  refreshDirectories_[directoryName] = ptr_lib::make_shared<DirectoryInfo>
    (certificateNames, ndn_getNowMilliseconds() + refreshPeriod,
     refreshPeriod);
}

void
ConfigPolicyManager::TrustAnchorRefreshManager::refreshAnchors()
{
  MillisecondsSince1970 refreshTime = ndn_getNowMilliseconds();
  // Save info in a list for calling addDirectory later so that it doesn't
  //   modify refreshDirectories_ while we are iterating.
  vector<string> directoriesToAdd;
  vector<MillisecondsSince1970> refreshPeriodsToAdd;

  for (map<string, ptr_lib::shared_ptr<DirectoryInfo> >::iterator it =
        refreshDirectories_.begin(); it != refreshDirectories_.end(); ++it) {
    const string& directory = it->first;
    const DirectoryInfo& info = *(it->second);

    MillisecondsSince1970 nextRefreshTime = info.nextRefresh_;
    if (nextRefreshTime <= refreshTime) {
      vector<string> certificateList(info.certificateNames_);

      // Delete the certificates associated with this directory if possible
      //   then re-import.
      // IdentityStorage subclasses may not support deletion.
      for (size_t i = 0; i < certificateList.size(); ++i)
        certificateCache_.deleteCertificate(Name(certificateList[i]));

      directoriesToAdd.push_back(directory);
      refreshPeriodsToAdd.push_back(info.refreshPeriod_);
    }
  }

  // Now that we are done with the iterator, add to refreshDirectories_.
  for (size_t i = 0; i < directoriesToAdd.size(); ++i)
    addDirectory(directoriesToAdd[i], refreshPeriodsToAdd[i]);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
