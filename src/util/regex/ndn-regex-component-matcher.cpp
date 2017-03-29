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
#include "ndn-regex-component-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexComponentMatcher::~NdnRegexComponentMatcher() {}

void
NdnRegexComponentMatcher::compile()
{
  componentRegex_ = regex_lib::regex(expr_);

  pseudoMatchers_.clear();
  pseudoMatchers_.push_back(ptr_lib::make_shared<NdnRegexPseudoMatcher>());

  for (size_t i = 1;
       i <= componentRegex_.mark_count() - NDN_REGEXP_MARK_COUNT_CORRECTION;
       ++i) {
    ptr_lib::shared_ptr<NdnRegexPseudoMatcher> pMatcher =
      ptr_lib::make_shared<NdnRegexPseudoMatcher>();
    pseudoMatchers_.push_back(pMatcher);
    backrefManager_->pushRef
      (ptr_lib::static_pointer_cast<NdnRegexMatcherBase>(pMatcher));
  }
}

bool
NdnRegexComponentMatcher::match
  (const Name& name, size_t offset, size_t len)
{
  matchResult_.clear();

  if (expr_.empty()) {
    matchResult_.push_back(name.get(offset));
    return true;
  }

  if (isExactMatch_) {
    regex_lib::smatch subResult;
    string targetStr = name.get(offset).toEscapedString();
    if (regex_lib::regex_match(targetStr, subResult, componentRegex_)) {
      for (size_t i = 1;
           i <= componentRegex_.mark_count() - NDN_REGEXP_MARK_COUNT_CORRECTION;
           ++i) {
        pseudoMatchers_[i]->resetMatchResult();
        pseudoMatchers_[i]->setMatchResult(subResult[i]);
      }

      matchResult_.push_back(name.get(offset));
      return true;
    }
  }
  else
    throw NdnRegexMatcherBase::Error
      ("Non-exact component search is not supported yet");

  return false;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
