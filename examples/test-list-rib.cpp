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

/* This sense a rib list request to the local NFD and prints the response.
 * This is equivalent to the NFD command line command "nfd-status -r".
 * See http://redmine.named-data.net/projects/nfd/wiki/Management .
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/util/segment-fetcher.hpp>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// This include is produced by:
// protoc --cpp_out=. rib-entry.proto
#include "rib-entry.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
printRibEntries(const Blob& encodedMessage, bool* enabled);

static void
onError(SegmentFetcher::ErrorCode errorCode, const string& message, bool* enabled);

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    Interest interest(Name("/localhost/nfd/rib/list"));
    interest.setInterestLifetimeMilliseconds(4000);
    cout << "Express request " << interest.getName().toUri() << endl;

    bool enabled = true;
    SegmentFetcher::fetch
      (face, interest, SegmentFetcher::DontVerifySegment,
       bind(&printRibEntries, _1, &enabled),
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
 * encodedMessage as repeated TLV RibEntry messages and display the values.
 * @param encodedMessage The repeated TLV-encoded RibEntry.
 * @param enabled On success or error, set *enabled = false.
 */
static void
printRibEntries(const Blob& encodedMessage, bool* enabled)
{
  *enabled = false;

  ndn_message::RibEntryMessage ribEntryMessage;
  ProtobufTlv::decode(ribEntryMessage, encodedMessage);

  cout << "RIB:" << endl;
  for (int iEntry = 0; iEntry < ribEntryMessage.rib_entry_size(); ++iEntry) {
    const ndn_message::RibEntryMessage_RibEntry& ribEntry = ribEntryMessage.rib_entry(iEntry);

    // Show the name.
    cout << "  ";
    for (int i = 0; i < ribEntry.name().component_size(); ++i)
      cout << "/" << ribEntry.name().component(i);

    // Show the routes.
    for (int iRoute = 0; iRoute < ribEntry.routes_size(); ++iRoute) {
      const ndn_message::RibEntryMessage_Route& route = ribEntry.routes(iRoute);

      cout << " route={faceId=" << route.face_id() << " (origin=" <<
        route.origin() << " cost=" << route.cost();
      if (route.flags() & 1)
        cout << " ChildInherit";
      if (route.flags() & 2)
        cout << " Capture";
      if (route.has_expiration_period())
        cout << " expirationPeriod=" << route.expiration_period();
      cout << ")}" << endl;
    }
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
