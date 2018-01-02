/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validation-policy-config.cpp
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

#include "../../util/regex/ndn-regex-matcher-base.hpp"
// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher-base.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

#include "../../util/regex/ndn-regex-matcher-base.hpp"

#include <stdlib.h>
#include <stdexcept>
#include "../../util/boost-info-parser.hpp"
#include "../../encoding/base64.hpp"
#include <ndn-cpp/security/validator-config-error.hpp>
#include <ndn-cpp/security/v2/validator.hpp>
#include <ndn-cpp/security/v2/validation-policy-config.hpp>

using namespace std;

namespace ndn {

void
ValidationPolicyConfig::load(const string& filePath)
{
  BoostInfoParser parser;
  parser.read(filePath);
  load(parser.getRoot(), filePath);
}

void
ValidationPolicyConfig::load(const string& input, const string& inputName)
{
  BoostInfoParser parser;
  parser.read(input, inputName);
  load(parser.getRoot(), inputName);
}

void
ValidationPolicyConfig::load
  (const BoostInfoTree& configSection, const string& inputName)
{
  if (isConfigured_) {
    // Reset the previous configuration.
    shouldBypass_ = false;
    dataRules_.clear();
    interestRules_.clear();

    validator_->resetAnchors();
    validator_->resetVerifiedCertificates();
  }
  isConfigured_ = true;

  vector<const BoostInfoTree*> validatorList = configSection["validator"];
  if (validatorList.size() != 1)
    throw ValidatorConfigError
      ("ValidationPolicyConfig: Expected one validator section");
  const BoostInfoTree& validatorSection = *validatorList[0];

  // Get the rules.
  vector<const BoostInfoTree*> ruleList = validatorSection["rule"];
  for (size_t i = 0; i < ruleList.size(); ++i) {
    ptr_lib::shared_ptr<ConfigRule> rule = ConfigRule::create(*ruleList[i]);
    if (rule->getIsForInterest())
      interestRules_.push_back(rule);
    else
      dataRules_.push_back(rule);
  }

  // Get the trust anchors.
  vector<const BoostInfoTree*> trustAnchorList = validatorSection["trust-anchor"];
  for (size_t i = 0; i < trustAnchorList.size(); ++i)
    processConfigTrustAnchor(*trustAnchorList[i], inputName);
}

void
ValidationPolicyConfig::checkPolicy
  (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  if (hasInnerPolicy())
    throw ValidatorConfigError
      ("ValidationPolicyConfig must be a terminal inner policy");

  if (shouldBypass_) {
    continueValidation(ptr_lib::shared_ptr<CertificateRequest>(), state);
    return;
  }

  Name keyLocatorName = getKeyLocatorName(data, *state);
  if (state->isOutcomeFailed())
    // Already called state.fail() .
    return;

  for (size_t i = 0; i < dataRules_.size(); ++i) {
    ConfigRule& rule = *dataRules_[i];

    if (rule.match(false, data.getName())) {
      if (rule.check(false, data.getName(), keyLocatorName, state)) {
        continueValidation
          (ptr_lib::make_shared<CertificateRequest>(Interest(keyLocatorName)),
           state);
        return;
      }
      else
        // rule.check failed and already called state.fail() .
        return;
    }
  }

  state->fail(ValidationError(ValidationError::POLICY_ERROR,
    "No rule matched for data `" + data.getName().toUri() + "`"));
}

void
ValidationPolicyConfig::checkPolicy
  (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
   const ValidationContinuation& continueValidation)
{
  if (hasInnerPolicy())
    throw ValidatorConfigError
      ("ValidationPolicyConfig must be a terminal inner policy");

  if (shouldBypass_) {
    continueValidation(ptr_lib::shared_ptr<CertificateRequest>(), state);
    return;
  }

  Name keyLocatorName = getKeyLocatorName(interest, *state);
  if (state->isOutcomeFailed())
    // Already called state.fail() .
    return;

  for (size_t i = 0; i < interestRules_.size(); ++i) {
    ConfigRule& rule = *interestRules_[i];

    if (rule.match(true, interest.getName())) {
      if (rule.check(true, interest.getName(), keyLocatorName, state)) {
        continueValidation
          (ptr_lib::make_shared<CertificateRequest>(Interest(keyLocatorName)),
           state);
        return;
      }
      else
        // rule.check failed and already called state.fail() .
        return;
    }
  }

  state->fail(ValidationError(ValidationError::POLICY_ERROR,
    "No rule matched for interest `" + interest.getName().toUri() + "`"));
}

void
ValidationPolicyConfig::processConfigTrustAnchor
  (const BoostInfoTree& configSection, const string& inputName)
{
  const string* anchorType = configSection.getFirstValue("type");
  if (!anchorType)
    throw ValidatorConfigError("Expected <trust-anchor.type>");

  if (equalsIgnoreCase(*anchorType, "file")) {
    // Get trust-anchor.file .
    const string* fileName = configSection.getFirstValue("file-name");
    if (!fileName)
      throw ValidatorConfigError("Expected <trust-anchor.file-name>");

    Milliseconds refreshPeriod = getRefreshPeriod(configSection);
    validator_->loadAnchor(*fileName, *fileName, refreshPeriod, false);

    return;
  }
  else if (equalsIgnoreCase(*anchorType, "base64")) {
    // Get trust-anchor.base64-string .
    const string* base64String = configSection.getFirstValue("base64-string");
    if (!base64String)
      throw ValidatorConfigError("Expected <trust-anchor.base64-string>");

    vector<uint8_t> encoding;
    fromBase64(base64String->c_str(), encoding);
    CertificateV2 certificate;
    try {
      certificate.wireDecode(Blob(encoding));
    } catch (const std::exception& ex) {
      throw ValidatorConfigError("Cannot decode certificate from base64-string");
    }
    validator_->loadAnchor("", certificate);

    return;
  }
  else if (equalsIgnoreCase(*anchorType, "dir")) {
    // Get trust-anchor.dir .
    const string* dirString = configSection.getFirstValue("dir");
    if (!dirString)
      throw ValidatorConfigError("Expected <trust-anchor.dir>");

    Milliseconds refreshPeriod = getRefreshPeriod(configSection);
    validator_->loadAnchor(*dirString, *dirString, refreshPeriod, true);

    return;
  }
  else if (equalsIgnoreCase(*anchorType, "any"))
    shouldBypass_ = true;
  else
    throw ValidatorConfigError("Unsupported trust-anchor.type");
}

Milliseconds
ValidationPolicyConfig::getRefreshPeriod(const BoostInfoTree& configSection)
{
  const string* refreshString = configSection.getFirstValue("refresh");
  if (!refreshString)
    // Return a large value (effectively no refresh).
    return 1e14;

  double refreshSeconds = 0;
  regex_lib::regex regex1("(\\d+)([hms])");
  regex_lib::smatch refreshMatch;
  if (regex_match(*refreshString, refreshMatch, regex1)) {
    refreshSeconds = ::atoi(refreshMatch[1].str().c_str());
    if (refreshMatch[2].str() != "s") {
      refreshSeconds *= 60;
      if (refreshMatch[2].str() != "m")
        refreshSeconds *= 60;
    }
  }

  if (refreshSeconds == 0.0)
    // Use an hour instead of 0.
    return 3600 * 1000.0;
  else
    // Convert from seconds to milliseconds.
    return refreshSeconds * 1000.0;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
