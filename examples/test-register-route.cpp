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

/* This sends a /localhost/nfd/faces/query command to the local NFD to get the
 * face ID for a face url, and creates the face if it doesn't exist. Then this sends
 * a /localhost/nfd/rib/register command to register a prefix to the face ID.
 * This is equivalent to the NFD command line command "nfdc register".
 * See http://redmine.named-data.net/projects/nfd/wiki/Management .
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/util/segment-fetcher.hpp>
#include <ndn-cpp/encoding/protobuf-tlv.hpp>
// This include is produced by: protoc --cpp_out=. face-query-filter.proto
#include "face-query-filter.pb.h"
// This include is produced by: protoc --cpp_out=. control-parameters.proto
#include "control-parameters.pb.h"
// This include is produced by: protoc --cpp_out=. face-status.proto
#include "face-status.pb.h"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
processFaceStatus
  (const Blob& encodedFaceStatus, const Name& prefix, string uri, Face* face,
   bool* enabled);

static void
processCreateFaceResponse
  (const ptr_lib::shared_ptr<const Interest>& expressedInterest,
   const ptr_lib::shared_ptr<Data>& responseData, const Name& prefix, Face* face,
   bool* enabled);

static void
registerRoute(const Name& prefix, uint64_t faceId, Face* face, bool* enabled);

static void
processRegisterResponse
  (const ptr_lib::shared_ptr<const Interest>& expressedInterest,
   const ptr_lib::shared_ptr<Data>& responseData, bool* enabled);

static void
onError(SegmentFetcher::ErrorCode errorCode, const string& message, bool* enabled);

static void
onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest, const string& message,
   bool* enabled);

int main(int argc, char** argv)
{
  try {
    Name prefix("/nfd/edu/ucla/remap/test");
    // Route to aleph.ndn.ucla.edu.  Have to use the canonical name with
    // an IP address and port.
    string uri = "udp4://128.97.98.7:6363";

    // The default Face connects to the local NFD.
    Face face;

    // Use the system default key chain and certificate name to sign commands.
    KeyChain keyChain;
    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

    // Create the /localhost/nfd/faces/query command interest, including the
    // FaceQueryFilter. Construct the FaceQueryFilter using the structure in
    // face-query-filter.pb.h which was produced by protoc.
    ndn_message::FaceQueryFilterMessage message;
    ndn_message::FaceQueryFilterMessage_FaceQueryFilter* filter =
      message.add_face_query_filter();
    filter->set_uri(uri);
    Blob encodedFilter = ProtobufTlv::encode(message);

    Interest interest(Name("/localhost/nfd/faces/query"));
    interest.getName().append(encodedFilter);

    bool enabled = true;
    SegmentFetcher::fetch
      (face, interest, 0,
       bind(&processFaceStatus, _1, prefix, uri, &face, &enabled),
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
 * encodedFaceStatus as a TLV FaceStatus message. If the face ID exists for
 * the face URL, use it to call registerRoute(), otherwise send a
 * /localhost/nfd/faces/create command to create the face.
 * @param encodedFaceStatus The TLV-encoded FaceStatus.
 * @param prefix The prefix name to register.
 * @param uri The remote URI in case we need to tell NFD to create a face.
 * @param face The Face which is used to sign the command interest and call
 * expressInterest.
 * @param enabled On success or error, set *enabled = false.
 */
static void
processFaceStatus
  (const Blob& encodedFaceStatus, const Name& prefix, string uri, Face* face,
   bool* enabled)
{
  if (encodedFaceStatus.size() == 0) {
    // No result, so we need to tell NFD to create the face.
    // Encode the ControlParameters.
    ndn_message::ControlParametersTypes_ControlParametersMessage message;
    message.mutable_control_parameters()->set_uri(uri);
    Blob encodedControlParameters = ProtobufTlv::encode(message);

    Interest interest(Name("/localhost/nfd/faces/create"));
    interest.getName().append(encodedControlParameters);
    interest.setInterestLifetimeMilliseconds(10000);

    // Sign and express the interest.
    face->makeCommandInterest(interest);
    face->expressInterest
      (interest,
       bind(&processCreateFaceResponse, _1, _2, prefix, face, enabled),
       bind(&onTimeout, _1, "Face create command timed out.", enabled));
  }
  else {
    ndn_message::FaceStatusMessage decodedFaceStatus;
    ProtobufTlv::decode(decodedFaceStatus, encodedFaceStatus);

    uint64_t faceId = decodedFaceStatus.face_status(0).face_id();

    cout << "Found face ID " << faceId << endl;
    registerRoute(prefix, faceId, face, enabled);
  }
}

/**
 * This is called when the face create command responds to decode the
 * encodedControlResonse as a TLV ControlResponse message containing one
 * ControlParameters. Get the face ID and call registerRoute().
 * @param expressedInterest The interest given to expressInterest.
 * @param responseData The response Data packet whose content is the
 * TLV-encoded ControlResponse.
 * @param prefix The prefix name to register.
 * @param face The Face which is used to sign the command interest and call
 * expressInterest.
 * @param enabled On success or error, set *enabled = false.
 */
static void
processCreateFaceResponse
  (const ptr_lib::shared_ptr<const Interest>& expressedInterest,
   const ptr_lib::shared_ptr<Data>& responseData, const Name& prefix, Face* face,
   bool* enabled)
{
  ndn_message::ControlParametersTypes_ControlParametersResponseMessage
    controlResponseMessage;
  ProtobufTlv::decode(controlResponseMessage, responseData->getContent());
  const ndn_message::ControlParametersTypes_ControlParametersResponse&
    controlResponse = controlResponseMessage.control_response();

  const int lowestErrorCode = 400;
  if (controlResponse.status_code() >= lowestErrorCode) {
    cout << "Face create command got error, code " <<
      controlResponse.status_code() << ": " + controlResponse.status_text() <<
      endl;
    *enabled = false;
    return;
  }
  if (controlResponse.control_parameters_size() != 1) {
    cout << "Face create command response does not have one ControlParameters" <<
      endl;
    *enabled = false;
    return;
  }

  uint64_t faceId = controlResponse.control_parameters(0).face_id();

  cout << "Created face ID " << faceId << endl;
  registerRoute(prefix, faceId, face, enabled);
}

/**
 * Use /localhost/nfd/rib/register to register the prefix to the faceId.
 * @param prefix The prefix name to register.
 * @param faceId The face ID.
 * @param face The Face which is used to sign the command interest and call
 * expressInterest.
 * @param enabled On success or error, set enabled[0] = false;
 */
static void
registerRoute(const Name& prefix, uint64_t faceId, Face* face, bool* enabled)
{
  // Use default values;
  uint64_t origin = 255;
  uint64_t cost = 0;
  const uint64_t CHILD_INHERIT = 1;
  uint64_t flags = CHILD_INHERIT;

  ndn_message::ControlParametersTypes_ControlParametersMessage parameters;
  for (size_t i = 0; i < prefix.size(); ++i)
    parameters.mutable_control_parameters()->mutable_name()->add_component
      (prefix.get(i).getValue().buf(), prefix.get(i).getValue().size());
  parameters.mutable_control_parameters()->set_face_id(faceId);
  parameters.mutable_control_parameters()->set_origin(origin);
  parameters.mutable_control_parameters()->set_cost(cost);
  parameters.mutable_control_parameters()->set_flags(flags);
  Blob encodedControlParameters = ProtobufTlv::encode(parameters);

  Interest interest(Name("/localhost/nfd/rib/register"));
  interest.getName().append(encodedControlParameters);
  interest.setInterestLifetimeMilliseconds(10000);

  // Sign and express the interest.
  face->makeCommandInterest(interest);
  face->expressInterest
    (interest,
     bind(&processRegisterResponse, _1, _2, enabled),
     bind(&onTimeout, _1, "Register route command timed out.", enabled));
}

/**
 * This is called when the register route command responds to decode the
 * encodedControlResponse as a TLV ControlParametersResponse message
 * containing one ControlParameters. On success, print the ControlParameters
 * values which should be the same as requested.
 * @param expressedInterest The interest given to expressInterest.
 * @param responseData The response Data packet whose content is the TLV-encoded
 * ControlParametersResponse.
 * @param enabled On success or error, set *enabled = false.
 */
static void
processRegisterResponse
  (const ptr_lib::shared_ptr<const Interest>& expressedInterest,
   const ptr_lib::shared_ptr<Data>& responseData, bool* enabled)
{
  // We are finished in all cases.
  *enabled = false;

  ndn_message::ControlParametersTypes_ControlParametersResponseMessage
    decodedControlResponse;
  ProtobufTlv::decode(decodedControlResponse, responseData->getContent());
  const ndn_message::ControlParametersTypes_ControlParametersResponse&
    controlResponse = decodedControlResponse.control_response();

  const uint64_t lowestErrorCode = 400;
  if (controlResponse.status_code() >= lowestErrorCode) {
    cout << "Face create command got error, code " <<
      controlResponse.status_code() <<": " + controlResponse.status_text();
    return;
  }
  if (controlResponse.control_parameters_size() != 1) {
    cout << "Face create command response does not have one ControlParameters";
    return;
  }

  // Success. Print the ControlParameters response.
  const ndn_message::ControlParametersTypes_ControlParameters& controlParameters =
    controlResponse.control_parameters(0);
  cout << "Successful in name registration: ControlParameters(Name: " <<
     ProtobufTlv::toName(controlParameters.name()).toUri() <<
     ", FaceId: " << controlParameters.face_id() <<
     ", Origin: " << controlParameters.origin() <<
     ", Cost: " << controlParameters.cost() <<
     ", Flags: " << controlParameters.flags() << ")" << endl;
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

/**
 * This is called when expressInterest times out.
 * @param errorCode The error code.
 * @param interest The interest that timed out.
 * @param message The message to print on timeout.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onTimeout
  (const ptr_lib::shared_ptr<const Interest>& interest, const string& message,
   bool* enabled)
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
