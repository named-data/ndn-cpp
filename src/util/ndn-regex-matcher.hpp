/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN ndn_regex.py by Adeola Bannis.
 * Originally from Yingdi Yu <http://irl.cs.ucla.edu/~yingdi/>.
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

#ifndef NDN_NDN_REGEX_MATCHER_HPP
#define NDN_NDN_REGEX_MATCHER_HPP

// Depending if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_REGEX or sets
//   NDN_CPP_HAVE_STD_REGEX = 1, define the regex_lib namespace alias.
// Set NDN_CPP_HAVE_REGEX_LIB = 1 if regex_lib is defined.

#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_STD_REGEX
  #include <regex>
  namespace ndn { namespace regex_lib = std; }
  #define NDN_CPP_HAVE_REGEX_LIB 1
#elif defined(NDN_CPP_HAVE_BOOST_REGEX)
  #include <boost/regex.hpp>
  namespace ndn { namespace regex_lib = boost; }
  #define NDN_CPP_HAVE_REGEX_LIB 1
#else
  #define NDN_CPP_HAVE_REGEX_LIB 0
#endif

// Only compile if we set NDN_CPP_HAVE_REGEX_LIB above.
#if NDN_CPP_HAVE_REGEX_LIB

#include <string>
#include <ndn-cpp/name.hpp>

namespace ndn {

/**
 * Convert an NDN regex (http://redmine.named-data.net/projects/ndn-cxx/wiki/Regex)
 * to a std::regex that can match against URIs.
 * Note that in PyNDN, etc. this has a static member called match, but here
 * we construct an NdnRegexMatcher object so that it can carry the internal
 * source string used for the match. This is necessary because the
 * sregex_iterator uses pointers and we have to keep the string they point into.
 */
class NdnRegexMatcher {
public:
  /**
   * Create a new NdnRegexMatcher to determine if the provided NDN regex pattern
   * matches the given Name.  The member iterator has the sregex_iterator for
   * the pattern. It is sregex_iterator() if pattern does not match.
   * @param pattern The NDN regex.
   * @param name The Name to match against the regex.
   */
  NdnRegexMatcher(const std::string& pattern, const Name& name);

  regex_lib::sregex_iterator iterator;

private:
  static std::string
  sanitizeSets(const std::string& pattern);

  // This keeps the source string that for the pointers in iterator.
  ptr_lib::shared_ptr<std::string> source_;
};

}

#endif // NDN_CPP_HAVE_REGEX_LIB

#endif
