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

class DataCallbacks
{
public:
  DataCallbacks() {
    callbackCount_ = 0;
  }

  void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data)
  {
    ++callbackCount_;

    // Decode the RibEntry and print the values.
    ndn_message::RibEntryMessage ribEntryMessage;
    ProtobufTlv::decode(ribEntryMessage, data->getContent());

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

  void onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    ++callbackCount_;
    cout << "Time out for interest " << interest->getName().toUri() << endl;
  }

  int callbackCount_;
};

int main(int argc, char** argv)
{
  try {
    // The default Face connects to the local NFD.
    Face face;

    // Counter holds data used by the callbacks.
    DataCallbacks callbacks;

    // Use bind to pass the counter object to the callbacks.
    Interest interest(Name("/localhost/nfd/rib/list"));
    interest.setChildSelector(1);
    cout << "Express request " << interest.getName().toUri() << endl;
    face.expressInterest(interest, bind(&DataCallbacks::onData, &callbacks, _1, _2), bind(&DataCallbacks::onTimeout, &callbacks, _1));

    // The main event loop.
    while (callbacks.callbackCount_ < 1) {
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
