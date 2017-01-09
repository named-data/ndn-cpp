/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/repetitive-interval.t.cpp
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
#include <ndn-cpp/encrypt/repetitive-interval.hpp>
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

class TestRepetitiveInterval : public ::testing::Test {
};

TEST_F(TestRepetitiveInterval, Construction)
{
  RepetitiveInterval repetitiveInterval1
    (fromIsoString("20150825T000000"), fromIsoString("20150825T000000"), 5, 10);
  ASSERT_EQ("20150825T000000", toIsoString(repetitiveInterval1.getStartDate()));
  ASSERT_EQ("20150825T000000", toIsoString(repetitiveInterval1.getEndDate()));
  ASSERT_EQ(5, repetitiveInterval1.getIntervalStartHour());
  ASSERT_EQ(10, repetitiveInterval1.getIntervalEndHour());

  RepetitiveInterval repetitiveInterval2
    (fromIsoString("20150825T000000"), fromIsoString("20150827T000000"), 5, 10, 1,
     RepetitiveInterval::RepeatUnit::DAY);

  ASSERT_EQ(1, repetitiveInterval2.getNRepeats());
  ASSERT_EQ
    (RepetitiveInterval::RepeatUnit::DAY, repetitiveInterval2.getRepeatUnit());

  RepetitiveInterval repetitiveInterval3
    (fromIsoString("20150825T000000"), fromIsoString("20151227T000000"), 5, 10, 2,
     RepetitiveInterval::RepeatUnit::MONTH);

  ASSERT_EQ(2, repetitiveInterval3.getNRepeats());
  ASSERT_EQ
    (RepetitiveInterval::RepeatUnit::MONTH, repetitiveInterval3.getRepeatUnit());

  RepetitiveInterval repetitiveInterval4
    (fromIsoString("20150825T000000"), fromIsoString("20301227T000000"), 5, 10, 5,
     RepetitiveInterval::RepeatUnit::YEAR);

  ASSERT_EQ(5, repetitiveInterval4.getNRepeats());
  ASSERT_EQ
    (RepetitiveInterval::RepeatUnit::YEAR, repetitiveInterval4.getRepeatUnit());

  RepetitiveInterval repetitiveInterval5;

  ASSERT_EQ(0, repetitiveInterval5.getNRepeats());
  ASSERT_EQ
    (RepetitiveInterval::RepeatUnit::NONE, repetitiveInterval5.getRepeatUnit());
}

TEST_F(TestRepetitiveInterval, CoverTimePoint)
{
  ///////////////////////////////////////////// With the repeat unit DAY.

  RepetitiveInterval repetitiveInterval1
    (fromIsoString("20150825T000000"), fromIsoString("20150925T000000"), 5, 10, 2,
     RepetitiveInterval::RepeatUnit::DAY);

  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T050000");

  RepetitiveInterval::Result result = repetitiveInterval1.getInterval(timePoint1);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  MillisecondsSince1970 timePoint2 = fromIsoString("20150902T060000");

  result = repetitiveInterval1.getInterval(timePoint2);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150902T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150902T100000", toIsoString(result.interval.getEndTime()));

  MillisecondsSince1970 timePoint3 = fromIsoString("20150929T040000");

  result = repetitiveInterval1.getInterval(timePoint3);
  ASSERT_EQ(false, result.isPositive);

  ///////////////////////////////////////////// With the repeat unit MONTH.

  RepetitiveInterval repetitiveInterval2
    (fromIsoString("20150825T000000"), fromIsoString("20160825T000000"), 5, 10, 2,
     RepetitiveInterval::RepeatUnit::MONTH);

  double timePoint4 = fromIsoString("20150825T050000");

  result = repetitiveInterval2.getInterval(timePoint4);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  double timePoint5 = fromIsoString("20151025T060000");

  result = repetitiveInterval2.getInterval(timePoint5);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20151025T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20151025T100000", toIsoString(result.interval.getEndTime()));

  double timePoint6 = fromIsoString("20151226T050000");

  result = repetitiveInterval2.getInterval(timePoint6);
  ASSERT_EQ(false, result.isPositive);

  double timePoint7 = fromIsoString("20151225T040000");

  result = repetitiveInterval2.getInterval(timePoint7);
  ASSERT_EQ(false, result.isPositive);

  ///////////////////////////////////////////// With the repeat unit YEAR.

  RepetitiveInterval repetitiveInterval3
    (fromIsoString("20150825T000000"), fromIsoString("20300825T000000"), 5, 10, 3,
     RepetitiveInterval::RepeatUnit::YEAR);

  double timePoint8 = fromIsoString("20150825T050000");

  result = repetitiveInterval3.getInterval(timePoint8);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  double timePoint9 = fromIsoString("20180825T060000");

  result = repetitiveInterval3.getInterval(timePoint9);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20180825T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20180825T100000", toIsoString(result.interval.getEndTime()));

  double timePoint10 = fromIsoString("20180826T050000");
  result = repetitiveInterval3.getInterval(timePoint10);
  ASSERT_EQ(false, result.isPositive);

  double timePoint11 = fromIsoString("20210825T040000");
  result = repetitiveInterval3.getInterval(timePoint11);
  ASSERT_EQ(false, result.isPositive);

  double timePoint12 = fromIsoString("20300825T040000");
  result = repetitiveInterval3.getInterval(timePoint12);
  ASSERT_EQ(false, result.isPositive);
}

#define check(small, big) \
  ASSERT_TRUE(small.compare(big) < 0); \
  ASSERT_TRUE(!(big.compare(small) < 0));

TEST_F(TestRepetitiveInterval, Comparison)
{
  check(RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY),
        RepetitiveInterval(fromIsoString("20150826T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY));

  check(RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY),
        RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           6, 10, 2, RepetitiveInterval::RepeatUnit::DAY));

  check(RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY),
        RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 11, 2, RepetitiveInterval::RepeatUnit::DAY));

  check(RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY),
        RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 3, RepetitiveInterval::RepeatUnit::DAY));

  check(RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::DAY),
        RepetitiveInterval(fromIsoString("20150825T000000"),
                           fromIsoString("20150828T000000"),
                           5, 10, 2, RepetitiveInterval::RepeatUnit::MONTH));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
