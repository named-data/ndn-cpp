/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/schedule.t.cpp
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

class TestSchedule : public ::testing::Test {
};

TEST_F(TestSchedule, CalculateIntervalWithBlackAndWhite)
{
  Schedule schedule;
  ptr_lib::shared_ptr<RepetitiveInterval> interval1(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 5, 10, 2, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval2(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 6, 8, 1, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval3(new RepetitiveInterval
    (fromIsoString("20150827T000000"),
     fromIsoString("20150827T000000"), 7, 8));
  ptr_lib::shared_ptr<RepetitiveInterval> interval4(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150825T000000"), 4, 7));

  schedule.addWhiteInterval(interval1);
  schedule.addWhiteInterval(interval2);
  schedule.addWhiteInterval(interval4);
  schedule.addBlackInterval(interval3);

  // timePoint1 --> positive 8.25 4-10
  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T063000");
  Schedule::Result result = schedule.getCoveringInterval(timePoint1);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T040000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  // timePoint2 --> positive 8.26 6-8
  MillisecondsSince1970 timePoint2 = fromIsoString("20150826T073000");
  result = schedule.getCoveringInterval(timePoint2);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150826T060000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T080000", toIsoString(result.interval.getEndTime()));

  // timePoint3 --> positive 8.27 5-7
  MillisecondsSince1970 timePoint3 = fromIsoString("20150827T053000");
  result = schedule.getCoveringInterval(timePoint3);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150827T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150827T070000", toIsoString(result.interval.getEndTime()));

  // timePoint4 --> positive 8.27 5-7
  MillisecondsSince1970 timePoint4 = fromIsoString("20150827T063000");
  result = schedule.getCoveringInterval(timePoint4);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150827T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150827T070000", toIsoString(result.interval.getEndTime()));

  // timePoint5 --> negative 8.27 7-8
  MillisecondsSince1970 timePoint5 = fromIsoString("20150827T073000");
  result = schedule.getCoveringInterval(timePoint5);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150827T070000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150827T080000", toIsoString(result.interval.getEndTime()));

  // timePoint6 --> negative 8.25 10-24
  MillisecondsSince1970 timePoint6 = fromIsoString("20150825T113000");
  result = schedule.getCoveringInterval(timePoint6);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T000000", toIsoString(result.interval.getEndTime()));
}

TEST_F(TestSchedule, CalculateIntervalWithoutBlack)
{
  Schedule schedule;
  ptr_lib::shared_ptr<RepetitiveInterval> interval1(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 5, 10, 2, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval2(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 6, 8, 1, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval3(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150825T000000"), 4, 7));

  schedule.addWhiteInterval(interval1);
  schedule.addWhiteInterval(interval2);
  schedule.addWhiteInterval(interval3);

  // timePoint1 --> positive 8.25 4-10
  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T063000");
  Schedule::Result result = schedule.getCoveringInterval(timePoint1);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T040000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  // timePoint2 --> positive 8.26 6-8
  MillisecondsSince1970 timePoint2 = fromIsoString("20150826T073000");
  result = schedule.getCoveringInterval(timePoint2);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150826T060000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T080000", toIsoString(result.interval.getEndTime()));

  // timePoint3 --> positive 8.27 5-10
  MillisecondsSince1970 timePoint3 = fromIsoString("20150827T053000");
  result = schedule.getCoveringInterval(timePoint3);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150827T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150827T100000", toIsoString(result.interval.getEndTime()));

  // timePoint4 --> negative 8.25 10-24
  MillisecondsSince1970 timePoint4 = fromIsoString("20150825T113000");
  result = schedule.getCoveringInterval(timePoint4);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T000000", toIsoString(result.interval.getEndTime()));

  // timePoint5 --> negative 8.25 0-4
  MillisecondsSince1970 timePoint5 = fromIsoString("20150825T013000");
  result = schedule.getCoveringInterval(timePoint5);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150825T000000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T040000", toIsoString(result.interval.getEndTime()));
}

TEST_F(TestSchedule, CalculateIntervalWithoutWhite)
{
  Schedule schedule;
  ptr_lib::shared_ptr<RepetitiveInterval> interval1(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 5, 10, 2, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval2(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150827T000000"), 6, 8, 1, RepetitiveInterval::RepeatUnit::DAY));

  schedule.addBlackInterval(interval1);
  schedule.addBlackInterval(interval2);

  // timePoint1 --> negative 8.25 4-10
  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T063000");
  Schedule::Result result = schedule.getCoveringInterval(timePoint1);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150825T050000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  // timePoint2 --> negative 8.25 0-4
  MillisecondsSince1970 timePoint2 = fromIsoString("20150825T013000");
  result = schedule.getCoveringInterval(timePoint2);
  ASSERT_EQ(false, result.isPositive);
  ASSERT_EQ(false, result.interval.isEmpty());
  ASSERT_EQ("20150825T000000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T000000", toIsoString(result.interval.getEndTime()));
}

static uint8_t SCHEDULE[] = {
  0x8f, 0xc4,// Schedule
  0x8d, 0x90,// WhiteIntervalList
  /////
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x04,
    0x89, 0x01,
      0x07,
    0x8a, 0x01,
      0x00,
    0x8b, 0x01,
      0x00,
  /////
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x38, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x05,
    0x89, 0x01,
      0x0a,
    0x8a, 0x01,
      0x02,
    0x8b, 0x01,
      0x01,
  /////
  0x8c, 0x2e, // RepetitiveInterval
    0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x35, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x38, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x06,
    0x89, 0x01,
      0x08,
    0x8a, 0x01,
      0x01,
    0x8b, 0x01,
      0x01,
  /////
  0x8e, 0x30, // BlackIntervalList
  /////
  0x8c, 0x2e, // RepetitiveInterval
     0x86, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x37, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x87, 0x0f,
      0x32, 0x30, 0x31, 0x35, 0x30, 0x38, 0x32, 0x37, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x88, 0x01,
      0x07,
    0x89, 0x01,
      0x08,
    0x8a, 0x01,
      0x00,
    0x8b, 0x01,
      0x00
};

TEST_F(TestSchedule, EncodeAndDecode)
{
  Schedule schedule;

  ptr_lib::shared_ptr<RepetitiveInterval> interval1(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150828T000000"), 5, 10, 2, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval2(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150828T000000"), 6, 8, 1, RepetitiveInterval::RepeatUnit::DAY));
  ptr_lib::shared_ptr<RepetitiveInterval> interval3(new RepetitiveInterval
    (fromIsoString("20150827T000000"),
     fromIsoString("20150827T000000"), 7, 8));
  ptr_lib::shared_ptr<RepetitiveInterval> interval4(new RepetitiveInterval
    (fromIsoString("20150825T000000"),
     fromIsoString("20150825T000000"), 4, 7));

  schedule.addWhiteInterval(interval1);
  schedule.addWhiteInterval(interval2);
  schedule.addWhiteInterval(interval4);
  schedule.addBlackInterval(interval3);

  Blob encoding = schedule.wireEncode();
  Blob encoding2(SCHEDULE, sizeof(SCHEDULE));
  ASSERT_TRUE(encoding.equals(encoding2));

  Schedule schedule2;
  schedule2.wireDecode(encoding);

  // timePoint1 --> positive 8.25 4-10
  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T063000");
  Schedule::Result result = schedule.getCoveringInterval(timePoint1);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150825T040000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.interval.getEndTime()));

  // timePoint2 --> positive 8.26 6-8
  MillisecondsSince1970 timePoint2 = fromIsoString("20150826T073000");
  result = schedule.getCoveringInterval(timePoint2);
  ASSERT_EQ(true, result.isPositive);
  ASSERT_EQ("20150826T060000", toIsoString(result.interval.getStartTime()));
  ASSERT_EQ("20150826T080000", toIsoString(result.interval.getEndTime()));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
