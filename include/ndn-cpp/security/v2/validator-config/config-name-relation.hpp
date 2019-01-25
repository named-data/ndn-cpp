/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2019 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/v2/validator-config/name-relation.hpp
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

#ifndef NDN_CONFIG_NAME_RELATION_HPP
#define NDN_CONFIG_NAME_RELATION_HPP

#include "../../../name.hpp"

namespace ndn {

/** ConfigNameRelation defines the ConfigNameRelation::Relation enum and static
 * methods to work with name relations for the ValidatorConfig.
 */
class ConfigNameRelation {
public:
  enum Relation {
    EQUAL,
    IS_PREFIX_OF,
    IS_STRICT_PREFIX_OF
  };

  /**
   * Get a string representation of the Relation enum.
   * @param relation The value for the ConfigNameRelation::Relation enum.
   * @return The string representation.
   */
  static std::string
  toString(Relation relation);

  /**
   * Check whether name1 and name2 satisfy the relation.
   * @param relation The value for the ConfigNameRelation::Relation enum.
   * @param name1 The first name to check.
   * @param name2 The second name to check.
   * @return True if the names satisfy the relation.
   */
  static bool
  checkNameRelation(Relation relation, const Name& name1, const Name& name2);

  /**
   * Convert relationString to a ConfigNameRelation::Relation enum.
   * @throws ValidatorConfigError if relationString cannot be converted.
   */
  static Relation
  getNameRelationFromString(const std::string& relationString);
};

}

#endif
