/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validator-config/filter.hpp
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

#ifndef NDN_CONFIG_FILTER_HPP
#define NDN_CONFIG_FILTER_HPP

#include "../../../interest.hpp"
#include "../../../data.hpp"
#include "config-name-relation.hpp"

namespace ndn {

class BoostInfoTree;
class NdnRegexTopMatcher;

/**
 * ConfigFilter is an abstract base class for RegexNameFilter, etc. used by
 * ValidatorConfig. The ValidatorConfig class consists of a set of rules.
 * The Filter class is a part of a rule and is used to match a packet.
 * Matched packets will be checked against the checkers defined in the rule.
 */
class ConfigFilter {
public:
  virtual
  ~ConfigFilter();

  /**
   * Call the virtual matchName method based on the packet type.
   * @param isForInterest True if packetName is for an Interest, false if for a
   * Data packet.
   * @param packetName The packet name. For a signed interest, the last two
   * components are skipped but not removed.
   * @return True for a match.
   */
  bool
  match(bool isForInterest, const Name& packetName);

  /**
   * Create a filter from the configuration section.
   * @param configSection The section containing the definition of the filter,
   * e.g. one of <validator.rule.filter>.
   * @return A new filter created from the configuration section.
   */
  static ptr_lib::shared_ptr<ConfigFilter>
  create(const BoostInfoTree& configSection);

protected:
  ConfigFilter() {}

private:
  /**
   * This is a helper for create() to create a filter from the configuration
   * section which is type "name".
   * @param configSection The section containing the definition of the filter.
   * @return A new filter created from the configuration section.
   */
  static ptr_lib::shared_ptr<ConfigFilter>
  createNameFilter(const BoostInfoTree& configSection);

  /**
   * Implementation of the check for match.
   * @param packetName The packet name, which is already stripped of signature
   * components if this is a signed Interest name.
   * @return True for a match.
   */
  virtual bool
  matchName(const Name& packetName) = 0;

  // Disable the copy constructor and assignment operator.
  ConfigFilter(const ConfigFilter& other);
  ConfigFilter& operator=(const ConfigFilter& other);
};

/**
 * ConfigRelationNameFilter extends ConfigFilter to check that the name is in
 * the given relation to the packet name.
 * The configuration
 * "filter
 * {
 *   type name
 *   name /example
 *   relation is-prefix-of
 * }"
 * creates ConfigRelationNameFilter("/example",
 *   ConfigNameRelation::Relation::IS_PREFIX_OF) .
 */
class ConfigRelationNameFilter : public ConfigFilter {
public:
  /**
   * Create a ConfigRelationNameFilter for the given values.
   * @param name The relation name, which is copied.
   * @param relation The relation type as a ConfigNameRelation::Relation enum.
   */
  ConfigRelationNameFilter
    (const Name& name, ConfigNameRelation::Relation relation)
  : name_(name),
    relation_(relation)
  {
  }

private:
  /**
   * Implementation of the check for match.
   * @param packetName The packet name, which is already stripped of signature
   * components if this is a signed Interest name.
   * @return True for a match.
   */
  virtual bool
  matchName(const Name& packetName);

  Name name_;
  ConfigNameRelation::Relation relation_;
};

/**
 * ConfigRegexNameFilter extends ConfigFilter to check that the packet name
 * matches the specified regular expression.
 * The configuration
 * "filter
 * {
 *   type name
 *   regex ^[^<KEY>]*<KEY><>*<ksk-.*>$
 * }"
 * creates ConfigRegexNameFilter("^[^<KEY>]*<KEY><>*<ksk-.*>$") .
 */
class ConfigRegexNameFilter : public ConfigFilter {
public:
  /**
   * Create a ConfigRegexNameFilter from the regex string.
   * @param regexString The regex string.
   */
  ConfigRegexNameFilter(const std::string& regexString);

private:
  /**
   * Implementation of the check for match.
   * @param packetName The packet name, which is already stripped of signature
   * components if this is a signed Interest name.
   * @return True for a match.
   */
  virtual bool
  matchName(const Name& packetName);

private:
  ptr_lib::shared_ptr<NdnRegexTopMatcher> regex_;
};

}

#endif
