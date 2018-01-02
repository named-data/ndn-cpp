/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validator-config/filter.cpp
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

#include <ndn-cpp/security/validator-config-error.hpp>
#include <ndn-cpp/security/v2/validator-config/config-name-relation.hpp>
#include "../../../util/boost-info-parser.hpp"
#include "../../../util/regex/ndn-regex-top-matcher.hpp"
#include <ndn-cpp/security/v2/validator-config/config-filter.hpp>

using namespace std;

namespace ndn {

ConfigFilter::~ConfigFilter() {}

bool
ConfigFilter::match(bool isForInterest, const Name& packetName)
{
  if (isForInterest) {
    const int signedInterestMinSize = 2;
    
    if (packetName.size() < signedInterestMinSize)
      return false;

    return matchName(packetName.getPrefix(-signedInterestMinSize));
  }
  else
    // Data packet.
    return matchName(packetName);
}

ptr_lib::shared_ptr<ConfigFilter>
ConfigFilter::create(const BoostInfoTree& configSection)
{
  const string* filterType = configSection.getFirstValue("type");
  if (!filterType)
    throw ValidatorConfigError("Expected <filter.type>");

  if (equalsIgnoreCase(*filterType, "name"))
    return createNameFilter(configSection);
  else
    throw ValidatorConfigError("Unsupported filter.type: " + *filterType);
}

ptr_lib::shared_ptr<ConfigFilter>
ConfigFilter::createNameFilter(const BoostInfoTree& configSection)
{
  const string* nameUri = configSection.getFirstValue("name");
  if (nameUri) {
    // Get the filter.name.
    Name name(*nameUri);

    // Get the filter.relation.
    const string* relationValue = configSection.getFirstValue("relation");
    if (!relationValue)
      throw ValidatorConfigError("Expected <filter.relation>");

    ConfigNameRelation::Relation relation =
      ConfigNameRelation::getNameRelationFromString(*relationValue);

    return ptr_lib::make_shared<ConfigRelationNameFilter>(name, relation);
  }

  const string* regexString = configSection.getFirstValue("regex");
  if (regexString) {
    try {
      return ptr_lib::make_shared<ConfigRegexNameFilter>(*regexString);
    }
    catch (const std::exception& e) {
      throw ValidatorConfigError("Wrong filter.regex: " + *regexString);
    }
  }

  throw ValidatorConfigError("Wrong filter(name) properties");
}

bool
ConfigRelationNameFilter::matchName(const Name& packetName)
{
  return ConfigNameRelation::checkNameRelation(relation_, name_, packetName);
}

ConfigRegexNameFilter::ConfigRegexNameFilter(const string& regexString)
: regex_(new NdnRegexTopMatcher(regexString))
{
}

bool
ConfigRegexNameFilter::matchName(const Name& packetName)
{
  return regex_->match(packetName);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
