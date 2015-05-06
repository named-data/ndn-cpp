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
 * This sends a faces list request to the local NFD and prints the response.
 * This is equivalent to the NFD command line command "nfd-status -f".
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
// This include is produced by: protoc --cpp_out=. face-status.proto
#include "face-status.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
printFaceStatuses(const Blob& encodedMessage, bool* enabled);

static void
onError(SegmentFetcher::ErrorCode errorCode, const string& message, bool* enabled);

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    Interest interest(Name("/localhost/nfd/faces/list"));
    interest.setInterestLifetimeMilliseconds(4000);
    cout << "Express request " << interest.getName().toUri() << endl;

    bool enabled = true;
    SegmentFetcher::fetch
      (face, interest, SegmentFetcher::DontVerifySegment,
       bind(&printFaceStatuses, _1, &enabled),
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
 * encodedMessage repeated TLV FaceStatus messages and display the values.
 * @param encodedMessage The repeated TLV-encoded FaceStatus.
 * @param enabled On success or error, set *enabled = false.
 */
static void
printFaceStatuses(const Blob& encodedMessage, bool* enabled)
{
  *enabled = false;

  ndn_message::FaceStatusMessage faceStatusMessage;
  ProtobufTlv::decode(faceStatusMessage, encodedMessage);

  cout << "Faces:" << endl;
  for (size_t iEntry = 0; iEntry < faceStatusMessage.face_status_size(); ++iEntry) {
    const ndn_message::FaceStatusMessage_FaceStatus faceStatus =
      faceStatusMessage.face_status(iEntry);

    // Format to look the same as "nfd-status -f".
    cout << "  faceid=" << faceStatus.face_id() <<
      " remote=" << faceStatus.uri() <<
      " local=" << faceStatus.local_uri();
    if (faceStatus.has_expiration_period())
      // Convert milliseconds to seconds.
      cout << " expires=" <<
        ::round((double)faceStatus.expiration_period() / 1000) << "s";
    cout << " counters={" << "in={" << faceStatus.n_in_interests() <<
      "i " << faceStatus.n_in_datas() << "d " << faceStatus.n_in_bytes() << "B}" <<
      " out={" << faceStatus.n_out_interests() << "i "<< faceStatus.n_out_datas() <<
      "d " << faceStatus.n_out_bytes() << "B}" << "}" <<
      " " << (faceStatus.face_scope() == 1 ? "local" : "non-local") <<
      " " << (faceStatus.face_persistency() == 2 ? "permanent" :
             faceStatus.face_persistency() == 1 ? "on-demand" : "persistent") <<
      " " << (faceStatus.link_type() == 1 ? "multi-access" : "point-to-point") <<
      endl;
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
