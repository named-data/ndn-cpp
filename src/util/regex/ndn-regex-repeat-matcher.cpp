/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
 * @author: Yingdi Yu <http://irl.cs.ucla.edu/~yingdi/>
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

#include "ndn-regex-matcher-base.hpp"
// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher-base.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

#include <stdlib.h>
#include "ndn-regex-backref-manager.hpp"
#include "ndn-regex-backref-matcher.hpp"
#include "ndn-regex-component-set-matcher.hpp"
#include "ndn-regex-repeat-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexRepeatMatcher::~NdnRegexRepeatMatcher() {}

void
NdnRegexRepeatMatcher::compile()
{
  ptr_lib::shared_ptr<NdnRegexMatcherBase> matcher;

  if ('(' == expr_[0]) {
    matcher = ptr_lib::make_shared<NdnRegexBackrefMatcher>
      (expr_.substr(0, indicator_), backrefManager_);
    backrefManager_->pushRef(matcher);
    ptr_lib::dynamic_pointer_cast<NdnRegexBackrefMatcher>(matcher)
      ->lateCompile();
  }
  else
    matcher = ptr_lib::make_shared<NdnRegexComponentSetMatcher>
      (expr_.substr(0, indicator_), backrefManager_);

  matchers_.push_back(matcher);

  parseRepetition();
}

bool
NdnRegexRepeatMatcher::parseRepetition()
{
  size_t exprSize = expr_.size();
  const size_t MAX_REPETITIONS = 32767;

  if (exprSize == indicator_) {
    repeatMin_ = 1;
    repeatMax_ = 1;

    return true;
  }
  else {
    if (exprSize == (indicator_ + 1)) {
      if ('?' == expr_[indicator_]) {
        repeatMin_ = 0;
        repeatMax_ = 1;
        return true;
      }
      if ('+' == expr_[indicator_]) {
        repeatMin_ = 1;
        repeatMax_ = MAX_REPETITIONS;
        return true;
      }
      if ('*' == expr_[indicator_]) {
        repeatMin_ = 0;
        repeatMax_ = MAX_REPETITIONS;
        return true;
      }
    }
    else {
      string repeatStruct = expr_.substr(indicator_, exprSize - indicator_);
      size_t rsSize = repeatStruct.size();
      size_t min = 0;
      size_t max = 0;

      if (regex_lib::regex_match
          (repeatStruct, regex_lib::regex("\\{[0-9]+,[0-9]+\\}"))) {
        size_t separator = repeatStruct.find_first_of(',', 0);
        min = ::atoi(repeatStruct.substr(1, separator - 1).c_str());
        max = ::atoi
          (repeatStruct.substr(separator + 1, rsSize - separator - 2).c_str());
      }
      else if (regex_lib::regex_match
               (repeatStruct, regex_lib::regex("\\{,[0-9]+\\}"))) {
        size_t separator = repeatStruct.find_first_of(',', 0);
        min = 0;
        max = ::atoi
          (repeatStruct.substr(separator + 1, rsSize - separator - 2).c_str());
      }
      else if (regex_lib::regex_match
               (repeatStruct, regex_lib::regex("\\{[0-9]+,\\}"))) {
        size_t separator = repeatStruct.find_first_of(',', 0);
        min = ::atoi(repeatStruct.substr(1, separator - 1).c_str());
        max = MAX_REPETITIONS;
      }
      else if (regex_lib::regex_match
               (repeatStruct, regex_lib::regex("\\{[0-9]+\\}"))) {
        min = ::atoi(repeatStruct.substr(1, rsSize - 2).c_str());
        max = min;
      }
      else
        throw NdnRegexMatcherBase::Error
          (string("Error: RegexRepeatMatcher.ParseRepetition():") +
           " Unrecognized format " + expr_);

      if (min > MAX_REPETITIONS || max > MAX_REPETITIONS || min > max)
        throw NdnRegexMatcherBase::Error
          (string("Error: RegexRepeatMatcher.ParseRepetition():") +
           " Wrong number " + expr_);

      repeatMin_ = min;
      repeatMax_ = max;

      return true;
    }
  }

  return false;
}

bool
NdnRegexRepeatMatcher::match(const Name& name, size_t offset, size_t len)
{
  matchResult_.clear();

  if (0 == repeatMin_)
    if (0 == len)
      return true;

  if (recursiveMatch(0, name, offset, len)) {
    for (size_t i = offset; i < offset + len; ++i)
      matchResult_.push_back(name.get(i));
    return true;
  }
  else
    return false;
}

bool
NdnRegexRepeatMatcher::recursiveMatch
  (size_t repeat, const Name& name, size_t offset, size_t len)
{
  int tried = len;
  ptr_lib::shared_ptr<NdnRegexMatcherBase> matcher = matchers_[0];

  if (0 < len && repeat >= repeatMax_)
    return false;

  if (0 == len && repeat < repeatMin_)
    return false;

  if (0 == len && repeat >= repeatMin_)
    return true;

  while (tried >= 0) {
    if (matcher->match(name, offset, tried) &&
        recursiveMatch(repeat + 1, name, offset + tried, len - tried))
      return true;
    --tried;
  }

  return false;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
