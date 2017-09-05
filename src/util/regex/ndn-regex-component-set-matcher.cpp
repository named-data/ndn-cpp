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

#include "ndn-regex-component-set-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexComponentSetMatcher::~NdnRegexComponentSetMatcher() {}

void
NdnRegexComponentSetMatcher::compile()
{
  if (expr_.size() < 2)
    throw NdnRegexMatcherBase::Error
      ("Regexp compile error (cannot parse " + expr_ + ")");

  switch (expr_[0]) {
  case '<':
    return compileSingleComponent();

  case '[':
    {
      size_t lastIndex = expr_.size() - 1;
      if (']' != expr_[lastIndex])
        throw NdnRegexMatcherBase::Error
          ("Regexp compile error (no matching ']' in " + expr_ + ")");

      if ('^' == expr_[1]) {
        isInclusion_ = false;
        compileMultipleComponents(2, lastIndex);
      }
      else
        compileMultipleComponents(1, lastIndex);

      break;
    }

  default:
    throw NdnRegexMatcherBase::Error
      ("Regexp compile error (cannot parse " + expr_ + ")");
  }
}

void
NdnRegexComponentSetMatcher::compileSingleComponent()
{
  size_t end = extractComponent(1);

  if (expr_.size() != end)
    throw NdnRegexMatcherBase::Error("Component expr error " + expr_);
  else {
    ptr_lib::shared_ptr<NdnRegexComponentMatcher> component =
      ptr_lib::make_shared<NdnRegexComponentMatcher>
        (expr_.substr(1, end - 2), backrefManager_);

    components_.push_back(component);
  }
}

void
NdnRegexComponentSetMatcher::compileMultipleComponents
  (size_t start, size_t lastIndex)
{
  size_t index = start;
  size_t tempIndex = start;

  while (index < lastIndex) {
    if ('<' != expr_[index])
      throw NdnRegexMatcherBase::Error("Component expr error " + expr_);

    tempIndex = index + 1;
    index = extractComponent(tempIndex);

    ptr_lib::shared_ptr<NdnRegexComponentMatcher> component =
      ptr_lib::make_shared<NdnRegexComponentMatcher>
        (expr_.substr(tempIndex, index - tempIndex - 1), backrefManager_);

    components_.push_back(component);
  }

  if (index != lastIndex)
    throw NdnRegexMatcherBase::Error("Not sufficient expr to parse " + expr_);
}

bool
NdnRegexComponentSetMatcher::match(const Name& name, size_t offset, size_t len)
{
  bool isMatched = false;

  // ComponentSet only matches one component.
  if (len != 1)
    return false;

  for (vector<ptr_lib::shared_ptr<NdnRegexComponentMatcher> >::iterator it = components_.begin();
       it != components_.end();
       ++it) {
    if ((*it)->match(name, offset, len)) {
      isMatched = true;
      break;
    }
  }

  matchResult_.clear();

  if (isInclusion_ ? isMatched : !isMatched) {
    matchResult_.push_back(name.get(offset));
    return true;
  }
  else
    return false;
}

size_t
NdnRegexComponentSetMatcher::extractComponent(size_t index)
{
  size_t lcount = 1;
  size_t rcount = 0;

  while (lcount > rcount) {
    if (index >= expr_.size())
      throw NdnRegexMatcherBase::Error("Error: angle brackets mismatch");

    switch (expr_[index]) {
    case '<':
      ++lcount;
      break;

    case '>':
      ++rcount;
      break;
    }

    ++index;
  }

  return index;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
