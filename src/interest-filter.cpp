/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#include <stdexcept>
#include "util/ndn-regex-matcher.hpp"
#include <ndn-cpp/interest-filter.hpp>

using namespace std;

namespace ndn {

// Only compile these constructors if we set NDN_CPP_HAVE_REGEX_LIB in
// ndn-regex-matcher.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

InterestFilter::InterestFilter(const Name& prefix, const string& regexFilter)
: prefix_(prefix), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

InterestFilter::InterestFilter(const Name& prefix, const char* regexFilter)
: prefix_(prefix), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

InterestFilter::InterestFilter(const string& prefixUri, const string& regexFilter)
: prefix_(prefixUri), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

InterestFilter::InterestFilter(const char* prefixUri, const string& regexFilter)
: prefix_(prefixUri), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

InterestFilter::InterestFilter(const string& prefixUri, const char* regexFilter)
: prefix_(prefixUri), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

InterestFilter::InterestFilter(const char* prefixUri, const char* regexFilter)
: prefix_(prefixUri), regexFilter_(regexFilter),
  regexFilterPattern_(makePattern(regexFilter))
{
}

#endif

bool
InterestFilter::doesMatch(const Name& name) const
{
  if (name.size() < prefix_.size())
    return false;

  if (hasRegexFilter()) {
#if NDN_CPP_HAVE_REGEX_LIB
    // Perform a prefix match and regular expression match for the remaining
    // components.
    if (!prefix_.match(name))
      return false;

    return regex_lib::sregex_iterator() != NdnRegexMatcher
      (regexFilterPattern_, name.getSubName(prefix_.size())).iterator;
#else
    // We should not reach this point because the constructors for regexFilter
    // don't compile.
    throw runtime_error("InterestFilter::regexFilter is not supported");
#endif
  }
  else
    // Just perform a prefix match.
    return prefix_.match(name);
}

string
InterestFilter::makePattern(const string& regexFilter)
{
  if (regexFilter.size() == 0)
    // We don't expect this.
    return "^$";

  string pattern;
  if (pattern[0] != '^')
    pattern = '^' + regexFilter;
  else
    pattern = regexFilter;

  if (pattern[pattern.size() - 1] != '$')
    pattern.push_back('$');

  return pattern;
}

}
