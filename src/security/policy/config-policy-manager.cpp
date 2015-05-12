/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/certificate/identity-certificate.hpp>
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

/**
 * Ignore data and call onVerifyFailed(interest). This is so that an
 * OnVerifyInterestFailed can be passed as an OnVerifyFailed.
 */
static void
onVerifyInterestFailedWrapper
  (const ptr_lib::shared_ptr<Data>& data,
   const OnVerifyInterestFailed& onVerifyFailed,
   const ptr_lib::shared_ptr<Interest>& interest)
{
  onVerifyFailed(interest);
}

ConfigPolicyManager::ConfigPolicyManager
  (const string& configFileName,
   const ptr_lib::shared_ptr<CertificateCache>& certificateCache,
   int searchDepth, Milliseconds graceInterval, Milliseconds keyTimestampTtl,
   int maxTrackedKeys)
  : maxDepth_(searchDepth),
    keyGraceInterval_(graceInterval),
    keyTimestampTtl_(keyTimestampTtl),
    maxTrackedKeys_(maxTrackedKeys),
    config_(new BoostInfoParser()),
    requiresVerification_(true)
{
  if (!certificateCache)
    certificateCache_.reset(new CertificateCache());
  else
    certificateCache_ = certificateCache;

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
  ptr_lib::shared_ptr<Interest> certificateInterest = getCertificateInterest
    (stepCount, "data", data->getName(), data->getSignature());
  if (!certificateInterest) {
    onVerifyFailed(data);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  if (certificateInterest->getName().size() > 0)
    return ptr_lib::make_shared<ValidationRequest>
      (certificateInterest,
       bind(&ConfigPolicyManager::onCertificateDownloadComplete, this, _1,
            data, stepCount, onVerified, onVerifyFailed),
       onVerifyFailed, 2, stepCount + 1);
  else {
    // Certificate is known. Verify the signature.
    // wireEncode returns the cached encoding if available.
    if (verify(data->getSignature(), data->wireEncode()))
      onVerified(data);
    else
      onVerifyFailed(data);

    return ptr_lib::shared_ptr<ValidationRequest>();
  }
}

ptr_lib::shared_ptr<ValidationRequest>
ConfigPolicyManager::checkVerificationPolicy
  (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat)
{
  ptr_lib::shared_ptr<Signature> signature = extractSignature
    (*interest, wireFormat);
  if (!signature) {
    // Can't get the signature from the interest name.
    onVerifyFailed(interest);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  // For command interests, we need to ignore the last 4 components when
  //   matching the name.
  ptr_lib::shared_ptr<Interest> certificateInterest = getCertificateInterest
    (stepCount, "interest", interest->getName().getPrefix(-4), signature.get());
  if (!certificateInterest) {
    onVerifyFailed(interest);
    return ptr_lib::shared_ptr<ValidationRequest>();
  }

  if (certificateInterest->getName().size() > 0)
    return ptr_lib::make_shared<ValidationRequest>
      (certificateInterest,
       bind(&ConfigPolicyManager::onCertificateDownloadCompleteForInterest, this, _1,
            interest, stepCount, onVerified, onVerifyFailed, wireFormat),
       bind(&onVerifyInterestFailedWrapper, _1, onVerifyFailed, interest),
       2, stepCount + 1);
  else {
    // For interests, we must check that the timestamp is fresh enough.
    // This is done after (possibly) downloading the certificate to avoid filling
    // the cache with bad keys.
    const Name& signatureName = KeyLocator::getFromSignature(signature.get()).getKeyName();
    Name keyName = IdentityCertificate::certificateNameToPublicKeyName(signatureName);
    MillisecondsSince1970 timestamp = interest->getName().get(-4).toNumber();

    if (!interestTimestampIsFresh(keyName, timestamp)) {
      onVerifyFailed(interest);
      return ptr_lib::shared_ptr<ValidationRequest>();
    }

    // Certificate is known. Verify the signature.
    // wireEncode returns the cached encoding if available.
    if (verify(signature.get(), interest->wireEncode())) {
      onVerified(interest);
      updateTimestampForKey(keyName, timestamp);
    }
    else
      onVerifyFailed(interest);

    return ptr_lib::shared_ptr<ValidationRequest>();
  }
}

ptr_lib::shared_ptr<Interest>
ConfigPolicyManager::getCertificateInterest
  (int stepCount, const string& matchType, const Name& objectName,
   const Signature* signature)
{
  if (stepCount > maxDepth_)
    return ptr_lib::shared_ptr<Interest>();

  if (!KeyLocator::canGetFromSignature(signature))
    // We only support signature types with key locators.
    return ptr_lib::shared_ptr<Interest>();

  const KeyLocator* keyLocator;
  try {
    keyLocator = &KeyLocator::getFromSignature(signature);
  }
  catch (std::exception& ex) {
    // No signature -> fail.
    return ptr_lib::shared_ptr<Interest>();
  }

  const Name& signatureName = keyLocator->getKeyName();
  // No key name in KeyLocator -> fail.
  if (signatureName.size() == 0)
    return ptr_lib::shared_ptr<Interest>();

  // first see if we can find a rule to match this packet
  const BoostInfoTree* matchedRule = findMatchingRule(objectName, matchType);

  // No matching rule -> fail.
  if (!matchedRule)
    return ptr_lib::shared_ptr<Interest>();

  bool signatureMatches = checkSignatureMatch
    (signatureName, objectName, *matchedRule);
  if (!signatureMatches)
    return ptr_lib::shared_ptr<Interest>();

  // Before we look up keys, refresh any certificate directories.
  refreshManager_.refreshAnchors();

  // If we don't actually have the certificate yet, return a certificateInterest
  //   for it.
  ptr_lib::shared_ptr<IdentityCertificate> foundCert =
    refreshManager_.getCertificate(signatureName);
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
   const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed)
{
  IdentityCertificate certificate(*data);
  certificateCache_->insertCertificate(certificate);

  // Now that we stored the needed certificate, increment stepCount and try again
  //   to verify the originalData.
  checkVerificationPolicy(originalData, stepCount + 1, onVerified, onVerifyFailed);
}

void
ConfigPolicyManager::onCertificateDownloadCompleteForInterest
  (const ptr_lib::shared_ptr<Data> &data,
   const ptr_lib::shared_ptr<Interest> &originalInterest, int stepCount,
   const OnVerifiedInterest& onVerified,
   const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat)
{
  IdentityCertificate certificate(*data);
  certificateCache_->insertCertificate(certificate);

  // Now that we stored the needed certificate, increment stepCount and try again
  //   to verify the originalData.
  checkVerificationPolicy
    (originalInterest, stepCount + 1, onVerified, onVerifyFailed, wireFormat);
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
  (const Interest& interest, WireFormat& wireFormat)
{
  if (interest.getName().size() < 2)
    return ptr_lib::shared_ptr<Signature>();

  try {
    return wireFormat.decodeSignatureInfoAndValue
      (interest.getName().get(-2).getValue(),
       interest.getName().get(-1).getValue());
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
  // We have already checked once that there is a key locator.
  const KeyLocator& keyLocator = KeyLocator::getFromSignature(signatureInfo);

  if (keyLocator.getType() == ndn_KeyLocatorType_KEYNAME) {
    // Assume the key name is a certificate name.
    Name signatureName = keyLocator.getKeyName();
    ptr_lib::shared_ptr<IdentityCertificate> certificate =
      refreshManager_.getCertificate(signatureName);
    if (!certificate)
      certificate = certificateCache_->getCertificate(signatureName);
    if (!certificate)
        return false;

    Blob publicKeyDer = certificate->getPublicKeyInfo().getKeyDer();
    if (publicKeyDer.isNull())
      // Can't find the public key with the name.
      return false;

    return verifySignature(signatureInfo, signedBlob, publicKeyDer);
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
  for (int i = 0; i < directoriesToAdd.size(); ++i)
    addDirectory(directoriesToAdd[i], refreshPeriodsToAdd[i]);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
