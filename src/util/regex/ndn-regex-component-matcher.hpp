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

#ifndef NDN_NDN_REGEX_COMPONENT_MATCHER_HPP
#define NDN_NDN_REGEX_COMPONENT_MATCHER_HPP

#include "ndn-regex-pseudo-matcher.hpp"
#include "ndn-regex-matcher-base.hpp"

namespace ndn {

class NdnRegexComponentMatcher : public NdnRegexMatcherBase {
public:
  /**
   * Create a RegexComponent matcher from expr.
   * @param expr The standard regular expression to match a component.
   * @param backrefManager The back reference manager.
   * @param isExactMatch The flag to provide exact match.
   */
  NdnRegexComponentMatcher
    (const std::string& expr,
     ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager,
     bool isExactMatch = true)
  : NdnRegexMatcherBase(expr, NDN_REGEX_EXPR_COMPONENT, backrefManager),
    isExactMatch_(isExactMatch)
  {
    compile();
  }

  virtual
  ~NdnRegexComponentMatcher();

  virtual bool
  match(const Name& name, size_t offset, size_t len);

protected:
  /**
   * Compile the regular expression to generate more matchers when necessary.
   */
  virtual void
  compile();

private:
  bool isExactMatch_;
  regex_lib::regex componentRegex_;
  std::vector<ptr_lib::shared_ptr<NdnRegexPseudoMatcher> > pseudoMatchers_;
};


}

#endif
