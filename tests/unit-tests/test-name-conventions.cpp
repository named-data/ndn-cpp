/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx NamingConventions unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/test-name.cpp.
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
#include <ndn-cpp/name.hpp>

using namespace std;
using namespace ndn;

class TestNameConventions : public ::testing::Test {
};

TEST_F(TestNameConventions, NumberWithMarker)
{
  const Name& expected("/%AA%03%E8");
  uint64_t number = 1000;
  uint8_t marker = 0xAA;
  ASSERT_EQ(Name().append(Name::Component::fromNumberWithMarker(number, marker)), expected) << "fromNumberWithMarker did not create the expected component";
  ASSERT_EQ(expected[0].toNumberWithMarker(marker), number) << "toNumberWithMarker did not return the expected value";
}

TEST_F(TestNameConventions, Segment)
{
  const Name& expected("/%00%27%10");
  uint64_t number = 10000;
  ASSERT_EQ(Name().appendSegment(number), expected) << "appendSegment did not create the expected component";
  ASSERT_EQ(expected[0].toSegment(), number) << "toSegment did not return the expected value";
}

TEST_F(TestNameConventions, SegmentOffset)
{
  const Name& expected("/%FB%00%01%86%A0");
  uint64_t number = 100000;
  ASSERT_EQ(Name().appendSegmentOffset(number), expected) << "appendSegmentOffset did not create the expected component";
  ASSERT_EQ(expected[0].toSegmentOffset(), number) << "toSegmentOffset did not return the expected value";
}

TEST_F(TestNameConventions, Version)
{
  const Name& expected("/%FD%00%0FB%40");
  uint64_t number = 1000000;
  ASSERT_EQ(Name().appendVersion(number), expected) << "appendVersion did not create the expected component";
  ASSERT_EQ(expected[0].toVersion(), number) << "toVersion did not return the expected value";
}

TEST_F(TestNameConventions, SequenceNumber)
{
  const Name& expected("/%FE%00%98%96%80");
  uint64_t number = 10000000;
  ASSERT_EQ(Name().appendSequenceNumber(number), expected) << "appendSequenceNumber did not create the expected component";
  ASSERT_EQ(expected[0].toSequenceNumber(), number) << "toSequenceNumber did not return the expected value";
}

TEST_F(TestNameConventions, Timestamp)
{
  const Name& expected("/%FC%00%04%7BE%E3%1B%00%00");
  // 40 years (not counting leap years) in microseconds.
  uint64_t number = (uint64_t)40 * 365 * 24 * 3600 * 1000000;
  ASSERT_EQ(Name().appendTimestamp(number), expected) << "appendTimestamp did not create the expected component";
  ASSERT_EQ(expected[0].toTimestamp(), number) << "toTimestamp did not return the expected value";
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
