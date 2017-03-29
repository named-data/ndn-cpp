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

#include "ndn-regex-backref-manager.hpp"
#include "ndn-regex-backref-matcher.hpp"
#include "ndn-regex-repeat-matcher.hpp"
#include "ndn-regex-pattern-list-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexPatternListMatcher::~NdnRegexPatternListMatcher() {}

void
NdnRegexPatternListMatcher::compile()
{
  size_t len = expr_.size();
  size_t index = 0;
  size_t subHead = index;

  while (index < len) {
    subHead = index;

    if (!extractPattern(subHead, &index))
      throw NdnRegexMatcherBase::Error("Compile error");
  }
}

bool
NdnRegexPatternListMatcher::extractPattern(size_t index, size_t* next)
{
  size_t start = index;
  size_t end = index;
  size_t indicator = index;

  switch (expr_[index]) {
  case '(':
    ++index;
    index = extractSubPattern('(', ')', index);
    indicator = index;
    end = extractRepetition(index);
    if (indicator == end) {
      ptr_lib::shared_ptr<NdnRegexMatcherBase> matcher =
        ptr_lib::make_shared<NdnRegexBackrefMatcher>
          (expr_.substr(start, end - start), backrefManager_);
      backrefManager_->pushRef(matcher);
      ptr_lib::dynamic_pointer_cast<NdnRegexBackrefMatcher>(matcher)
        ->lateCompile();

      matchers_.push_back(matcher);
    }
    else
      matchers_.push_back(ptr_lib::make_shared<NdnRegexRepeatMatcher>
        (expr_.substr(start, end - start), backrefManager_, indicator - start));
    break;

  case '<':
    ++index;
    index = extractSubPattern('<', '>', index);
    indicator = index;
    end = extractRepetition(index);
    matchers_.push_back(ptr_lib::make_shared<NdnRegexRepeatMatcher>
      (expr_.substr(start, end - start), backrefManager_, indicator - start));
    break;

  case '[':
    ++index;
    index = extractSubPattern('[', ']', index);
    indicator = index;
    end = extractRepetition(index);
    matchers_.push_back(ptr_lib::make_shared<NdnRegexRepeatMatcher>
      (expr_.substr(start, end - start), backrefManager_, indicator - start));
    break;

  default:
    throw NdnRegexMatcherBase::Error("Unexpected syntax");
  }

  *next = end;

  return true;
}

size_t
NdnRegexPatternListMatcher::extractSubPattern
  (const char left, const char right, size_t index)
{
  size_t lcount = 1;
  size_t rcount = 0;

  while (lcount > rcount) {
    if (index >= expr_.size())
      throw NdnRegexMatcherBase::Error("Parenthesis mismatch");

    if (left == expr_[index])
      lcount++;

    if (right == expr_[index])
      rcount++;

    ++index;
  }
  
  return index;
}

size_t
NdnRegexPatternListMatcher::extractRepetition(size_t index)
{
  size_t exprSize = expr_.size();

  if (index == exprSize)
    return index;

  if (('+' == expr_[index] || '?' == expr_[index] || '*' == expr_[index]))
    return ++index;

  if ('{' == expr_[index]) {
    while ('}' != expr_[index]) {
      index++;
      if (index == exprSize)
        break;
    }
    if (index == exprSize)
      throw NdnRegexMatcherBase::Error("Missing right brace bracket");
    else
      return ++index;
  }
  else
    return index;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
