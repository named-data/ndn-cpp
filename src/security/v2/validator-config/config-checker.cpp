/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validator-config/checker.cpp
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

#include "../../../util/regex/ndn-regex-matcher-base.hpp"
// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher-base.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

#include "../../../util/boost-info-parser.hpp"
#include "../../../util/regex/ndn-regex-top-matcher.hpp"
#include <ndn-cpp/security/pib/pib-key.hpp>
#include <ndn-cpp/security/validator-config-error.hpp>
#include <ndn-cpp/security/v2/validator-config/config-checker.hpp>

using namespace std;

namespace ndn {

ConfigChecker::~ConfigChecker() {}

bool
ConfigChecker::check
  (bool isForInterest, const Name& packetName, const Name& keyLocatorName,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  if (isForInterest) {
    const int signedInterestMinSize = 2;

    if (packetName.size() < signedInterestMinSize)
      return false;

    return checkNames
      (packetName.getPrefix(-signedInterestMinSize), keyLocatorName, state);
  }
  else
    return checkNames(packetName, keyLocatorName, state);
}

ptr_lib::shared_ptr<ConfigChecker>
ConfigChecker::create(const BoostInfoTree& configSection)
{
  // Get checker.type.
  const string* checkerType = configSection.getFirstValue("type");
  if (!checkerType)
    throw ValidatorConfigError("Expected <checker.type>");

  if (equalsIgnoreCase(*checkerType, "customized"))
    return createCustomizedChecker(configSection);
  else if (equalsIgnoreCase(*checkerType, "hierarchical"))
    return createHierarchicalChecker(configSection);
  else
    throw ValidatorConfigError("Unsupported checker type: " + *checkerType);
}

ptr_lib::shared_ptr<ConfigChecker>
ConfigChecker::createCustomizedChecker(const BoostInfoTree& configSection)
{
  // Ignore sig-type.
  // Get checker.key-locator .
  vector<const BoostInfoTree*> keyLocatorSection = configSection["key-locator"];
  if (keyLocatorSection.size() != 1)
    throw ValidatorConfigError("Expected one <checker.key-locator>");

  return createKeyLocatorChecker(*keyLocatorSection[0]);
}

ptr_lib::shared_ptr<ConfigChecker>
ConfigChecker::createHierarchicalChecker(const BoostInfoTree& configSection)
{
  // Ignore sig-type.
  return ptr_lib::make_shared<ConfigHyperRelationChecker>
    ("^(<>*)$",        "\\1",
     "^(<>*)<KEY><>$", "\\1",
     ConfigNameRelation::Relation::IS_PREFIX_OF);
}

ptr_lib::shared_ptr<ConfigChecker>
ConfigChecker::createKeyLocatorChecker(const BoostInfoTree& configSection)
{
  // Get checker.key-locator.type .
  const string* keyLocatorType = configSection.getFirstValue("type");
  if (!keyLocatorType)
    throw ValidatorConfigError("Expected <checker.key-locator.type>");

  if (equalsIgnoreCase(*keyLocatorType, "name"))
    return createKeyLocatorNameChecker(configSection);
  else
    throw ValidatorConfigError
      ("Unsupported checker.key-locator.type: " + *keyLocatorType);
}

ptr_lib::shared_ptr<ConfigChecker>
ConfigChecker::createKeyLocatorNameChecker(const BoostInfoTree& configSection)
{
  const string* nameUri = configSection.getFirstValue("name");
  if (nameUri) {
    Name name(*nameUri);

    const string* relationValue = configSection.getFirstValue("relation");
    if (!relationValue)
      throw ValidatorConfigError("Expected <checker.key-locator.relation>");

    ConfigNameRelation::Relation relation =
      ConfigNameRelation::getNameRelationFromString(*relationValue);
    return ptr_lib::make_shared<ConfigNameRelationChecker>(name, relation);
  }

  const string* regexString = configSection.getFirstValue("regex");
  if (regexString) {
    try {
      return ptr_lib::make_shared<ConfigRegexChecker>(*regexString);
    }
    catch (const std::exception& e) {
      throw ValidatorConfigError
        ("Invalid checker.key-locator.regex: " + *regexString);
    }
  }

  vector<const BoostInfoTree*> hyperRelationList = configSection["hyper-relation"];
  if (hyperRelationList.size() == 1) {
    const BoostInfoTree& hyperRelation = *hyperRelationList[0];

    // Get k-regex.
    const string* keyRegex = hyperRelation.getFirstValue("k-regex");
    if (!keyRegex)
      throw ValidatorConfigError
        ("Expected <checker.key-locator.hyper-relation.k-regex>");

    // Get k-expand.
    const string* keyExpansion = hyperRelation.getFirstValue("k-expand");
    if (!keyExpansion)
      throw ValidatorConfigError
        ("Expected <checker.key-locator.hyper-relation.k-expand");

    // Get h-relation.
    const string* hyperRelationString = hyperRelation.getFirstValue("h-relation");
    if (!hyperRelationString)
      throw ValidatorConfigError
        ("Expected <checker.key-locator.hyper-relation.h-relation>");

    // Get p-regex.
    const string* packetNameRegex = hyperRelation.getFirstValue("p-regex");
    if (!packetNameRegex)
      throw ValidatorConfigError
        ("Expected <checker.key-locator.hyper-relation.p-regex>");

    // Get p-expand.
    const string* packetNameExpansion = hyperRelation.getFirstValue("p-expand");
    if (!packetNameExpansion)
      throw ValidatorConfigError
        ("Expected <checker.key-locator.hyper-relation.p-expand>");

    ConfigNameRelation::Relation relation =
      ConfigNameRelation::getNameRelationFromString(*hyperRelationString);

    try {
      return ptr_lib::make_shared<ConfigHyperRelationChecker>
        (*packetNameRegex, *packetNameExpansion, *keyRegex, *keyExpansion, relation);
    }
    catch (const std::exception& e) {
      throw ValidatorConfigError
        ("Invalid regex for key-locator.hyper-relation");
    }
  }

  throw ValidatorConfigError("Unsupported checker.key-locator");
}

bool
ConfigNameRelationChecker::checkNames
  (const Name& packetName, const Name& keyLocatorName,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  // packetName is not used in this check.

  Name identity = PibKey::extractIdentityFromKeyName(keyLocatorName);
  bool result = ConfigNameRelation::checkNameRelation(relation_, name_, identity);
  if (!result)
    state->fail(ValidationError(ValidationError::POLICY_ERROR,
      "KeyLocator check failed: name relation " + name_.toUri() + " " +
      ConfigNameRelation::toString(relation_) + " for packet " +
      packetName.toUri() + " is invalid (KeyLocator=" +
      keyLocatorName.toUri() + ", identity=" + identity.toUri() + ")"));

  return result;
}

ConfigRegexChecker::ConfigRegexChecker(const string& regexString)
: regex_(new NdnRegexTopMatcher(regexString))
{
}

bool
ConfigRegexChecker::checkNames
  (const Name& packetName, const Name& keyLocatorName,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  bool result = regex_->match(keyLocatorName);
  if (!result)
    state->fail(ValidationError(ValidationError::POLICY_ERROR,
      "KeyLocator check failed: regex " + regex_->getExpr() + " for packet " +
      packetName.toUri() + " is invalid (KeyLocator=" + keyLocatorName.toUri() +
      ")"));

  return result;
}

ConfigHyperRelationChecker::ConfigHyperRelationChecker
  (const string& packetNameRegexString, const string& packetNameExpansion,
   const string& keyNameRegexString, const string& keyNameExpansion,
   ConfigNameRelation::Relation hyperRelation)
: packetNameRegex_(new NdnRegexTopMatcher(packetNameRegexString)),
  packetNameExpansion_(packetNameExpansion),
  keyNameRegex_(new NdnRegexTopMatcher(keyNameRegexString)),
  keyNameExpansion_(keyNameExpansion),
  hyperRelation_(hyperRelation)
{
}

bool
ConfigHyperRelationChecker::checkNames
  (const Name& packetName, const Name& keyLocatorName,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  if (!packetNameRegex_->match(packetName)) {
    state->fail(ValidationError(ValidationError::POLICY_ERROR,
      "The packet " + packetName.toUri() + " (KeyLocator=" +
      keyLocatorName.toUri() + 
      ") does not match the hyper relation packet name regex " +
      packetNameRegex_->getExpr()));
    return false;
  }
  if (!keyNameRegex_->match(keyLocatorName)) {
    state->fail(ValidationError(ValidationError::POLICY_ERROR,
      "The packet " + packetName.toUri() + " (KeyLocator=" +
      keyLocatorName.toUri() +
      ") does not match the hyper relation key name regex " +
      keyNameRegex_->getExpr()));
    return false;
  }

  Name keyNameMatchExpansion = keyNameRegex_->expand(keyNameExpansion_);
  Name packetNameMatchExpansion = packetNameRegex_->expand(packetNameExpansion_);
  bool result = ConfigNameRelation::checkNameRelation
    (hyperRelation_, keyNameMatchExpansion, packetNameMatchExpansion);
  if (!result)
    state->fail(ValidationError(ValidationError::POLICY_ERROR,
      "KeyLocator check failed: hyper relation " +
      ConfigNameRelation::toString(hyperRelation_) + " packet name match=" +
      packetNameMatchExpansion.toUri() + ", key name match=" +
      keyNameMatchExpansion.toUri() + " of packet " + packetName.toUri() +
      " (KeyLocator=" + keyLocatorName.toUri() + ") is invalid"));

  return result;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
