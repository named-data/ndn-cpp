/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validation-policy-config.hpp
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

#ifndef NDN_VALIDATION_POLICY_CONFIG_HPP
#define NDN_VALIDATION_POLICY_CONFIG_HPP

#include "validation-policy.hpp"
#include "validator-config/config-rule.hpp"

namespace ndn {

class BoostInfoTree;

/**
 * ValidationPolicyConfig implements a validator which can be set up via a
 * configuration file. For command Interest validation, this policy must be
 * combined with ValidationPolicyCommandInterest in order to guard against
 * replay attacks.
 * @note This policy does not support inner policies (a sole policy or a
 * terminal inner policy).
 * @see https://named-data.net/doc/ndn-cxx/current/tutorials/security-validator-config.html
 */
class ValidationPolicyConfig : public ValidationPolicy {
public:
  /**
   * Create a default ValidationPolicyConfig.
   */
  ValidationPolicyConfig()
  : shouldBypass_(false),
    isConfigured_(false)
  {
  }

  /**
   * Load the configuration from the given config file. This replaces any
   * existing configuration.
   * @param filePath The The path of the config file.
   */
  void
  load(const std::string& filePath);

  /**
   * Load the configuration from the given input string. This replaces any
   * existing configuration.
   * @param input The contents of the configuration rules, with lines separated
   * by "\n" or "\r\n".
   * @param inputName Used for log messages, etc.
   */
  void
  load(const std::string& input, const std::string& inputName);

  /**
   * Load the configuration from the given configSection. This replaces any
   * existing configuration.
   * @param configSection The configuration section loaded from the config file.
   * It should have one <validator> section.
   * @param inputName Used for log messages, etc.
   */
  void
  load(const BoostInfoTree& configSection, const std::string& inputName);

  virtual void
  checkPolicy
    (const Data& data, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

  virtual void
  checkPolicy
    (const Interest& interest, const ptr_lib::shared_ptr<ValidationState>& state,
     const ValidationContinuation& continueValidation);

private:
  /**
   * Process the trust-anchor configuration section and call
   * validator_.loadAnchor as needed.
   * @param configSection The section containing the definition of the trust
   * anchor, e.g. one of <validator.trust-anchor>.
   * @param inputName Used for log messages, etc.
   */
  void
  processConfigTrustAnchor
    (const BoostInfoTree& configSection, const std::string& inputName);

  /**
   * Get the "refresh" value. If the value is 9, return a period of one hour.
   * @param configSection The section containing the definition of the trust
   * anchor, e.g. one of <validator.trust-anchor>.
   * @return The refresh period in milliseconds. However if there is no
   * "refresh" value, return a large number (effectively no refresh).
   */
  static Milliseconds
  getRefreshPeriod(const BoostInfoTree& configSection);

  /** shouldBypass_ is set to true when 'any' is specified as a trust anchor,
   * causing all packets to bypass validation.
   */
  bool shouldBypass_;
  bool isConfigured_;
  std::vector<ptr_lib::shared_ptr<ConfigRule>> dataRules_;
  std::vector<ptr_lib::shared_ptr<ConfigRule>> interestRules_;
};

}

#endif
