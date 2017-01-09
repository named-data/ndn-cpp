/**
 * Copyright (C) 2014-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/name.t.cpp
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
#include <ndn-cpp/encoding/tlv-wire-format.hpp>

using namespace std;
using namespace ndn;

static const uint8_t COMP2[] = { 0x00, 0x01, 0x02, 0x03 };

static const uint8_t TEST_NAME[] = {
  0x7,  0x14, // Name
    0x8,  0x5, // NameComponent
        0x6c,  0x6f,  0x63,  0x61,  0x6c,
    0x8,  0x3, // NameComponent
        0x6e,  0x64,  0x6e,
    0x8,  0x6, // NameComponent
        0x70,  0x72,  0x65,  0x66,  0x69,  0x78
};

static const uint8_t TEST_NAME_IMPLICIT_DIGEST[] = {
  0x7,  0x36, // Name
    0x8,  0x5, // NameComponent
        0x6c,  0x6f,  0x63,  0x61,  0x6c,
    0x8,  0x3, // NameComponent
        0x6e,  0x64,  0x6e,
    0x8,  0x6, // NameComponent
        0x70,  0x72,  0x65,  0x66,  0x69,  0x78,
    0x01, 0x20, // ImplicitSha256DigestComponent
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

class TestNameComponentMethods : public ::testing::Test {
};

TEST_F(TestNameComponentMethods, Compare)
{
  Name c7fName("/%7F");
  Name c80Name("/%80");
  Name c81Name("/%81");

  const Name::Component& c7f = c7fName.get(0);
  const Name::Component& c80 = c80Name.get(0);
  const Name::Component& c81 = c81Name.get(0);

  ASSERT_TRUE(c81.compare(c80) > 0) << "%81 should be greater than %80";
  ASSERT_TRUE(c80.compare(c7f) > 0) << "%80 should be greater than %7f";
}

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

  // Tests from ndn-cxx name.t.cpp Compare.
  ASSERT_EQ(Name("/A")  .compare(Name("/A")),   0);
  ASSERT_EQ(Name("/A")  .compare(Name("/A")),   0);
  ASSERT_LT(Name("/A")  .compare(Name("/B")),   0);
  ASSERT_GT(Name("/B")  .compare(Name("/A")),   0);
  ASSERT_LT(Name("/A")  .compare(Name("/AA")),  0);
  ASSERT_GT(Name("/AA") .compare(Name("/A")),   0);
  ASSERT_LT(Name("/A")  .compare(Name("/A/C")), 0);
  ASSERT_GT(Name("/A/C").compare(Name("/A")),   0);

  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/A")),   0);
  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/B")),   0);
  ASSERT_GT(Name("/Z/B/Y")  .compare(1, 1, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/AA")),  0);
  ASSERT_GT(Name("/Z/AA/Y") .compare(1, 1, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/A/C")), 0);
  ASSERT_GT(Name("/Z/A/C/Y").compare(1, 2, Name("/A")),   0);

  ASSERT_EQ(Name("/Z/A")  .compare(1, 9, Name("/A")),   0);
  ASSERT_EQ(Name("/Z/A")  .compare(1, 9, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A")  .compare(1, 9, Name("/B")),   0);
  ASSERT_GT(Name("/Z/B")  .compare(1, 9, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A")  .compare(1, 9, Name("/AA")),  0);
  ASSERT_GT(Name("/Z/AA") .compare(1, 9, Name("/A")),   0);
  ASSERT_LT(Name("/Z/A")  .compare(1, 9, Name("/A/C")), 0);
  ASSERT_GT(Name("/Z/A/C").compare(1, 9, Name("/A")),   0);

  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A/W"),   1, 1), 0);
  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A/W"),   1, 1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/B/W"),   1, 1), 0);
  ASSERT_GT(Name("/Z/B/Y")  .compare(1, 1, Name("/X/A/W"),   1, 1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/AA/W"),  1, 1), 0);
  ASSERT_GT(Name("/Z/AA/Y") .compare(1, 1, Name("/X/A/W"),   1, 1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A/C/W"), 1, 2), 0);
  ASSERT_GT(Name("/Z/A/C/Y").compare(1, 2, Name("/X/A/W"),   1, 1), 0);

  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A"),   1), 0);
  ASSERT_EQ(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A"),   1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/B"),   1), 0);
  ASSERT_GT(Name("/Z/B/Y")  .compare(1, 1, Name("/X/A"),   1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/AA"),  1), 0);
  ASSERT_GT(Name("/Z/AA/Y") .compare(1, 1, Name("/X/A"),   1), 0);
  ASSERT_LT(Name("/Z/A/Y")  .compare(1, 1, Name("/X/A/C"), 1), 0);
  ASSERT_GT(Name("/Z/A/C/Y").compare(1, 2, Name("/X/A"),   1), 0);
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

TEST_F(TestNameMethods, GetSuccessor)
{
  ASSERT_EQ(Name("ndn:/%00%01/%01%03"), Name("ndn:/%00%01/%01%02").getSuccessor());
  ASSERT_EQ(Name("ndn:/%00%01/%02%00"), Name("ndn:/%00%01/%01%FF").getSuccessor());
  ASSERT_EQ(Name("ndn:/%00%01/%00%00%00"), Name("ndn:/%00%01/%FF%FF").getSuccessor());
  ASSERT_EQ(Name("/%00"), Name().getSuccessor());
  ASSERT_EQ(Name("/%00%01/%00"), Name("/%00%01/...").getSuccessor());
}

TEST_F(TestNameMethods, EncodeDecode)
{
  Name name("/local/ndn/prefix");

  Blob encoding = name.wireEncode(*TlvWireFormat::get());
  ASSERT_TRUE(encoding.equals(Blob(TEST_NAME, sizeof(TEST_NAME))));

  Name decodedName;
  decodedName.wireDecode(Blob(TEST_NAME, sizeof(TEST_NAME)), *TlvWireFormat::get());
  ASSERT_EQ(decodedName, name);

  // Test ImplicitSha256Digest.
  Name name2("/local/ndn/prefix/sha256digest="
             "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");

  Blob encoding2 = name2.wireEncode(*TlvWireFormat::get());
  ASSERT_TRUE(encoding2.equals
    (Blob(TEST_NAME_IMPLICIT_DIGEST, sizeof(TEST_NAME_IMPLICIT_DIGEST))));

  Name decodedName2;
  decodedName2.wireDecode(Blob(TEST_NAME_IMPLICIT_DIGEST, sizeof(TEST_NAME_IMPLICIT_DIGEST)),
                          *TlvWireFormat::get());
  ASSERT_EQ(decodedName2, name2);
}

TEST_F(TestNameMethods, ImplicitSha256Digest)
{
  Name name;

  uint8_t digest[] = {
    0x28, 0xba, 0xd4, 0xb5, 0x27, 0x5b, 0xd3, 0x92,
    0xdb, 0xb6, 0x70, 0xc7, 0x5c, 0xf0, 0xb6, 0x6f,
    0x13, 0xf7, 0x94, 0x2b, 0x21, 0xe8, 0x0f, 0x55,
    0xc0, 0xe8, 0x6b, 0x37, 0x47, 0x53, 0xa5, 0x48,
    0x00, 0x00
  };

  name.appendImplicitSha256Digest(digest, 32);
  name.appendImplicitSha256Digest(Blob(digest, 32));
  ASSERT_EQ(name.get(0), name.get(1));

  ASSERT_THROW
    (name.appendImplicitSha256Digest(Blob(digest, 34)),
    runtime_error) << "Expected error in appendImplicitSha256Digest";
  ASSERT_THROW
    (name.appendImplicitSha256Digest(Blob(digest, 30)),
    runtime_error) << "Expected error in appendImplicitSha256Digest";

  // Add name.get(2) as a generic component.
  name.append(digest, 32);
  ASSERT_TRUE(name.get(0).compare(name.get(2)) < 0);
  ASSERT_TRUE(name.get(0).getValue().equals(name.get(2).getValue()));

  // Add name.get(3) as a generic component whose first byte is greater.
  name.append(digest + 1, 32);
  ASSERT_TRUE(name.get(0).compare(name.get(3)) < 0);

  ASSERT_EQ
    ("sha256digest="
     "28bad4b5275bd392dbb670c75cf0b66f13f7942b21e80f55c0e86b374753a548",
     name.get(0).toEscapedString());

  ASSERT_EQ(true, name.get(0).isImplicitSha256Digest());
  ASSERT_EQ(false, name.get(2).isImplicitSha256Digest());

  ASSERT_THROW
    (Name("/hello/sha256digest=hmm"),
    runtime_error) << "Expected error in new Name from URI";

  // Check canonical URI encoding (lower case).
  Name name2
    ("/hello/sha256digest="
     "28bad4b5275bd392dbb670c75cf0b66f13f7942b21e80f55c0e86b374753a548");
  ASSERT_EQ(name.get(0), name2.get(1));

  // Check that it will accept a hex value in upper case too.
  name2 = Name
    ("/hello/sha256digest="
     "28BAD4B5275BD392DBB670C75CF0B66F13F7942B21E80F55C0E86B374753A548");
  ASSERT_EQ(name.get(0), name2.get(1));

  // This is not valid sha256digest component. It should be treated as generic.
  name2 = Name
    ("/hello/SHA256DIGEST="
     "28BAD4B5275BD392DBB670C75CF0B66F13F7942B21E80F55C0E86B374753A548");
  ASSERT_FALSE(name.get(0).equals(name2.get(1)));
  ASSERT_TRUE(name2.get(1).isGeneric());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

