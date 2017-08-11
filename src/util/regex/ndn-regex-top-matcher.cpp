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
#include "ndn-regex-pattern-list-matcher.hpp"
#include "ndn-regex-top-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexTopMatcher::NdnRegexTopMatcher(const string& expr, const string& expand)
: NdnRegexMatcherBase(expr, NDN_REGEX_EXPR_TOP), expand_(expand),
  isSecondaryUsed_(false)
{
  primaryBackrefManager_ = ptr_lib::make_shared<NdnRegexBackrefManager>();
  secondaryBackrefManager_ = ptr_lib::make_shared<NdnRegexBackrefManager>();
  compile();
}

NdnRegexTopMatcher::~NdnRegexTopMatcher()
{
}

void
NdnRegexTopMatcher::compile()
{
  string errMsg = "Error: RegexTopMatcher.Compile(): ";

  string expr = expr_;

  if ('$' != expr[expr.size() - 1])
    expr = expr + "<.*>*";
  else
    expr = expr.substr(0, expr.size() - 1);

  if ('^' != expr[0])
    secondaryMatcher_ = ptr_lib::make_shared<NdnRegexPatternListMatcher>
      ("<.*>*" + expr, secondaryBackrefManager_);
  else
    expr = expr.substr(1, expr.size() - 1);

  primaryMatcher_ = ptr_lib::make_shared<NdnRegexPatternListMatcher>
    (expr, primaryBackrefManager_);
}

bool
NdnRegexTopMatcher::match(const Name& name)
{
  isSecondaryUsed_ = false;

  matchResult_.clear();

  if (primaryMatcher_->match(name, 0, name.size())) {
    matchResult_ = primaryMatcher_->getMatchResult();
    return true;
  }
  else {
    if (secondaryMatcher_ && secondaryMatcher_->match(name, 0, name.size())) {
      matchResult_ = secondaryMatcher_->getMatchResult();
      isSecondaryUsed_ = true;
      return true;
    }

    return false;
  }
}

bool
NdnRegexTopMatcher::match(const Name& name, size_t offset, size_t len)
{
  return match(name);
}

Name
NdnRegexTopMatcher::expand(const string& expandStr)
{
  Name result;

  ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager =
    (isSecondaryUsed_ ? secondaryBackrefManager_ : primaryBackrefManager_);

  size_t backrefNo = backrefManager->size();

  string expand;

  if (!expandStr.empty())
    expand = expandStr;
  else
    expand = expand_;

  size_t offset = 0;
  while (offset < expand.size()) {
    string item = getItemFromExpand(expand, offset);
    if (item[0] == '<')
      result.append(item.substr(1, item.size() - 2));

    if (item[0] == '\\') {
      size_t index = ::atoi(item.substr(1, item.size() - 1).c_str());

      if (0 == index) {
        vector<Name::Component>::iterator it = matchResult_.begin();
        vector<Name::Component>::iterator end = matchResult_.end();
        for (; it != end; ++it)
          result.append(*it);
      }
      else if (index <= backrefNo) {
        vector<Name::Component>::const_iterator it =
          backrefManager->getBackref(index - 1)->getMatchResult().begin();
        vector<Name::Component>::const_iterator end =
          backrefManager->getBackref(index - 1)->getMatchResult().end();
        for (; it != end; ++it)
          result.append(*it);
      }
      else
        throw NdnRegexMatcherBase::Error("Exceed the range of back reference");
    }
  }

  return result;
}

string
NdnRegexTopMatcher::getItemFromExpand(const string& expand, size_t& offset)
{
  size_t begin = offset;

  if (expand[offset] == '\\') {
    ++offset;
    if (offset >= expand.size())
      throw NdnRegexMatcherBase::Error("Wrong format of expand string!");

    while (offset < expand.size() &&
           expand[offset] <= '9' && expand[offset] >= '0') {
      ++offset;
      if (offset > expand.size())
        throw NdnRegexMatcherBase::Error("Wrong format of expand string!");
    }

    if (offset > begin + 1)
      return expand.substr(begin, offset - begin);
    else
      throw NdnRegexMatcherBase::Error("Wrong format of expand string!");
  }
  else if (expand[offset] == '<') {
    ++offset;
    if (offset >= expand.size())
      throw NdnRegexMatcherBase::Error("Wrong format of expand string!");

    size_t left = 1;
    size_t right = 0;
    while (right < left) {
      if (expand[offset] == '<')
        ++left;
      if (expand[offset] == '>')
        ++right;

      ++offset;
      if (offset >= expand.size())
        throw NdnRegexMatcherBase::Error("Wrong format of expand string!");
    }

    return expand.substr(begin, offset - begin);
  }
  else
    throw NdnRegexMatcherBase::Error("Wrong format of expand string!");
}

ptr_lib::shared_ptr<NdnRegexTopMatcher>
NdnRegexTopMatcher::fromName(const Name& name, bool hasAnchor)
{
  string regexStr("^");

  for (size_t i = 0; i < name.size(); ++i) {
    regexStr.append("<");
    regexStr.append(convertSpecialChar(name.get(i).toEscapedString()));
    regexStr.append(">");
  }

  if (hasAnchor)
    regexStr.append("$");

  return ptr_lib::make_shared<NdnRegexTopMatcher>(regexStr);
}

string
NdnRegexTopMatcher::convertSpecialChar(const string& str)
{
  string newStr;
  for (size_t i = 0; i < str.size(); ++i) {
    char c = str[i];
    switch (c) {
      case '.':
      case '[':
      case '{':
      case '}':
      case '(':
      case ')':
      case '\\':
      case '*':
      case '+':
      case '?':
      case '|':
      case '^':
      case '$':
        newStr.push_back('\\');
        // Fall through.
      default:
        newStr.push_back(c);
        break;
    }
  }

  return newStr;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
