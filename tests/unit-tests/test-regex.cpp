/**
 * Copyright (C) 2014-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx Regex unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/util/regex.t.cpp
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

#include "../../src/util/regex/ndn-regex-backref-manager.hpp"
#include "../../src/util/regex/ndn-regex-backref-matcher.hpp"
#include "../../src/util/regex/ndn-regex-component-set-matcher.hpp"
#include "../../src/util/regex/ndn-regex-repeat-matcher.hpp"
#include "../../src/util/regex/ndn-regex-pattern-list-matcher.hpp"
#include "../../src/util/regex/ndn-regex-top-matcher.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace ndn;

class TestRegex : public ::testing::Test {
};

TEST_F(TestRegex, ComponentMatcher)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef = 
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexComponentMatcher> cm =
    ptr_lib::make_shared<NdnRegexComponentMatcher>("a", backRef);
  bool res = cm->match(Name("/a/b/"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexComponentMatcher>("a", backRef);
  res = cm->match(Name("/a/b/"), 1, 1);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexComponentMatcher>("(c+)\\.(cd)", backRef);
  res = cm->match(Name("/ccc.cd/b/"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("ccc.cd"), cm->getMatchResult()[0].toEscapedString());

  ASSERT_EQ(2, backRef->size());
  ASSERT_EQ(string("ccc"), backRef->getBackref(0)->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("cd"), backRef->getBackref(1)->getMatchResult()[0].toEscapedString());
}

TEST_F(TestRegex, ComponentSetMatcher)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef =
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexComponentSetMatcher> cm =
    ptr_lib::make_shared<NdnRegexComponentSetMatcher>("<a>", backRef);
  bool res = cm->match(Name("/a/b/"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());

  res = cm->match(Name("/a/b/"), 1, 1);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/"), 0, 2);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexComponentSetMatcher>("[<a><b><c>]", backRef);
  res = cm->match(Name("/a/b/d"), 1, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("b"), cm->getMatchResult()[0].toEscapedString());

  res = cm->match(Name("/a/b/d"), 2, 1);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexComponentSetMatcher>("[^<a><b><c>]", backRef);
  res = cm->match(Name("/b/d"), 1, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("d"), cm->getMatchResult()[0].toEscapedString());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  ASSERT_THROW(NdnRegexComponentSetMatcher("[<a]", backRef),
               NdnRegexMatcherBase::Error);
}

TEST_F(TestRegex, RepeatMatcher)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef = make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexRepeatMatcher> cm = make_shared<NdnRegexRepeatMatcher>
    ("[<a><b>]*", backRef, 8);
  bool res = cm->match(Name("/a/b/c"), 0, 0);
  ASSERT_EQ(true, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("[<a><b>]+", backRef, 8);
  res = cm->match(Name("/a/b/c"), 0, 0);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("<.*>*", backRef, 4);
  res = cm->match(Name("/a/b/c/d/e/f/"), 0, 6);
  ASSERT_EQ(true, res);
  ASSERT_EQ(6, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());
  ASSERT_EQ(string("e"), cm->getMatchResult()[4].toEscapedString());
  ASSERT_EQ(string("f"), cm->getMatchResult()[5].toEscapedString());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("<>*", backRef, 2);
  res = cm->match(Name("/a/b/c/d/e/f/"), 0, 6);
  ASSERT_EQ(true, res);
  ASSERT_EQ(6, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());
  ASSERT_EQ(string("e"), cm->getMatchResult()[4].toEscapedString());
  ASSERT_EQ(string("f"), cm->getMatchResult()[5].toEscapedString());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("<a>?", backRef, 3);
  res = cm->match(Name("/a/b/c"), 0, 0);
  ASSERT_EQ(true, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  cm = make_shared<NdnRegexRepeatMatcher>("<a>?", backRef, 3);
  res = cm->match(Name("/a/b/c"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());

  cm = make_shared<NdnRegexRepeatMatcher>("<a>?", backRef, 3);
  res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("[<a><b>]{3}", backRef, 8);
  res = cm->match(Name("/a/b/a/d/"), 0, 2);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/a/d/"), 0, 3);
  ASSERT_EQ(true, res);
  ASSERT_EQ(3, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("a"), cm->getMatchResult()[2].toEscapedString());

  res = cm->match(Name("/a/b/a/d/"), 0, 4);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("[<a><b>]{2,3}", backRef, 8);
  res = cm->match(Name("/a/b/a/d/e/"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  res = cm->match(Name("/a/b/a/d/e/"), 0, 3);
  ASSERT_EQ(true, res);
  ASSERT_EQ(3, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("a"), cm->getMatchResult()[2].toEscapedString());

  res = cm->match(Name("/a/b/a/b/e/"), 0, 4);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/a/d/e/"), 0, 1);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("[<a><b>]{2,}", backRef, 8);
  res = cm->match(Name("/a/b/a/d/e/"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  res = cm->match(Name("/a/b/a/b/e/"), 0, 4);
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("a"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[3].toEscapedString());

  res = cm->match(Name("/a/b/a/d/e/"), 0, 1);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = make_shared<NdnRegexBackrefManager>();
  cm = make_shared<NdnRegexRepeatMatcher>("[<a><b>]{,2}", backRef, 8);
  res = cm->match(Name("/a/b/a/b/e/"), 0, 3);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  res = cm->match(Name("/a/b/a/b/e/"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  res = cm->match(Name("/a/b/a/d/e/"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());

  res = cm->match(Name("/a/b/a/d/e/"), 0, 0);
  ASSERT_EQ(true, res);
  ASSERT_EQ(0, cm->getMatchResult().size());
}

TEST_F(TestRegex, BackRefMatcher)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef =
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexBackrefMatcher> cm =
    ptr_lib::make_shared<NdnRegexBackrefMatcher>("(<a><b>)", backRef);
  backRef->pushRef(ptr_lib::static_pointer_cast<NdnRegexMatcherBase>(cm));
  cm->lateCompile();
  bool res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(1, backRef->size());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexBackrefMatcher>("(<a>(<b>))", backRef);
  backRef->pushRef(cm);
  cm->lateCompile();
  res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(2, backRef->size());
  ASSERT_EQ(string("a"), backRef->getBackref(0)->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), backRef->getBackref(0)->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("b"), backRef->getBackref(1)->getMatchResult()[0].toEscapedString());
}

TEST_F(TestRegex, BackRefMatcherAdvanced)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef =
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexRepeatMatcher> cm =
    ptr_lib::make_shared<NdnRegexRepeatMatcher>("([<a><b>])+", backRef, 10);
  bool res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(1, backRef->size());
  ASSERT_EQ(string("b"), backRef->getBackref(0)->getMatchResult()[0].toEscapedString());
}

TEST_F(TestRegex, BackRefMatcherAdvanced2)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef =
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexPatternListMatcher> cm =
    ptr_lib::make_shared<NdnRegexPatternListMatcher>("(<a>(<b>))<c>", backRef);
  bool res = cm->match(Name("/a/b/c"), 0, 3);
  ASSERT_EQ(true, res);
  ASSERT_EQ(3, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(2, backRef->size());
  ASSERT_EQ(string("a"), backRef->getBackref(0)->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), backRef->getBackref(0)->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("b"), backRef->getBackref(1)->getMatchResult()[0].toEscapedString());
}

TEST_F(TestRegex, PatternListMatcher)
{
  ptr_lib::shared_ptr<NdnRegexBackrefManager> backRef =
    ptr_lib::make_shared<NdnRegexBackrefManager>();
  ptr_lib::shared_ptr<NdnRegexPatternListMatcher> cm =
    ptr_lib::make_shared<NdnRegexPatternListMatcher>("<a>[<a><b>]", backRef);
  bool res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(true, res);
  ASSERT_EQ(2, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexPatternListMatcher>("<>*<a>", backRef);
  res = cm->match(Name("/a/b/c"), 0, 1);
  ASSERT_EQ(true, res);
  ASSERT_EQ(1, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexPatternListMatcher>("<>*<a>", backRef);
  res = cm->match(Name("/a/b/c"), 0, 2);
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  backRef = ptr_lib::make_shared<NdnRegexBackrefManager>();
  cm = ptr_lib::make_shared<NdnRegexPatternListMatcher>("<>*<a><>*", backRef);
  res = cm->match(Name("/a/b/c"), 0, 3);
  ASSERT_EQ(true, res);
  ASSERT_EQ(3, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
}

TEST_F(TestRegex, TopMatcher)
{
  ptr_lib::shared_ptr<NdnRegexTopMatcher> cm =
    ptr_lib::make_shared<NdnRegexTopMatcher>("^<a><b><c>");
  bool res = cm->match(Name("/a/b/c/d"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("<b><c><d>$");
  res = cm->match(Name("/a/b/c/d"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("^<a><b><c><d>$");
  res = cm->match(Name("/a/b/c/d"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());

  res = cm->match(Name("/a/b/c/d/e"));
  ASSERT_EQ(false, res);
  ASSERT_EQ(0, cm->getMatchResult().size());

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("<a><b><c><d>");
  res = cm->match(Name("/a/b/c/d"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("<b><c>");
  res = cm->match(Name("/a/b/c/d"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(string("a"), cm->getMatchResult()[0].toEscapedString());
  ASSERT_EQ(string("b"), cm->getMatchResult()[1].toEscapedString());
  ASSERT_EQ(string("c"), cm->getMatchResult()[2].toEscapedString());
  ASSERT_EQ(string("d"), cm->getMatchResult()[3].toEscapedString());
}

TEST_F(TestRegex, TopMatcherAdvanced)
{
  ptr_lib::shared_ptr<NdnRegexTopMatcher> cm =
    ptr_lib::make_shared<NdnRegexTopMatcher>("^(<.*>*)<.*>");
  bool res = cm->match(Name("/n/a/b/c"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(Name("/n/a/b/"), cm->expand("\\1"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("^(<.*>*)<.*><c>(<.*>)<.*>");
  res = cm->match(Name("/n/a/b/c/d/e/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(6, cm->getMatchResult().size());
  ASSERT_EQ(Name("/n/a/d/"), cm->expand("\\1\\2"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("(<.*>*)<.*>$");
  res = cm->match(Name("/n/a/b/c/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(Name("/n/a/b/"), cm->expand("\\1"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("<.*>(<.*>*)<.*>$");
  res = cm->match(Name("/n/a/b/c/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(Name("/a/b/"), cm->expand("\\1"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("<a>(<>*)<>$");
  res = cm->match(Name("/n/a/b/c/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(4, cm->getMatchResult().size());
  ASSERT_EQ(Name("/b/"), cm->expand("\\1"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>("^<ndn><(.*)\\.(.*)><DNS>(<>*)<>");
  res = cm->match(Name("/ndn/ucla.edu/DNS/yingdi/mac/ksk-1/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(6, cm->getMatchResult().size());
  ASSERT_EQ(Name("/ndn/edu/ucla/yingdi/mac/"), cm->expand("<ndn>\\2\\1\\3"));

  cm = ptr_lib::make_shared<NdnRegexTopMatcher>
    ("^<ndn><(.*)\\.(.*)><DNS>(<>*)<>", "<ndn>\\2\\1\\3");
  res = cm->match(Name("/ndn/ucla.edu/DNS/yingdi/mac/ksk-1/"));
  ASSERT_EQ(true, res);
  ASSERT_EQ(6, cm->getMatchResult().size());
  ASSERT_EQ(Name("/ndn/edu/ucla/yingdi/mac/"), cm->expand());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
