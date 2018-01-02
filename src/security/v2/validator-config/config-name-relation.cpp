/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/v2/validator-config/name-relation.hpp
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

using namespace std;

namespace ndn {

string
ConfigNameRelation::toString(Relation relation)
{
  if (relation == Relation::EQUAL)
    return "equal";
  else if (relation == Relation::IS_PREFIX_OF)
    return "is-prefix-of";
  else if (relation == Relation::IS_STRICT_PREFIX_OF)
    return "is-strict-prefix-of";
  else
    // We don't expect this to happen.
    return "";
}

bool
ConfigNameRelation::checkNameRelation
  (Relation relation, const Name& name1, const Name& name2)
{
  if (relation == Relation::EQUAL)
    return name1.equals(name2);
  else if (relation == Relation::IS_PREFIX_OF)
    return name1.isPrefixOf(name2);
  else if (relation == Relation::IS_STRICT_PREFIX_OF)
    return name1.isPrefixOf(name2) && name1.size() < name2.size();
  else
    // We don't expect this to happen.
    return false;
}

ConfigNameRelation::Relation
ConfigNameRelation::getNameRelationFromString(const string& relationString)
{
  if (equalsIgnoreCase(relationString, "equal"))
    return Relation::EQUAL;
  else if (equalsIgnoreCase(relationString, "is-prefix-of"))
    return Relation::IS_PREFIX_OF;
  else if (equalsIgnoreCase(relationString, "is-strict-prefix-of"))
    return Relation::IS_STRICT_PREFIX_OF;
  else
    throw ValidatorConfigError("Unsupported relation: " + relationString);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
