/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <iostream>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// This include is produced by:
// protoc --cpp_out=. fib-entry.proto
#include "fib-entry.pb.h"

using namespace std;
using namespace ndn;

int main(int argc, char** argv)
{
  try {
    // Construct a sample FibEntry message using the structure in fib-entry.pb.cc 
    // which was produced by protoc.
    ndn_message::FibEntryMessage message; 
    message.mutable_fib_entry()->mutable_name()->add_component("ndn");
    message.mutable_fib_entry()->mutable_name()->add_component("ucla");
    ndn_message::FibEntryMessage_NextHopRecord& nextHopRecord = 
      *message.mutable_fib_entry()->add_next_hop_records();
    nextHopRecord.set_face_id(16);
    nextHopRecord.set_cost(1);

    // Encode the Protobuf message object as TLV.
    Blob encoding = ProtobufTlv::encode(message);
    
    ndn_message::FibEntryMessage decodedMessage; 
    ProtobufTlv::decode(decodedMessage, *encoding);
    
    cout << "Re-decoded FibEntry:" << endl;
    // This should print the same values that we put in message above.
    for (int i = 0; i < decodedMessage.fib_entry().name().component_size(); ++i)
      cout << "/" << decodedMessage.fib_entry().name().component(i);
    cout << " nexthops = {";
    for (int i = 0; i < decodedMessage.fib_entry().next_hop_records_size(); ++i)
      cout << "faceid=" << decodedMessage.fib_entry().next_hop_records(i).face_id()
           << " (cost=" << decodedMessage.fib_entry().next_hop_records(i).cost() << ")";
    cout << " }" << endl;
  } catch (exception& e) {
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
