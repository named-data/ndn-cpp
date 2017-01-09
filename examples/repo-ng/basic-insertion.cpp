/**
 * Copyright (C) 2015-2017 Regents of the University of California.
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

/*
 * This shows an example of the repo-ng basic insertion protocol, described here:
 * http://redmine.named-data.net/projects/repo-ng/wiki/Basic_Repo_Insertion_Protocol
 * See main() for more details.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <cstdlib>
#include <iostream>
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// These includes are produced by:
// protoc --cpp_out=. repo-command-parameter.proto
#include "repo-command-parameter.pb.h"
// protoc --cpp_out=. repo-command-response.proto
#include "repo-command-response.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

typedef func_lib::function<void()> SimpleCallback;

static void
requestInsertOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onInsertStarted,
   const SimpleCallback& onFailed);

static void
requestInsertOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed);

/**
 * Send a command interest for the repo to fetch the given fetchName and insert
 * it in the repo.
 * @param face The Face used to call makeCommandInterest and expressInterest.
 * @param repoCommandPrefix The repo command prefix.
 * @param fetchName The name to fetch. If startBlockId and endBlockId are
 * supplied, then the repo will request multiple segments by appending the range
 * of block IDs (segment numbers).
 * @param onInsertStarted When the request insert command successfully returns,
 * this calls onInsertStarted(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 * @param onFailed If the command fails for any reason, this prints an error and
 * calls onFailed(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 * @param startBlockId (optional) The starting block ID (segment number) to fetch.
 * @param endBlockId (optional) The end block ID (segment number) to fetch.
 */
void
requestInsert
  (Face& face, const Name& repoCommandPrefix, const Name& fetchName,
   const SimpleCallback& onInsertStarted, const SimpleCallback& onFailed,
   int startBlockId = -1, int endBlockId = -1)
{
  // Construct a RepoCommandParameterMessage using the structure in
  // repo-command-parameter.pb.cc which was produced by protoc.
  ndn_message::RepoCommandParameterMessage parameter;

  // Add the Name.
  for (size_t i = 0; i < fetchName.size(); ++i)
    parameter.mutable_repo_command_parameter()->mutable_name()->add_component
      (fetchName.get(i).getValue().buf(), fetchName.get(i).getValue().size());
  // Add startBlockId and endBlockId if supplied.
  if (startBlockId >= 0)
    parameter.mutable_repo_command_parameter()->set_start_block_id(startBlockId);
  if (endBlockId >= 0)
    parameter.mutable_repo_command_parameter()->set_end_block_id(endBlockId);

  // Create the command interest.
  Interest interest(Name(repoCommandPrefix).append("insert")
    .append(Name::Component(ProtobufTlv::encode(parameter))));
  face.makeCommandInterest(interest);

  // Send the command interest and get the response or timeout.
  face.expressInterest
    (interest, bind(&requestInsertOnData, _1, _2, onInsertStarted, onFailed),
     bind(&requestInsertOnTimeout, _1, onFailed));
}

static void
requestInsertOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onInsertStarted,
   const SimpleCallback& onFailed)
{
  ndn_message::RepoCommandResponseMessage response;
  try {
    ProtobufTlv::decode(response, data->getContent());
  }
  catch (std::exception& e) {
    cout << "Cannot decode the repo command response " << e.what() << endl;
    onFailed();
  }

  if (response.repo_command_response().status_code() == 100)
    onInsertStarted();
  else {
    cout << "Got repo command error code "  <<
      response.repo_command_response().status_code() << endl;
    onFailed();
  }
}

static void
requestInsertOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed)
{
  cout << "Insert repo command timeout" << endl;
  onFailed();
}

/**
 * This is an example class to supply the data requested by the repo-ng
 * insertion process.  For you application, you would supply data in a different
 * way.  This sends data packets until it has sent (endBlockId - startBlockId) + 1
 * packets.  It might be simpler to finish when onInterest has sent the packet
 * for segment endBlockId, but there is no guarantee that the interests will
 * arrive in order.  Therefore we send packets until the total is sent.
 */
class ProduceSegments {
public:
  ProduceSegments
    (KeyChain& keyChain, const Name& certificateName, int startBlockId,
     int endBlockId, const SimpleCallback& onFinished)
  : keyChain_(keyChain), certificateName_(certificateName),
    startBlockId_(startBlockId), endBlockId_(endBlockId),
    onFinished_(onFinished), nSegmentsSent_(0)
  {}

  // onInterest.
  void
  operator()
     (const ptr_lib::shared_ptr<const Name>& prefix,
      const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
      uint64_t interestFilterId,
      const ptr_lib::shared_ptr<const InterestFilter>& filter);

private:
  KeyChain& keyChain_;
  Name certificateName_;
  int startBlockId_;
  int endBlockId_;
  SimpleCallback onFinished_;
  int nSegmentsSent_;
};

void
ProduceSegments::operator()
   (const ptr_lib::shared_ptr<const Name>& prefix,
    const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
    uint64_t interestFilterId,
    const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  cout << "Got interest " << interest->toUri() << endl;

  // Make and sign a Data packet with the interest name.
  Data data(interest->getName());
  string content(string("Data packet ") + interest->getName().toUri());
  data.setContent((const uint8_t *)&content[0], content.size());
  keyChain_.sign(data, certificateName_);

  face.putData(data);
  cout << "Sent data packet " << data.getName().toUri() << endl;

  nSegmentsSent_ += 1;
  if (nSegmentsSent_ >= (endBlockId_ - startBlockId_) + 1)
    // We sent the final segment.
    onFinished_();
}

static MillisecondsSince1970
getNowMilliseconds()
{
  struct timeval t;
  gettimeofday(&t, 0);
  return t.tv_sec * 1000.0 + t.tv_usec / 1000.0;
}

/**
 * Print the message (if not empty) and set *enabled = false.
 */
static void
printAndQuit(const string& message, bool *enabled);

static void
onInsertStarted(const Name& fetchPrefix);

/**
 * Call requestInsert and register a prefix so that ProduceSegments will answer
 * interests from the repo to send the data packets. This assumes that repo-ng
 * is already running (e.g. `sudo ndn-repo-ng`).
 */
int main(int argc, char** argv)
{
  try {
    Name repoCommandPrefix("/example/repo/1");
    Name repoDataPrefix("/example/data/1");

    MillisecondsSince1970 nowMilliseconds = getNowMilliseconds();
    Name fetchPrefix = Name(repoDataPrefix).append("testinsert").appendVersion
      (nowMilliseconds);

    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;

    // Use the system default key chain and certificate name to sign commands.
    KeyChain keyChain;
    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

    bool enabled = true;
    // Register the prefix and send the repo insert command at the same time.
    int startBlockId = 0;
    int endBlockId = 1;

    ProduceSegments produceSegments
      (keyChain, keyChain.getDefaultCertificateName(), startBlockId, endBlockId,
       bind(&printAndQuit, "All data was inserted.", &enabled));
    cout << "Register prefix " << fetchPrefix.toUri() << endl;
    // TODO: After we remove the registerPrefix with the deprecated OnInterest,
    // we can remove the explicit cast to OnInterestCallback (needed for boost).
    face.registerPrefix
      (fetchPrefix, (const OnInterestCallback&)func_lib::ref(produceSegments),
       bind(&printAndQuit, "Register failed for prefix " + fetchPrefix.toUri(),
            &enabled));

    requestInsert
      (face, repoCommandPrefix, fetchPrefix,
       bind(&onInsertStarted, fetchPrefix),
       // For failure, already printed the error.
       bind(&printAndQuit, "", &enabled), startBlockId, endBlockId);

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

static void
printAndQuit(const string& message, bool *enabled)
{
  if (message != "")
    cout << message << endl;
  *enabled = false;
}

static void
onInsertStarted(const Name& fetchPrefix)
{
  cout << "Insert started for " << fetchPrefix.toUri() << endl;
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
