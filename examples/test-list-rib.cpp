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
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// This include is produced by:
// protoc --cpp_out=. rib-entry.proto
#include "rib-entry.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

/**
 * DataCallbacks handles the onData event to fetch multiple segments. When the
 * final segment is fetched, pass the result to printRibEntry.
 */
class DataCallbacks
{
public:
  /**
   * Create a new DataCallbacks to use the Face.
   * @param face This calls face.expressInterest to fetch more segments.
   */
  DataCallbacks(Face& face)
  : face_(face),
    enabled_(true)
  {
  }

  void
  onData
    (const ptr_lib::shared_ptr<const Interest>& interest,
     const ptr_lib::shared_ptr<Data>& data)
  {
    if (!endsWithSegmentNumber(data->getName())) {
      // We don't expect a name without a segment number.  Treat it as a bad packet.
      cout << "Got an unexpected packet without a segment number";
      enabled_ = false;
    }
    else {
      uint64_t segmentNumber = data->getName().get(-1).toSegment();

      uint64_t expectedSegmentNumber = contentParts_.size();
      if (segmentNumber != expectedSegmentNumber)
        // Try again to get the expected segment.  This also includes the case
        //   where the first segment is not segment 0.
        face_.expressInterest
          (data->getName().getPrefix(-1).appendSegment(expectedSegmentNumber),
           bind(&DataCallbacks::onData, this, _1, _2),
           bind(&DataCallbacks::onTimeout, this, _1));
      else {
        // Save the content and check if we are finished.
        contentParts_.push_back(data->getContent());

        if (data->getMetaInfo().getFinalBlockId().getValue().size() > 0) {
          uint64_t finalSegmentNumber =
            data->getMetaInfo().getFinalBlockId().toSegment();

          if (segmentNumber == finalSegmentNumber) {
            // We are finished.
            enabled_ = false;

            // Get the total size and concatenate to get encodedContent.
            int totalSize = 0;
            for (int i = 0; i < contentParts_.size(); ++i)
              totalSize += contentParts_[i].size();
            ptr_lib::shared_ptr<vector<uint8_t> > encodedMessage
              (new std::vector<uint8_t>(totalSize));
            int offset = 0;
            for (size_t i = 0; i < contentParts_.size(); ++i) {
              const Blob& content = contentParts_[i];
              // Use an explicit loop to copy since not all platforms have memcpy.
              for (size_t j = 0; j < content.size(); ++j)
                encodedMessage->at(offset + j) = content.buf()[j];
              offset += content.size();
            }

            printRibEntry(Blob(encodedMessage, false));
            return;
          }
        }

        // Fetch the next segment.
        face_.expressInterest
          (data->getName().getPrefix(-1).appendSegment(expectedSegmentNumber + 1),
           bind(&DataCallbacks::onData, this, _1, _2),
           bind(&DataCallbacks::onTimeout, this, _1));
      }
    }
  }

  void
  onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    enabled_ = false;
    cout << "Time out for interest " << interest->getName().toUri() << endl;
  }

  /**
   * Decode the encodedMessage as a TLV RibEntry message and display the values.
   * @param encodedMessage The TLV-encoded RibEntry.
   */
  static void
  printRibEntry(const Blob& encodedMessage)
  {
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

  bool 
  getEnabled() { return enabled_; }
  
private:
  /**
   * Check if the last component in the name is a segment number.
   * @param name The name to check.
   * @return True if the name ends with a segment number, otherwise false.
   */
  static bool
  endsWithSegmentNumber(Name name)
  {
    return name.size() >= 1 &&
           name.get(-1).getValue().size() >= 1 &&
           name.get(-1).getValue().buf()[0] == 0;
  }

  bool enabled_;
  vector<Blob> contentParts_;
  Face& face_;
};

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    // Counter holds data used by the callbacks.
    DataCallbacks callbacks(face);

    // Use bind to pass the counter object to the callbacks.
    Interest interest(Name("/localhost/nfd/rib/list"));
    interest.setChildSelector(1);
    cout << "Express request " << interest.getName().toUri() << endl;
    face.expressInterest
      (interest, bind(&DataCallbacks::onData, &callbacks, _1, _2),
       bind(&DataCallbacks::onTimeout, &callbacks, _1));

    // Loop calling processEvents until callbacks is finished and sets enabled_ false.
    while (callbacks.getEnabled()) {
      face.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
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
