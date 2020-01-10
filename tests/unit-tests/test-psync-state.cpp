/**
 * Copyright (C) 2019-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PSync unit tests:
 * https://github.com/named-data/PSync/blob/master/tests/test-state.cpp
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
#include <ndn-cpp/data.hpp>
#include "../../src/sync/detail/psync-state.hpp"

using namespace std;
using namespace ndn;

class TestPSyncState : public ::testing::Test {
};

TEST_F(TestPSyncState, EncodeDecode)
{
  PSyncState state;
  state.addContent(Name("test1"));
  state.addContent(Name("test2"));

  // Simulate getting a buffer of content from a segment fetcher.
  Data data;
  Blob encoding = state.wireEncode();
  uint8_t expectedEncoding[] = {
    0x80, 0x12, // PSyncContent
      0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x31, // Name = "/test1"
      0x07, 0x07, 0x08, 0x05, 0x74, 0x65, 0x73, 0x74, 0x32  // Name = "/test2"
  };
  ASSERT_TRUE(encoding.equals(Blob(expectedEncoding, sizeof(expectedEncoding))));
  data.setContent(encoding);

  PSyncState receivedState;
  receivedState.wireDecode(data.getContent());

  ASSERT_TRUE(state.getContent() == receivedState.getContent());
}

TEST_F(TestPSyncState, EmptyContent)
{
  PSyncState state;

  // Simulate getting a buffer of content from a segment fetcher.
  Data data;
  data.setContent(state.wireEncode());

  PSyncState state2(data.getContent());
  ASSERT_EQ(0, state2.getContent().size());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
