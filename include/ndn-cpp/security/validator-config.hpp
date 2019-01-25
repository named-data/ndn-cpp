/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/validator-config.hpp
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

#ifndef NDN_VALIDATOR_CONFIG_HPP
#define NDN_VALIDATOR_CONFIG_HPP

#include "v2/validation-policy-config.hpp"
#include "v2/certificate-fetcher-from-network.hpp"
#include "v2/validator.hpp"

namespace ndn {

/**
 * ValidatorConfig extends Validator to implements a validator which can be
 * set up via a configuration file.
 */
class ValidatorConfig : public Validator {
public:
  // TODO: Add Options.
  // TODO: Add ValidationPolicyCommandInterest.
  /**
   * Create a ValidatorConfig to use the given certificate fetcher.
   * @param fetcher The certificate fetcher.
   */
  ValidatorConfig(const ptr_lib::shared_ptr<CertificateFetcher>& fetcher)
  : Validator(ptr_lib::make_shared<ValidationPolicyConfig>(), fetcher),
    // TODO: Use getInnerPolicy().
    policyConfig_(dynamic_cast<ValidationPolicyConfig&>(getPolicy()))
  {
  }

  // TODO: Add Options.
  // TODO: Add ValidationPolicyCommandInterest.
  /**
   * Create a ValidatorConfig that uses a CertificateFetcherFromNetwork for the
   * given Face.
   * @param face The face for the certificate fetcher to call expressInterest.
   */
  ValidatorConfig(Face& face)
  : Validator
      (ptr_lib::make_shared<ValidationPolicyConfig>(),
       ptr_lib::make_shared<CertificateFetcherFromNetwork>(face)),
    // TODO: Use getInnerPolicy().
    policyConfig_(dynamic_cast<ValidationPolicyConfig&>(getPolicy()))
  {
  }

  /**
   * Load the configuration from the given config file. This replaces any
   * existing configuration.
   * @param filePath The The path of the config file.
   */
  void
  load(const std::string& filePath)
  {
    policyConfig_.load(filePath);
  }

  /**
   * Load the configuration from the given input string. This replaces any
   * existing configuration.
   * @param input The contents of the configuration rules, with lines separated
   * by "\n" or "\r\n".
   * @param inputName Used for log messages, etc.
   */
  void
  load(const std::string& input, const std::string& inputName)
  {
    policyConfig_.load(input, inputName);
  }

  /**
   * Load the configuration from the given configSection. This replaces any
   * existing configuration.
   * @param configSection The configuration section loaded from the config file.
   * It should have one <validator> section.
   * @param inputName Used for log messages, etc.
   */
  void
  load(const BoostInfoTree& configSection, const std::string& inputName)
  {
    policyConfig_.load(configSection, inputName);
  }

private:
  ValidationPolicyConfig& policyConfig_;
};

}

#endif
