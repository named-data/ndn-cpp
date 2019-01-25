/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validator-config/checker.hpp
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

#ifndef NDN_CONFIG_CHECKER_HPP
#define NDN_CONFIG_CHECKER_HPP

#include "../../../name.hpp"
#include "config-name-relation.hpp"
#include "../validation-state.hpp"

namespace ndn {

class BoostInfoTree;
class NdnRegexTopMatcher;

/**
 * A ConfigChecker is an abstract base class for ConfigNameRelationChecker, etc.
 * used by ValidatorConfig to check if a packet name and KeyLocator satisfy the
 * conditions in a configuration section.
 */
class ConfigChecker {
public:
  virtual
  ~ConfigChecker();

  /**
   * Check if the packet name ane KeyLocator name satisfy this checker's
   * conditions.
   * @param isForInterest True if packetName is for an Interest, false if for a
   * Data packet.
   * @param packetName The packet name. For a signed interest, the last two
   * components are skipped but not removed.
   * @param keyLocatorName The KeyLocator's name.
   * @param state This calls state.fail() if the packet is invalid.
   * @return True if further signature verification is needed, or false if the
   * packet is immediately determined to be invalid in which case this calls
   * state.fail() with the proper code and message.
   */
  bool
  check
    (bool isForInterest, const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state);

  /**
   * Create a checker from the configuration section.
   * @param configSection The section containing the definition of the checker,
   * e.g. one of <validation.rule.checker>.
   * @return A new checker created from the configuration section.
   */
  static ptr_lib::shared_ptr<ConfigChecker>
  create(const BoostInfoTree& configSection);

protected:
  ConfigChecker() {}

  /**
   * Check if the packet name ane KeyLocator name satisfy this checker's
   * conditions.
   * @param packetName The packet name, which is already stripped of signature
   * components if this is a signed Interest name.
   * @param keyLocatorName The KeyLocator's name.
   * @param state This calls state.fail() if the packet is invalid.
   * @return True if further signature verification is needed, or false if the
   * packet is immediately determined to be invalid in which case this calls
   * state.fail() with the proper code and message.
   */
  virtual bool
  checkNames
    (const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state) = 0;

private:
  static ptr_lib::shared_ptr<ConfigChecker>
  createCustomizedChecker(const BoostInfoTree& configSection);

  static ptr_lib::shared_ptr<ConfigChecker>
  createHierarchicalChecker(const BoostInfoTree& configSection);

  static ptr_lib::shared_ptr<ConfigChecker>
  createKeyLocatorChecker(const BoostInfoTree& configSection);

  static ptr_lib::shared_ptr<ConfigChecker>
  createKeyLocatorNameChecker(const BoostInfoTree& configSection);

  // Disable the copy constructor and assignment operator.
  ConfigChecker(const ConfigChecker& other);
  ConfigChecker& operator=(const ConfigChecker& other);
};

class ConfigNameRelationChecker : public ConfigChecker {
public:
  ConfigNameRelationChecker
    (const Name& name, ConfigNameRelation::Relation relation)
  : name_(name),
    relation_(relation)
  {
  }

protected:
  virtual bool
  checkNames
    (const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state);

private:
  Name name_;
  ConfigNameRelation::Relation relation_;
};

class ConfigRegexChecker : public ConfigChecker {
public:
  ConfigRegexChecker(const std::string& regexString);

protected:
  virtual bool
  checkNames
    (const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state);

private:
  ptr_lib::shared_ptr<NdnRegexTopMatcher> regex_;
};

class ConfigHyperRelationChecker : public ConfigChecker {
public:
  ConfigHyperRelationChecker
    (const std::string& packetNameRegexString, const std::string& packetNameExpansion,
     const std::string& keyNameRegexString, const std::string& keyNameExpansion,
     ConfigNameRelation::Relation hyperRelation);

protected:
  virtual bool
  checkNames
    (const Name& packetName, const Name& keyLocatorName,
     const ptr_lib::shared_ptr<ValidationState>& state);

private:
  ptr_lib::shared_ptr<NdnRegexTopMatcher> packetNameRegex_;
  std::string packetNameExpansion_;
  ptr_lib::shared_ptr<NdnRegexTopMatcher> keyNameRegex_;
  std::string keyNameExpansion_;
  ConfigNameRelation::Relation hyperRelation_;
};

}

#endif
