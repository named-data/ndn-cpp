/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
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

#include "gtest/gtest.h"
#include <algorithm>
#include <ndn-cpp/name.hpp>

using namespace std;
using namespace ndn;

static const uint8_t COMP2[] = { 0x00, 0x01, 0x02, 0x03 };

class TestNameMethods : public ::testing::Test {
public:
  TestNameMethods()
  : comp2(Blob(COMP2, sizeof(COMP2))),
    expectedURI("/entr%C3%A9e/..../%00%01%02%03")
  {}

  const Name::Component comp2;
  const string expectedURI;
};

TEST_F(TestNameMethods, UriConstructor)
{
  Name name(expectedURI);
  ASSERT_EQ(name.size(), 3) << "Constructed name has " << name.size() << " components instead of 3";
  ASSERT_EQ(name.toUri(), expectedURI) << "URI is incorrect";
}

TEST_F(TestNameMethods, CopyConstructor)
{
  Name name(expectedURI);
  Name name2(name);
  ASSERT_TRUE(name.equals(name2)) << "Name from copy constructor does not match original";
}

TEST_F(TestNameMethods, GetComponent)
{
  Name name(expectedURI);
  Name::Component component2 = name.get(2);
  ASSERT_TRUE(comp2.equals(component2)) << "Component at index 2 is incorrect";
}

TEST_F(TestNameMethods, Append)
{
  // could possibly split this into different tests
  string uri = "/localhost/user/folders/files/%00%0F";
  Name name(uri);
  Name name2 = Name("/localhost").append(Name("/user/folders/"));
  ASSERT_EQ(name2.size(), 3) << "Name constructed by appending names has " << name2.size() << " components instead of 3";
  string folders("folders");
  ASSERT_TRUE(name2.get(2).getValue().equals(Blob((const uint8_t*)&folders[0], folders.size()))) << "Name constructed with append has wrong suffix";
  name2.append("files");
  ASSERT_EQ(name2.size(), 4) << "Name constructed by appending string has " << name2.size() << " components instead of 4";
  name2.appendSegment(15);
  ASSERT_TRUE(name2.get(4).getValue().equals(Blob((const uint8_t*)"\x00\x0f", 2))) << "Name constructed by appending segment has wrong segment value";

  ASSERT_TRUE(name2.equals(name)) << "Name constructed with append is not equal to URI constructed name";
  ASSERT_EQ(name2.toUri(), name.toUri()) << "Name constructed with append has wrong URI";
}

TEST_F(TestNameMethods, Prefix)
{
  Name name("/edu/cmu/andrew/user/3498478");
  Name prefix1 = name.getPrefix(2);

  ASSERT_EQ(prefix1.size(), 2) << "Name prefix has " << prefix1.size() << " components instead of 2";
  for (size_t i = 0; i < 2; ++i)
    ASSERT_TRUE(name.get(i).getValue().equals(prefix1.get(i).getValue()));

  Name prefix2 = name.getPrefix(100);
  ASSERT_EQ(prefix2, name) << "Prefix with more components than original should stop at end of original name";
}

TEST_F(TestNameMethods, SubName)
{
  Name name("/edu/cmu/andrew/user/3498478");
  Name subName1 = name.getSubName(0);
  ASSERT_EQ(subName1, name) << "Subname from first component does not match original name";
  Name subName2 = name.getSubName(3);
  ASSERT_EQ(subName2.toUri(), "/user/3498478");

  Name subName3 = name.getSubName(1, 3);
  ASSERT_EQ(subName3.toUri(), "/cmu/andrew/user");

  Name subName4 = name.getSubName(0, 100);
  ASSERT_EQ(name, subName4) << "Subname with more components than original should stop at end of original name";

  Name subName5 = name.getSubName(7, 2);
  ASSERT_EQ(Name(), subName5) << "Subname beginning after end of name should be empty";

  Name subName6 = name.getSubName(-1,7);
  ASSERT_EQ(subName6, Name("/3498478")) << "Negative subname with more components than original should stop at end of original name";

  Name subName7 = name.getSubName(-5,5);
  ASSERT_EQ(subName7, name) << "Subname from (-length) should match original name";
}

TEST_F(TestNameMethods, Clear)
{
  Name name(expectedURI);
  name.clear();
  ASSERT_TRUE(Name().equals(name)) << "Cleared name is not empty";
}

TEST_F(TestNameMethods, Compare)
{
  const char* namesValues[] = { "/a/b/d", "/c", "/c/a", "/bb", "/a/b/cc" };
  vector<Name> names(namesValues, namesValues + 5);
  const char* expectedOrderValues[] = { "/a/b/d", "/a/b/cc", "/c", "/c/a", "/bb" };
  vector<string> expectedOrder(expectedOrderValues, expectedOrderValues + 5);
  // Sort using Name::operator< .
  ::sort(names.begin(), names.end());

  vector<string> sortedURIs;
  for (size_t i = 0; i < names.size(); ++i)
    sortedURIs.push_back(names[i].toUri());

  ASSERT_EQ(sortedURIs, expectedOrder) << "Name comparison gave incorrect order";
}

TEST_F(TestNameMethods, Match)
{
  Name name("/edu/cmu/andrew/user/3498478");
  Name name1 = name;
  ASSERT_TRUE(name.match(name1)) << "Name does not match deep copy of itself";

  Name name2 = name.getPrefix(2);
  ASSERT_TRUE(name2.match(name)) << "Name did not match prefix";
  ASSERT_FALSE(name.match(name2)) << "Name should not match shorter name";
  ASSERT_TRUE(Name().match(name)) << "Empty name should always match another";
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

