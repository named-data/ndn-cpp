/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/interval.t.cpp
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
#include <ndn-cpp/encrypt/interval.hpp>
// Use Schedule::fromIsoString.
#include <ndn-cpp/encrypt/schedule.hpp>

using namespace std;
using namespace ndn;

static MillisecondsSince1970
fromIsoString(const string& dateString)
{ 
  return Schedule::fromIsoString(dateString);
}

string
toIsoString(MillisecondsSince1970 msSince1970)
{
  return Schedule::toIsoString(msSince1970);
}

class TestInterval : public ::testing::Test {
};

TEST_F(TestInterval, Construction)
{
  // Construct with the right parameters.
  Interval interval1(fromIsoString("20150825T120000"),
                     fromIsoString("20150825T160000"));
  ASSERT_EQ("20150825T120000", toIsoString(interval1.getStartTime()));
  ASSERT_EQ("20150825T160000", toIsoString(interval1.getEndTime()));
  ASSERT_EQ(true, interval1.isValid());

  // Construct with the invalid interval.
  Interval interval2;
  ASSERT_EQ(false, interval2.isValid());

  // Construct with the empty interval.
  Interval interval3(true);
  ASSERT_EQ(true, interval3.isValid());
  ASSERT_EQ(true, interval3.isEmpty());
}

TEST_F(TestInterval, CoverTimePoint)
{
  Interval interval(fromIsoString("20150825T120000"),
                    fromIsoString("20150825T160000"));

  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T120000");
  MillisecondsSince1970 timePoint2 = fromIsoString("20150825T130000");
  MillisecondsSince1970 timePoint3 = fromIsoString("20150825T170000");
  MillisecondsSince1970 timePoint4 = fromIsoString("20150825T110000");

  ASSERT_EQ(true, interval.covers(timePoint1));
  ASSERT_EQ(true, interval.covers(timePoint2));
  ASSERT_EQ(false, interval.covers(timePoint3));
  ASSERT_EQ(false, interval.covers(timePoint4));
}

TEST_F(TestInterval, IntersectionAndUnion)
{
  Interval interval1(fromIsoString("20150825T030000"),
                     fromIsoString("20150825T050000"));
  // No intersection.
  Interval interval2(fromIsoString("20150825T050000"),
                     fromIsoString("20150825T070000"));
  // No intersection.
  Interval interval3(fromIsoString("20150825T060000"),
                     fromIsoString("20150825T070000"));
  // There's an intersection.
  Interval interval4(fromIsoString("20150825T010000"),
                     fromIsoString("20150825T040000"));
  // Right in interval1, there's an intersection.
  Interval interval5(fromIsoString("20150825T030000"),
                     fromIsoString("20150825T040000"));
  // Wrap interval1, there's an intersection.
  Interval interval6(fromIsoString("20150825T010000"),
                     fromIsoString("20150825T050000"));
  // Empty interval.
  Interval interval7(true);

  Interval tempInterval(interval1);
  tempInterval.intersectWith(interval2);
  ASSERT_EQ(true, tempInterval.isEmpty());

  tempInterval = Interval(interval1);
  ASSERT_THROW(tempInterval.unionWith(interval2), Interval::Error)
    << "Expected error in unionWith(interval2)";

  tempInterval = Interval(interval1);
  tempInterval.intersectWith(interval3);
  ASSERT_EQ(true, tempInterval.isEmpty());

  tempInterval = Interval(interval1);
  ASSERT_THROW(tempInterval.unionWith(interval3), Interval::Error)
    << "Expected error in unionWith(interval3)";

  tempInterval = Interval(interval1);
  tempInterval.intersectWith(interval4);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T030000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T040000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.unionWith(interval4);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T010000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T050000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.intersectWith(interval5);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T030000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T040000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.unionWith(interval5);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T030000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T050000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.intersectWith(interval6);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T030000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T050000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.unionWith(interval6);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T010000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T050000", toIsoString(tempInterval.getEndTime()));

  tempInterval = Interval(interval1);
  tempInterval.intersectWith(interval7);
  ASSERT_EQ(true, tempInterval.isEmpty());

  tempInterval = Interval(interval1);
  tempInterval.unionWith(interval7);
  ASSERT_EQ(false, tempInterval.isEmpty());
  ASSERT_EQ("20150825T030000", toIsoString(tempInterval.getStartTime()));
  ASSERT_EQ("20150825T050000", toIsoString(tempInterval.getEndTime()));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
