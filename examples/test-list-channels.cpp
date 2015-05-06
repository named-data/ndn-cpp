/**
 * Copyright (C) 2015 Regents of the University of California.
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

/**
 * This sends a faces channels request to the local NFD and prints the response.
 * This is equivalent to the NFD command line command "nfd-status -c".
 * See http://redmine.named-data.net/projects/nfd/wiki/Management .
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <ndn-cpp/util/segment-fetcher.hpp>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// This include is produced by: protoc --cpp_out=. channel-status.proto
#include "channel-status.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
printChannelStatuses(const Blob& encodedMessage, bool* enabled);

static void
onError(SegmentFetcher::ErrorCode errorCode, const string& message, bool* enabled);

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    Interest interest(Name("/localhost/nfd/faces/channels"));
    interest.setInterestLifetimeMilliseconds(4000);
    cout << "Express request " << interest.getName().toUri() << endl;

    bool enabled = true;
    SegmentFetcher::fetch
      (face, interest, SegmentFetcher::DontVerifySegment,
       bind(&printChannelStatuses, _1, &enabled),
       bind(&onError, _1, _2, &enabled));

    // Loop calling processEvents until a callback sets enabled = false.
    while (enabled) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}

/**
 * This is called when all the segments are received to decode the
 * encodedMessage repeated TLV ChannelStatus messages and display the values.
 * @param encodedMessage The repeated TLV-encoded ChannelStatus.
 * @param enabled On success or error, set *enabled = false.
 */
static void
printChannelStatuses(const Blob& encodedMessage, bool* enabled)
{
  *enabled = false;

  ndn_message::ChannelStatusMessage channelStatusMessage;
  ProtobufTlv::decode(channelStatusMessage, encodedMessage);

  cout << "Channels:" << endl;
  for (size_t iEntry = 0; iEntry < channelStatusMessage.channel_status_size();
       ++iEntry) {
    const ndn_message::ChannelStatusMessage_ChannelStatus channelStatus =
      channelStatusMessage.channel_status(iEntry);

    // Format to look the same as "nfd-status -c".
    cout << "  " + channelStatus.local_uri() << endl;
  }
}

/**
 * This is called to print an error from SegmentFetcher.
 * @param errorCode The error code.
 * @param message The error message.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onError(SegmentFetcher::ErrorCode errorCode, const string& message, bool* enabled)
{
  *enabled = false;
  cout << message << endl;
}

#else // NDN_CPP_HAVE_PROTOBUF

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  cout <<
    "This program uses Protobuf but it is not installed. Install it and ./configure again." << endl;
}

#endif // NDN_CPP_HAVE_PROTOBUF
