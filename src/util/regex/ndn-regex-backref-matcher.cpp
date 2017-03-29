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

#include "ndn-regex-pattern-list-matcher.hpp"
#include "ndn-regex-backref-matcher.hpp"

using namespace std;

namespace ndn {

NdnRegexBackrefMatcher::~NdnRegexBackrefMatcher() {}

void
NdnRegexBackrefMatcher::compile()
{
  if (expr_.size() < 2)
    throw NdnRegexMatcherBase::Error("Unrecognized format: " + expr_);

  size_t lastIndex = expr_.size() - 1;
  if ('(' == expr_[0] && ')' == expr_[lastIndex]) {
    // backRefManager_->pushRef(this);

    ptr_lib::shared_ptr<NdnRegexMatcherBase> matcher
      (new NdnRegexPatternListMatcher
       (expr_.substr(1, lastIndex - 1), backrefManager_));
    matchers_.push_back(matcher);
  }
  else
    throw NdnRegexMatcherBase::Error("Unrecognized format: " + expr_);
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
