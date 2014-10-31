/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
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

#include <stdexcept>
#include <fstream>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/certificate/identity-certificate.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include "../../util/ndn-regex-matcher.hpp"
#include "../../util/boost-info-parser.hpp"
#include "../../c/util/time.h"
#include "../../encoding/base64.hpp"
#include <ndn-cpp/security/policy/config-policy-manager.hpp>

// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

using namespace std;
using namespace ndn::func_lib;

namespace ndn {

using namespace regex_lib;

ConfigPolicyManager::ConfigPolicyManager
  (IdentityStorage* identityStorage, const string& configFileName,
   int searchDepth, Milliseconds graceInterval, Milliseconds keyTimestampTtl,
   int maxTrackedKeys)
  : identityStorage_(identityStorage),
    maxDepth_(searchDepth),
    keyGraceInterval_(graceInterval),
    keyTimestampTtl_(keyTimestampTtl),
    maxTrackedKeys_(1000),
    config_(new BoostInfoParser()),
    requiresVerification_(true),
    refreshManager_(identityStorage)
{
  config_->read(configFileName);
  loadTrustAnchorCertificates();
}

ConfigPolicyManager::~ConfigPolicyManager()
{
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
   const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed)
{
  if (stepCount > maxDepth_) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  const Sha256WithRsaSignature *signature =
    dynamic_cast<const Sha256WithRsaSignature *>(data->getSignature());
  // No signature -> fail.
  if (!signature) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  const Name& signatureName = signature->getKeyLocator().getKeyName();
  // No key name in KeyLocator -> fail.
  if (signatureName.size() == 0) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  const Name& objectName = data->getName();
  string matchType = "data";

#if 0 // for checkVerificationPolicy(Interest)
  // For command interests, we need to ignore the last 4 components when
  //   matching the name
  objectName = objectName.getPrefix(-4);
  matchType = "interest";
#endif

  // first see if we can find a rule to match this packet
  const BoostInfoTree* matchedRule = findMatchingRule(objectName, matchType);

  // No matching rule -> fail.
  if (!matchedRule) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  bool signatureMatches = checkSignatureMatch
    (signatureName, objectName, *matchedRule);
  if (!signatureMatches) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  // Before we look up keys, refresh any certificate directories.
  refreshManager_.refreshAnchors();

  // Now finally check that the object was signed correctly.
  // If we don't actually have the certificate yet, create a
  //   ValidationRequest for it.
  if (!identityStorage_->doesCertificateExist(signatureName)) {
    ptr_lib::shared_ptr<Interest> certificateInterest(new Interest(signatureName));
    return ptr_lib::make_shared<ValidationRequest>
      (certificateInterest, 
       bind(&ConfigPolicyManager::onCertificateDownloadComplete, this, _1,
            data, stepCount, onVerified, onVerifyFailed),
       onVerifyFailed, 2, stepCount + 1);
  }
  else {
#if 0 // for checkVerificationPolicy(Interest)
    // For interests, we must check that the timestamp is fresh enough.
    // I do this after (possibly) downloading the certificate to avoid filling
    // the cache with bad keys.
    keyName = IdentityCertificate.certificateNameToPublicKeyName(signatureName);
    timestamp = dataOrInterest.getName().get(-4).toNumber() / 1000;

    if (!interestTimestampIsFresh(keyName, timestamp)) {
      onVerifyFailed(dataOrInterest);
      return ptr_lib::shared_ptr<ValidationRequest>();
    }
#endif

    // Certificate is known. Verify the signature.
    // wireEncode returns the cached encoding if available.
    if (verify(signature, data->wireEncode())) {
      onVerified(data);
#if 0 // for checkVerificationPolicy(Interest)
      updateTimestampForKey(keyName, timestamp);
#endif
    }
    else
      onVerifyFailed(data);

    return ptr_lib::shared_ptr<ValidationRequest>();
  }
}

void
ConfigPolicyManager::onCertificateDownloadComplete
  (const ptr_lib::shared_ptr<Data> &data,
   const ptr_lib::shared_ptr<Data> &originalData, int stepCount,
   const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed)
{
  IdentityCertificate certificate(*data);
  identityStorage_->addCertificate(certificate);
  // Now that we stored the needed certificate, increment stepCount and try again
  //   to verify the originalData.
  checkVerificationPolicy(originalData, stepCount + 1, onVerified, onVerifyFailed);
}

ptr_lib::shared_ptr<ValidationRequest>
ConfigPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat)
{
  throw runtime_error("ConfigPolicyManager::checkVerificationPolicy(Interest) is not implemented");
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
        if (regex_match(refreshPeriodStr, refreshMatch, regex1))
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
      refreshManager_.addDirectory(dirName, refreshPeriod * 1000);
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
  (const Name& signatureName, const Name& objectName, const BoostInfoTree& rule)
{
  const BoostInfoTree& checker = *rule["checker"][0];
  const string& checkerType = checker["type"][0]->getValue();
  if (checkerType == "fixed-signer") {
    const BoostInfoTree& signerInfo = *checker["signer"][0];
    const string& signerType = signerInfo["type"][0]->getValue();

    ptr_lib::shared_ptr<Certificate> cert;
    if (signerType == "file")
      cert = lookupCertificate(signerInfo["file-name"][0]->getValue(), true);
    else if (signerType == "base64")
      cert = lookupCertificate(signerInfo["base64-string"][0]->getValue(), false);
    else
      return false;

    if (!cert)
      return false;
    else
      return cert->getName().equals(signatureName);
  }
  else if (checkerType == "hierarchical") {
    // This just means the data/interest name has the signing identity as a prefix.
    // That means everything before "ksk-?" in the key name.
    string identityRegex = "^([^<KEY>]*)<KEY>(<>*)<ksk-.+><ID-CERT>";
    NdnRegexMatcher identityMatch(identityRegex, signatureName);
    if (identityMatch.iterator != sregex_iterator()) {
      Name identityPrefix = Name(identityMatch.iterator->str(1)).append
        (Name(identityMatch.iterator->str(2)));
      return matchesRelation(objectName, identityPrefix, "is-prefix-of");
    }
    else
      return false;
  }
  else if (checkerType == "customized") {
    const BoostInfoTree& keyLocatorInfo = *checker["key-locator"][0];
    // Not checking type - only name is supported.

    // Is this a simple relation?
    const string* relationType = keyLocatorInfo.getFirstValue("relation");
    if (relationType) {
      Name matchName(keyLocatorInfo["name"][0]->getValue());
      return matchesRelation(signatureName, matchName, *relationType);
    }

    // Is this a simple regex?
    const string* keyRegex = keyLocatorInfo.getFirstValue("regex");
    if (keyRegex)
      return NdnRegexMatcher(*keyRegex, signatureName).iterator != sregex_iterator();

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
        NdnRegexMatcher keyMatch(*keyRegex, signatureName);
        if (keyMatch.iterator->size() == 0)
          return false;
        // format_sed will substitute with \1 instead of $1.
        string keyMatchPrefix = keyMatch.iterator->format(*keyExpansion, regex_constants::format_sed);
        
        NdnRegexMatcher nameMatch(*nameRegex, objectName);
        if (nameMatch.iterator->size() == 0)
          return false;
        string nameMatchStr = nameMatch.iterator->format(*nameExpansion, regex_constants::format_sed);

        return matchesRelation
          (Name(nameMatchStr), Name(keyMatchPrefix), *relationType);
      }
    }
  }

  // unknown type
  return false;
}

ptr_lib::shared_ptr<IdentityCertificate>
ConfigPolicyManager::lookupCertificate(const string& certID, bool isPath)
{
  ptr_lib::shared_ptr<IdentityCertificate> cert;

  map<string, string>::iterator certUri = certificateCache_.find(certID);
  if (certUri == certificateCache_.end()) {
    if (isPath)
      // Load the certificate data (base64 encoded IdentityCertificate)
      cert = refreshManager_.loadIdentityCertificateFromFile(certID);
    else {
      vector<uint8_t> certData;
      fromBase64(certID.c_str(), certData);
      cert.reset(new IdentityCertificate());
      cert->wireDecode(certData);
    }
    
    certificateCache_[certID] = cert->getName().toUri();
    try {
      identityStorage_->addCertificate(*cert);
    }
    catch (const SecurityException& ex) {
      // Already exists? It's okay.
    }
  }
  else
    cert = identityStorage_->getCertificate(Name(certUri->second));

  return cert;
}

const BoostInfoTree*
ConfigPolicyManager::findMatchingRule
  (const Name& objName, const string& matchType) const
{
  vector<const BoostInfoTree*> rules = config_->getRoot()["validator/rule"];
  for (int iRule = 0; iRule < rules.size(); ++iRule) {
    const BoostInfoTree& r = *rules[iRule];
    
    if (r["for"][0]->getValue() == matchType) {
      bool passed = true;
      vector<const BoostInfoTree*> filters = r["filter"];
      if (filters.size() == 0)
        // no filters means we pass!
        return &r;
      else {
        for (int iFilter = 0; iFilter < filters.size(); ++iFilter) {
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
            passed = (NdnRegexMatcher(*regexPattern, objName).iterator !=
                      sregex_iterator());

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
  (const Name& name, const Name& matchName, const string& matchRelation) const
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
  (const Interest& interest, WireFormat& wireFormat)
{
  if (interest.getName().size() < 2)
    return ptr_lib::shared_ptr<Signature>();

  try {
    return wireFormat.decodeSignatureInfoAndValue
      (interest.getName().get(-2).getValue().buf(),
       interest.getName().get(-2).getValue().size(),
       interest.getName().get(-1).getValue().buf(),
       interest.getName().get(-1).getValue().size());
  } catch (std::exception& e) {
    return ptr_lib::shared_ptr<Signature>();
  }
}

bool
ConfigPolicyManager::interestTimestampIsFresh
  (const Name& keyName, MillisecondsSince1970 timestamp) const
{
  map<string, MillisecondsSince1970>::const_iterator lastTimestamp =
    keyTimestamps_.find(keyName.toUri());
  if (lastTimestamp == keyTimestamps_.end()) {
    MillisecondsSince1970 now = ndn_getNowMilliseconds();
    MillisecondsSince1970 notBefore = now - keyGraceInterval_;
    MillisecondsSince1970 notAfter = now + keyGraceInterval_;
    return timestamp > notBefore && timestamp < notAfter;
  }
  else
    return timestamp > lastTimestamp->second;
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
    for (int i = 0; i < keysToErase.size(); ++i)
      keyTimestamps_.erase(keysToErase[i]);

    if (keyTimestamps_.size() > maxTrackedKeys_ && oldestKey != "")
      // have not removed enough
      keyTimestamps_.erase(oldestKey);
  }
}

bool
ConfigPolicyManager::verify
  (const Signature* signatureInfo, const SignedBlob& signedBlob) const
{
  const Sha256WithRsaSignature *signature =
    dynamic_cast<const Sha256WithRsaSignature *>(signatureInfo);
  if (!signature)
    throw SecurityException
      ("ConfigPolicyManager: Signature is not Sha256WithRsaSignature.");

  if (signature->getKeyLocator().getType() == ndn_KeyLocatorType_KEYNAME &&
      identityStorage_) {
    // Assume the key name is a certificate name.
    Blob publicKeyDer = identityStorage_->getKey
      (IdentityCertificate::certificateNameToPublicKeyName
       (signature->getKeyLocator().getKeyName()));
    if (!publicKeyDer)
      // Can't find the public key with the name.
      return false;

    return verifySha256WithRsaSignature(signature, signedBlob, publicKeyDer);
  }
  else
    // Can't find a key to verify.
    return false;
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
  // TODO: Implement.
  throw runtime_error("ConfigPolicyManager::TrustAnchorRefreshManager::addDirectory is not implemented");
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
      for (size_t i = 0; i < certificateList.size(); ++i) {
        try {
          identityStorage_->deleteCertificateInfo(Name(certificateList[i]));
        }
        catch (const SecurityException& ex) {
          // Was already removed? Not supported?
        }
        catch (const std::exception& ex) {
          // Not implemented.
          break;
        }
      }

      directoriesToAdd.push_back(directory);
      refreshPeriodsToAdd.push_back(info.refreshPeriod_);
    }
  }

  // Now that we are done with the iterator, add to refreshDirectories_.
  for (int i = 0; i < directoriesToAdd.size(); ++i)
    addDirectory(directoriesToAdd[i], refreshPeriodsToAdd[i]);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
