/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validator-config/rule.hpp
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

#ifndef NDN_CONFIG_RULE_HPP
#define NDN_CONFIG_RULE_HPP

#include "config-filter.hpp"
#include "config-checker.hpp"

namespace ndn {

class BoostInfoTree;

/**
 * A ConfigRule represents a rule configuration section, used by ConfigValidator.
 */
class ConfigRule {
public:
  /**
   * Create a ConfigRule with empty filters and checkers.
   * @param id The rule ID from the configuration section.
   * @param isForInterest True if the rule is for an Interest packet, false if
   * it is for a Data packet.
   */
  ConfigRule(const std::string& id, bool isForInterest)
  : id_(id),
    isForInterest_(isForInterest)
  {
  }

  /**
   * Get the rule ID.
   * @return The rule ID.
   */
  const std::string&
  getId() const { return id_; }

  /**
   * Get the isForInterest flag.
   * @return True if the rule is for an Interest packet, false if it is for a
   * Data packet.
   */
  bool
  getIsForInterest() const { return isForInterest_; }

  /**
   * Add the ConfigFilter to the list of filters.
   * @param filter The ConfigFilter.
   */
  void
  addFilter(const ptr_lib::shared_ptr<ConfigFilter>& filter)
  {
    filters_.push_back(filter);
  }

  /**
   * Add the ConfigChecker to the list of checkers.
   * @param checker The ConfigChecker.
   */
  void
  addChecker(const ptr_lib::shared_ptr<ConfigChecker>& checker)
  {
    checkers_.push_back(checker);
  }

  /**
   * Check if the packet name matches the rule's filter.
   * If no filters were added, the rule matches everything.
   * @param isForInterest True if packetName is for an Interest, false if for a
   * Data packet.
   * @param packetName The packet name. For a signed interest, the last two
   * components are skipped but not removed.
   * @retur True if at least one filter matches the packet name, false if none
   * of the filters match the packet name.
   * @throws ValidatorConfigError if the supplied isForInterest doesn't match the
   * one for which the rule is designed.
   */
  bool
  match(bool isForInterest, const Name& packetName);

  /**
   * Check if the packet satisfies the rule's condition.
   * @param isForInterest True if packetName is for an Interest, false if for a
   * Data packet.
   * @param packetName The packet name. For a signed interest, the last two
   * components are skipped but not removed.
   * @param keyLocatorName The KeyLocator's name.
   * @param state This calls state.fail() if the packet is invalid.
   * @return True if further signature verification is needed, or false if the
   * packet is immediately determined to be invalid in which case this calls
   * state.fail() with the proper code and message.
   * @throws ValidatorConfigError if the supplied isForInterest doesn't match the
   * one for which the rule is designed.
   */
  bool
  check
    (bool isForInterest, const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state);

  /**
   * Create a rule from configuration section.
   * @param configSection The section containing the definition of the rule,
   * e.g. one of <validator.rule>.
   * @return A new ConfigRule created from the configuration
   */
  static ptr_lib::shared_ptr<ConfigRule>
  create(const BoostInfoTree& configSection);

private:
  // Disable the copy constructor and assignment operator.
  ConfigRule(const ConfigRule& other);
  ConfigRule& operator=(const ConfigRule& other);

  std::string id_;
  bool isForInterest_;
  std::vector<ptr_lib::shared_ptr<ConfigFilter>> filters_;
  std::vector<ptr_lib::shared_ptr<ConfigChecker>> checkers_;
};

}

#endif
