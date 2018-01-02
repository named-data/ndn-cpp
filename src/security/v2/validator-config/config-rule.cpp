/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validator-config/rule.cpp
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
#include <ndn-cpp/util/logging.hpp>
#include <ndn-cpp/security/validator-config-error.hpp>
#include <ndn-cpp/security/v2/validator-config/config-rule.hpp>

using namespace std;

INIT_LOGGER("ndn.ConfigRule");

namespace ndn {

bool
ConfigRule::match(bool isForInterest, const Name& packetName)
{
  _LOG_TRACE("Trying to match " << packetName.toUri());

  if (isForInterest != isForInterest_)
    throw ValidatorConfigError
      (string("Invalid packet type supplied ( ") +
       (isForInterest ? "interest" : "data") + " != " +
       (isForInterest_ ? "interest" : "data") + ")");

  if (filters_.size() == 0)
    return true;

  bool result = false;
  for (size_t i = 0; i < filters_.size(); ++i) {
    result = (result || filters_[i]->match(isForInterest, packetName));
    if (result)
      break;
  }

  return result;
}

bool
ConfigRule::check
  (bool isForInterest, const Name& packetName, const Name& keyLocatorName,
   const ptr_lib::shared_ptr<ValidationState>& state)
{
  _LOG_TRACE("Trying to check " << packetName.toUri() << " with keyLocator " <<
     keyLocatorName.toUri());

  if (isForInterest != isForInterest_)
    throw ValidatorConfigError
      (string("Invalid packet type supplied ( ") +
       (isForInterest ? "interest" : "data") + " != " +
       (isForInterest_ ? "interest" : "data") + ")");

  bool hasPendingResult = false;
  for (size_t i = 0; i < checkers_.size(); ++i) {
    bool result = checkers_[i]->check
      (isForInterest, packetName, keyLocatorName, state);
    if (!result)
      return result;
    hasPendingResult = true;
  }

  return hasPendingResult;
}

ptr_lib::shared_ptr<ConfigRule>
ConfigRule::create(const BoostInfoTree& configSection)
{
  // Get rule.id .
  const string* ruleId = configSection.getFirstValue("id");
  if (!ruleId)
    throw ValidatorConfigError("Expecting <rule.id>");

  // Get rule.for .
  const string* usage = configSection.getFirstValue("for");
  if (!usage)
    throw ValidatorConfigError("Expecting <rule.for> in rule: " + *ruleId);

  bool isForInterest;
  if (equalsIgnoreCase(*usage, "data"))
    isForInterest = false;
  else if (equalsIgnoreCase(*usage, "interest"))
    isForInterest = true;
  else
    throw ValidatorConfigError
      ("Unrecognized <rule.for>: " + *usage + " in rule: " + *ruleId);

  ptr_lib::shared_ptr<ConfigRule> rule =
    ptr_lib::make_shared<ConfigRule>(*ruleId, isForInterest);

  // Get rule.filter(s)
  vector<const BoostInfoTree*> filterList = configSection["filter"];
  for (size_t i = 0; i < filterList.size(); ++i)
    rule->addFilter(ConfigFilter::create(*filterList[i]));

  // Get rule.checker(s)
  vector<const BoostInfoTree*> checkerList = configSection["checker"];
  for (size_t i = 0; i < checkerList.size(); ++i)
    rule->addChecker(ConfigChecker::create(*checkerList[i]));

  // Check other stuff.
  if (checkerList.size() == 0)
    throw ValidatorConfigError
      ("No <rule.checker> is specified in rule: " + *ruleId);

  return rule;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
