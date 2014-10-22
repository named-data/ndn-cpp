/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
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

#include "ndn-regex-matcher.hpp"

// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

using namespace std;

namespace ndn {

using namespace regex_lib;

sregex_iterator
NdnRegexMatcher::match(const string& patternIn, const Name& name)
{
  string pattern = patternIn;
  
  // nameParts = [name.get(i).getValue().toRawStr() for i in range(name.size())]
  // nameUri = '/'+'/'.join(nameParts)
  string nameUri = name.toUri();

  pattern = sanitizeSets(pattern);

  pattern = regex_replace(pattern, regex("<>"), "(?:<.+?>)");
  // Use regex_replace since std::string does have "replace all".
  pattern = regex_replace(pattern, regex(">"), "");
  pattern = regex_replace(pattern, regex("<(?!!)"), "/");

  return sregex_iterator(nameUri.begin(), nameUri.end(), regex(pattern));
}

string
NdnRegexMatcher::sanitizeSets(const string& pattern)
{
  string newPattern = pattern;

  // Positive sets can be changed to (comp1|comp2).
  // Negative sets must be changed to negative lookahead assertions.

  regex regex1("\\[(\\^?)(.*?)\\]");
  const sregex_iterator iEnd;
  for (sregex_iterator i(pattern.begin(), pattern.end(), regex1); i != iEnd; ++i) {
    smatch match = *i;

    // insert | between components
    int start = match.position(2);
    int end = start + match.length(2);
    if (end - start == 0)
        continue;
    string oldStr = match.str(2);
    string newStr = regex_replace(oldStr, regex("><"), ">|<");
    string tempStr = newPattern.substr(0, start) + newStr + newPattern.substr(end);
    newPattern = tempStr;
  }

  // Replace [] with (),  or (?! ) for negative lookahead.
  // If we use negative lookahead, we also have to consume one component.
  if (newPattern.find("[^") != string::npos) {
    // Use regex_replace since std::string does have "replace all".
    newPattern = regex_replace(newPattern, regex("\\[\\^"), "(?:(?!");
    newPattern = regex_replace(newPattern, regex("\\]"), ")(?:/.*)*)");
  }
  else {
    newPattern = regex_replace(newPattern, regex("\\["), "(");
    newPattern = regex_replace(newPattern, regex("\\]"), ")");
  }

  return newPattern;
}

string
NdnRegexMatcher::escapeComponents(const string& pattern)
{
  // Escape all the components in the pattern to match the toUri format.
  string componentPattern = "<(.*?)>(.*?)(?=<|$)";

  string modifiedStr = pattern;

  regex regex1(componentPattern);
  const sregex_iterator iEnd;
  for (sregex_iterator i(pattern.begin(), pattern.end(), regex1); i != iEnd; ++i) {
    smatch match = *i;

    int start = match.position(1);
    int end = start + match.length(1);
    if (end - start == 0)
        continue;
    string oldStr = match.str(2);
    string newStr = Name::Component(oldStr).toEscapedString();
    modifiedStr = modifiedStr.substr(0, start) + newStr + modifiedStr.substr(end);
  }
  
  return modifiedStr;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
