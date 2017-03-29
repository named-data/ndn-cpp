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

#include "ndn-regex-matcher-base.hpp"
#include "ndn-regex-backref-manager.hpp"

using namespace std;

namespace ndn {

NdnRegexMatcherBase::Error::Error(const string& errorMessage) throw()
: errorMessage_(errorMessage)
{
}

NdnRegexMatcherBase::Error::~Error() throw()
{
}

const char*
NdnRegexMatcherBase::Error::what() const throw() { return errorMessage_.c_str(); }

NdnRegexMatcherBase::NdnRegexMatcherBase
  (const string& expr, const NdnRegexExprType& type,
   ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager)
: expr_(expr), type_(type), backrefManager_(backrefManager)
{
  if (!backrefManager_)
    backrefManager_ = ptr_lib::make_shared<NdnRegexBackrefManager>();
}

NdnRegexMatcherBase::~NdnRegexMatcherBase() {}

bool
NdnRegexMatcherBase::match(const Name& name, size_t offset, size_t len)
{
  bool result = false;

  matchResult_.clear();

  if (recursiveMatch(0, name, offset, len)) {
    for (size_t i = offset; i < offset + len; i++)
      matchResult_.push_back(name.get(i));
    result = true;
  }
  else
    result = false;

  return result;
}

bool
NdnRegexMatcherBase::recursiveMatch
  (size_t matcherNo, const Name& name, size_t offset, size_t len)
{
  int tried = len;

  if (matcherNo >= matchers_.size())
    return (len == 0);

  ptr_lib::shared_ptr<NdnRegexMatcherBase> matcher = matchers_[matcherNo];

  while (tried >= 0) {
    if (matcher->match(name, offset, tried) &&
        recursiveMatch(matcherNo + 1, name, offset + tried, len - tried))
      return true;
    --tried;
  }

  return false;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB
