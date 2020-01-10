/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
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

#ifndef NDN_NDN_REGEX_BACKREF_MATCHER_HPP
#define NDN_NDN_REGEX_BACKREF_MATCHER_HPP

#include "ndn-regex-matcher-base.hpp"

namespace ndn {

class NdnRegexBackrefMatcher : public NdnRegexMatcherBase {
public:
  NdnRegexBackrefMatcher
    (const std::string& expr,
     ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager)
  : NdnRegexMatcherBase(expr, NDN_REGEX_EXPR_BACKREF, backrefManager)
  {
    // compile();
  }

  virtual
  ~NdnRegexBackrefMatcher();

  void
  lateCompile() { compile(); }

protected:
  virtual void
  compile();
};

}

#endif
