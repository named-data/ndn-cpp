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

#ifndef NDN_INTEREST_FILTER_HPP
#define NDN_INTEREST_FILTER_HPP

#include "name.hpp"

namespace ndn {

/**
 * An InterestFilter holds a Name prefix and optional regex match expression for
 * use in Face::setInterestFilter.
 */
class InterestFilter {
public:
  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix.
   * @param prefix The prefix Name. This makes a copy of the Name.
   */
  InterestFilter(const Name& prefix)
  : prefix_(prefix)
  {
  }

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix.
   * @param prefixUri The URI of the prefix Name.
   */
  InterestFilter(const std::string& prefixUri)
  : prefix_(prefixUri)
  {
  }

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix.
   * @param prefixUri The URI of the prefix Name.
   */
  InterestFilter(const char* prefixUri)
  : prefix_(prefixUri)
  {
  }

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefix The prefix Name. This makes a copy of the Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const Name& prefix, const std::string& regexFilter);

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix URI and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefix The prefix Name. This makes a copy of the Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const Name& prefix, const char* regexFilter);

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix URI and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefixUri The URI of the prefix Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const std::string& prefixUri, const std::string& regexFilter);

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix URI and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefixUri The URI of the prefix Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const char* prefixUri, const std::string& regexFilter);

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix URI and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefixUri The URI of the prefix Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const std::string& prefixUri, const char* regexFilter);

  /**
   * Create an InterestFilter to match any Interest whose name starts with the
   * given prefix URI and the remaining components match the regexFilter regular
   * expression as described in doesMatch.
   * @param prefixUri The URI of the prefix Name.
   * @param regexFilter The regular expression for matching the remaining name
   * components.
   */
  InterestFilter(const char* prefixUri, const char* regexFilter);

  /**
   * Check if the given name matches this filter. Match if name starts with this
   * filter's prefix. If this filter has the optional regexFilter then the
   * remaining components match the regexFilter regular expression.
   * For example, the following InterestFilter:
   *
   *    InterestFilter("/hello", "<world><>+")
   *
   * will match all Interests, whose name has the prefix `/hello` which is
   * followed by a component `world` and has at least one more component after it.
   * Examples:
   *
   *    /hello/world/!
   *    /hello/world/x/y/z
   *
   * Note that the regular expression will need to match all remaining components
   * (e.g., there are implicit heading `^` and trailing `$` symbols in the
   * regular expression).
   * @param name The name to check against this filter.
   * @return True if name matches this filter, otherwise false.
   */
  bool
  doesMatch(const Name& name) const;

  /**
   * Get the prefix given to the constructor.
   * @return The prefix Name.
   */
  const Name&
  getPrefix() const { return prefix_; }

  /**
   * Check if a regexFilter was supplied to the constructor.
   * @return True if a regexFilter was supplied to the constructor.
   */
  bool
  hasRegexFilter() const { return regexFilter_.size() != 0; }

  /**
   * Get the regex filter. This is only valid if hasRegexFilter()is true.
   * @return The regular expression for matching the remaining name components.
   */
  const std::string&
  getRegexFilter() const { return regexFilter_; }

private:
  /**
   * If regexFilter doesn't already have them, add ^ to the beginning and $ to
   * the end since these are required by NdnRegexMatcher.match.
   * @param regexFilter The regex filter.
   * @return The regex pattern with ^ and $.
   */
  static std::string
  makePattern(const std::string& regexFilter);

  Name prefix_;
  std::string regexFilter_;
  std::string regexFilterPattern_;
};

}

#endif
