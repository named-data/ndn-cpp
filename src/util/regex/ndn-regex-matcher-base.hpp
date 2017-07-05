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

#ifndef NDN_NDN_REGEX_MATCHER_BASE_HPP
#define NDN_NDN_REGEX_MATCHER_BASE_HPP

// Depending if ndn-cpp-config.h defines NDN_CPP_HAVE_BOOST_REGEX or sets
//   NDN_CPP_HAVE_STD_REGEX = 1, define the regex_lib namespace alias.
// Set NDN_CPP_HAVE_REGEX_LIB = 1 if regex_lib is defined.

#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_STD_REGEX
  #include <regex>
  namespace ndn { namespace regex_lib = std; }
  #define NDN_CPP_HAVE_REGEX_LIB 1

  // Only Boost needs the correction.
  static const size_t NDN_REGEXP_MARK_COUNT_CORRECTION = 0;
#elif defined(NDN_CPP_HAVE_BOOST_REGEX)
  #include <boost/regex.hpp>
  #include <boost/version.hpp>
  namespace ndn { namespace regex_lib = boost; }
  #define NDN_CPP_HAVE_REGEX_LIB 1

  // Re: http://www.boost.org/users/history/version_1_56_0.html
  // Correct the behavior of basic_regex<>::mark_count() to match existing
  // documentation, basic_regex<>::subexpression(n) changed to match, see
  // https://svn.boost.org/trac/boost/ticket/9227
  static const size_t NDN_REGEXP_MARK_COUNT_CORRECTION =
  #if BOOST_VERSION < 105600
    1;
  #else
    0;
  #endif
#else
  #define NDN_CPP_HAVE_REGEX_LIB 0
#endif

// Only compile if we set NDN_CPP_HAVE_REGEX_LIB.
#if NDN_CPP_HAVE_REGEX_LIB

#include <string>
#include <ndn-cpp/name.hpp>

namespace ndn {

class NdnRegexBackrefManager;

class NdnRegexMatcherBase {
public:
  /**
   * NdnRegexMatcherBase::Error extends std::exception for errors using
   * NdnRegexMatcherBase methods.
   */
  class Error : public std::exception {
  public:
    Error(const std::string& errorMessage) throw();

    virtual ~Error() throw();

    std::string
    Msg() const { return errorMessage_; }

    virtual const char*
    what() const throw();

  private:
    const std::string errorMessage_;
  };

  enum NdnRegexExprType {
    NDN_REGEX_EXPR_TOP,
    NDN_REGEX_EXPR_PATTERN_LIST,
    NDN_REGEX_EXPR_REPEAT_PATTERN,
    NDN_REGEX_EXPR_BACKREF,
    NDN_REGEX_EXPR_COMPONENT_SET,
    NDN_REGEX_EXPR_COMPONENT,
    NDN_REGEX_EXPR_PSEUDO
  };

  NdnRegexMatcherBase
    (const std::string& expr, const NdnRegexExprType& type,
     ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager =
       ptr_lib::shared_ptr<NdnRegexBackrefManager>());

  virtual
  ~NdnRegexMatcherBase();

  virtual bool
  match(const Name& name, size_t offset, size_t len);

  /**
   * Get the list of matched name components.
   * @return The matched name components.
   */
  const std::vector<Name::Component>&
  getMatchResult() const { return matchResult_; }

  const std::string&
  getExpr() const { return expr_; }

protected:
  /**
   * Compile the regular expression to generate more matchers when necessary.
   */
  virtual void
  compile() = 0;

private:
  bool
  recursiveMatch(size_t matcherNo, const Name& name, size_t offset, size_t len);

protected:
  const std::string expr_;
  const NdnRegexExprType type_;
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backrefManager_;
  std::vector<ptr_lib::shared_ptr<NdnRegexMatcherBase> > matchers_;
  std::vector<Name::Component> matchResult_;
};

inline std::ostream&
operator<<(std::ostream& os, const NdnRegexMatcherBase& regex)
{
  os << regex.getExpr();
  return os;
}

}

#endif // NDN_CPP_HAVE_REGEX_LIB

#endif
