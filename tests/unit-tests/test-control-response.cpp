/**
 * Copyright (C) 2014-2017 Regents of the University of California.
 * @author Andrew Brown <andrew.brown@intel.com>
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From jNDN TestControlResponse.
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
#include <ndn-cpp/control-response.hpp>

using namespace std;
using namespace ndn;

static uint8_t TestControlResponse1[] = {
  0x65, 0x1c, // ControlResponse
    0x66, 0x02, 0x01, 0x94, // StatusCode
    0x67, 0x11, // StatusText
      0x4e, 0x6f, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x20, 0x6e, 0x6f, 0x74, 0x20,
      0x66, 0x6f, 0x75, 0x6e, 0x64,
    0x68, 0x03, // ControlParameters
      0x69, 0x01, 0x0a // FaceId
};

/**
 * Test encoding/decoding of ControlResponses.
 */
class TestControlResponse : public ::testing::Test {
};

TEST_F(TestControlResponse, Encode)
{
  ControlResponse response;
  response.setStatusCode(404);
  response.setStatusText("Nothing not found");
  ControlParameters controlParameters;
  controlParameters.setFaceId(10);
  response.setBodyAsControlParameters(&controlParameters);
  Blob wire = response.wireEncode();

  ASSERT_TRUE(wire.equals(Blob(TestControlResponse1, sizeof(TestControlResponse1))));
}

TEST_F(TestControlResponse, Decode)
{
  ControlResponse response;
  response.wireDecode(Blob(TestControlResponse1, sizeof(TestControlResponse1)));

  ASSERT_EQ(response.getStatusCode(), 404);
  ASSERT_EQ(response.getStatusText(), "Nothing not found");
  ASSERT_EQ(response.getBodyAsControlParameters()->getFaceId(), 10);
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
