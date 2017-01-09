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
 * This shows an example of the repo-ng watched prefix insertion protocol,
 * described here:
 * http://redmine.named-data.net/projects/repo-ng/wiki/Watched_Prefix_Insertion_Protocol
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
#include <sstream>
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
startRepoWatchOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onRepoWatchStarted,
   const SimpleCallback& onFailed);

static void
startRepoWatchOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed);

/**
 * Send a command interest for the repo to start watching the given watchPrefix.
 * @param face The Face used to call makeCommandInterest and expressInterest.
 * @param repoCommandPrefix The repo command prefix.
 * @param watchPrefix The prefix that the repo will watch.
 * @param onRepoWatchStarted When the start watch command successfully returns,
 * this calls onRepoWatchStarted(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 * @param onFailed If the command fails for any reason, this prints an error and
 * calls onFailed(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 */
void
startRepoWatch
  (Face& face, const Name& repoCommandPrefix, const Name& watchPrefix,
   const SimpleCallback& onRepoWatchStarted, const SimpleCallback& onFailed)
{
  // Construct a RepoCommandParameterMessage using the structure in
  // repo-command-parameter.pb.cc which was produced by protoc.
  ndn_message::RepoCommandParameterMessage parameter;

  // Add the Name.
  for (size_t i = 0; i < watchPrefix.size(); ++i)
    parameter.mutable_repo_command_parameter()->mutable_name()->add_component
      (watchPrefix.get(i).getValue().buf(), watchPrefix.get(i).getValue().size());

  // Create the command interest.
  Interest interest(Name(repoCommandPrefix).append("watch").append("start")
    .append(Name::Component(ProtobufTlv::encode(parameter))));
  face.makeCommandInterest(interest);

  // Send the command interest and get the response or timeout.
  face.expressInterest
    (interest, bind(&startRepoWatchOnData, _1, _2, onRepoWatchStarted, onFailed),
     bind(&startRepoWatchOnTimeout, _1, onFailed));
}

static void
startRepoWatchOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onRepoWatchStarted,
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
    onRepoWatchStarted();
  else {
    cout << "Got repo command error code "  <<
      response.repo_command_response().status_code() << endl;
    onFailed();
  }
}

static void
startRepoWatchOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed)
{
  cout << "Start repo watch command timeout" << endl;
  onFailed();
}

static void
stopRepoWatchOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onRepoWatchStopped,
   const SimpleCallback& onFailed);

static void
stopRepoWatchOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed);

/**
 * Send a command interest for the repo to stop watching the given watchPrefix.
 * @param face The Face used to call makeCommandInterest and expressInterest.
 * @param repoCommandPrefix The repo command prefix.
 * @param watchPrefix The prefix that the repo will stop watching.
 * @param onRepoWatchStopped When the stop watch command successfully returns,
 * this calls onRepoWatchStopped(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 * @param onFailed If the command fails for any reason, this prints an error and
 * calls onFailed(). This copies the function object, so you may
 * need to use func_lib::ref() as appropriate.
 */
void
stopRepoWatch
  (Face& face, const Name& repoCommandPrefix, const Name& watchPrefix,
   const SimpleCallback& onRepoWatchStopped, const SimpleCallback& onFailed)
{
  // Construct a RepoCommandParameterMessage using the structure in
  // repo-command-parameter.pb.cc which was produced by protoc.
  ndn_message::RepoCommandParameterMessage parameter;

  // Add the Name.
  for (size_t i = 0; i < watchPrefix.size(); ++i)
    parameter.mutable_repo_command_parameter()->mutable_name()->add_component
      (watchPrefix.get(i).getValue().buf(), watchPrefix.get(i).getValue().size());

  // Create the command interest.
  Interest interest(Name(repoCommandPrefix).append("watch").append("stop")
    .append(Name::Component(ProtobufTlv::encode(parameter))));
  face.makeCommandInterest(interest);

  // Send the command interest and get the response or timeout.
  face.expressInterest
    (interest, bind(&stopRepoWatchOnData, _1, _2, onRepoWatchStopped, onFailed),
     bind(&stopRepoWatchOnTimeout, _1, onFailed));
}

static void
stopRepoWatchOnData
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const ptr_lib::shared_ptr<Data>& data, const SimpleCallback& onRepoWatchStopped,
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

  if (response.repo_command_response().status_code() == 101)
    onRepoWatchStopped();
  else {
    cout << "Got repo command error code "  <<
      response.repo_command_response().status_code() << endl;
    onFailed();
  }
}

static void
stopRepoWatchOnTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest,
   const SimpleCallback& onFailed)
{
  cout << "Stop repo watch command timeout" << endl;
  onFailed();
}

/**
 * This is an example class to supply the data requested by the repo-ng
 * watched prefix process. For you application, you would supply data in a
 * different way. Repo-ng sends interests for the watchPrefix given to
 * startRepoWatch(). (The interest also has Exclude selectors but for
 * simplicity we ignore them and assume that the exclude values increase along
 * with the segment numbers that we send.) This sends data packets where the
 * name has the prefix plus increasing segment numbers up to a maximum.
 */
class SendSegments {
public:
  SendSegments
    (KeyChain& keyChain, const Name& certificateName,
     const SimpleCallback& onFinished)
  : keyChain_(keyChain), certificateName_(certificateName),
    onFinished_(onFinished), segment_(-1)
  {}

  /**
   * This is the onInterest callback to append the next segment number to the
   * prefix and send a new data packet. If the last packet is sent, then call
   * onFinished_().
   */
  void
  operator()
     (const ptr_lib::shared_ptr<const Name>& prefix,
      const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
      uint64_t interestFilterId,
      const ptr_lib::shared_ptr<const InterestFilter>& filter);

private:
  KeyChain& keyChain_;
  Name certificateName_;
  SimpleCallback onFinished_;
  int segment_;
};

void
SendSegments::operator()
   (const ptr_lib::shared_ptr<const Name>& prefix,
    const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
    uint64_t interestFilterId,
    const ptr_lib::shared_ptr<const InterestFilter>& filter)
{
  int maxSegment = 2;
  if (segment_ >= maxSegment)
    // We have already called onFinished_().
    return;

  cout << "Got interest " << interest->toUri() << endl;

  // Make and sign a Data packet with the interest name.
  segment_ += 1;
  Data data(Name(*prefix).appendSegment(segment_));
  ostringstream content;
  content << "Segment number " << segment_;
  data.setContent((const uint8_t *)&content.str()[0], content.str().size());
  keyChain_.sign(data, certificateName_);

  face.putData(data);
  cout << "Sent data packet " << data.getName().toUri() << endl;

  if (segment_ >= maxSegment)
    // We sent the final data packet.
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
onRepoWatchStarted(const Name& watchPrefix);

static void
stopRepoWatchAndQuit
  (Face* face, const Name& repoCommandPrefix, const Name& watchPrefix,
   bool *enabled);

/**
 * Call startRepoWatch and register a prefix so that SendSegments will answer
 * interests from the repo to send data packets for the watched prefix.  When
 * all the data is sent (or an error), call stopRepoWatch. This assumes that
 * repo-ng is already running (e.g. `sudo ndn-repo-ng`).
 */
int main(int argc, char** argv)
{
  try {
    Name repoCommandPrefix("/example/repo/1");
    Name repoDataPrefix("/example/data/1");

    MillisecondsSince1970 nowMilliseconds = getNowMilliseconds();
    Name watchPrefix = Name(repoDataPrefix).append("testwatch").appendVersion
      (nowMilliseconds);

    // The default Face will connect using a Unix socket, or to "localhost".
    Face face;

    // Use the system default key chain and certificate name to sign commands.
    KeyChain keyChain;
    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

    bool enabled = true;
    // Register the prefix and start the repo watch at the same time.
    SendSegments sendSegments
      (keyChain, keyChain.getDefaultCertificateName(),
       bind(&stopRepoWatchAndQuit, &face, repoCommandPrefix, watchPrefix, &enabled));
    cout << "Register prefix " << watchPrefix.toUri() << endl;
    // TODO: After we remove the registerPrefix with the deprecated OnInterest,
    // we can remove the explicit cast to OnInterestCallback (needed for boost).
    face.registerPrefix
      (watchPrefix, (const OnInterestCallback&)func_lib::ref(sendSegments),
       bind(&printAndQuit, "Register failed for prefix " + watchPrefix.toUri(),
            &enabled));

    startRepoWatch
      (face, repoCommandPrefix, watchPrefix,
       bind(&onRepoWatchStarted, watchPrefix),
       bind(&stopRepoWatchAndQuit, &face, repoCommandPrefix, watchPrefix, &enabled));

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
onRepoWatchStarted(const Name& watchPrefix)
{
  cout << "Watch started for " << watchPrefix.toUri() << endl;
}

static void
stopRepoWatchAndQuit
  (Face* face, const Name& repoCommandPrefix, const Name& watchPrefix,
   bool *enabled)
{
  stopRepoWatch
    (*face, repoCommandPrefix, watchPrefix,
     bind(&printAndQuit, "Watch stopped for " + watchPrefix.toUri(), enabled),
     bind(&printAndQuit, "stopRepoWatch failed.", enabled));
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
