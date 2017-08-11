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

#ifndef NDN_NDN_REGEX_TOP_MATCHER_HPP
#define NDN_NDN_REGEX_TOP_MATCHER_HPP

#include "ndn-regex-matcher-base.hpp"

// Only compile if we set NDN_CPP_HAVE_REGEX_LIB in ndn-regex-matcher-base.hpp.
#if NDN_CPP_HAVE_REGEX_LIB

namespace ndn {

class NdnRegexPatternListMatcher;
class NdnRegexBackrefManager;

class NdnRegexTopMatcher: public NdnRegexMatcherBase {
public:
  NdnRegexTopMatcher(const std::string& expr, const std::string& expand = "");

  virtual
  ~NdnRegexTopMatcher();

  bool
  match(const Name& name);

  virtual bool
  match(const Name& name, size_t offset, size_t len);

  virtual Name
  expand(const std::string& expand = "");

  static ptr_lib::shared_ptr<NdnRegexTopMatcher>
  fromName(const Name& name, bool hasAnchor=false);

protected:
  virtual void
  compile();

private:
  static std::string
  getItemFromExpand(const std::string& expand, size_t& offset);

  static std::string
  convertSpecialChar(const std::string& str);

private:
  const std::string expand_;
  ptr_lib::shared_ptr<NdnRegexPatternListMatcher> primaryMatcher_;
  ptr_lib::shared_ptr<NdnRegexPatternListMatcher> secondaryMatcher_;
  ptr_lib::shared_ptr<NdnRegexBackrefManager> primaryBackrefManager_;
  ptr_lib::shared_ptr<NdnRegexBackrefManager> secondaryBackrefManager_;
  bool isSecondaryUsed_;
};

}

#endif // NDN_CPP_HAVE_REGEX_LIB

#endif
