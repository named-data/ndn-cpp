/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017 Regents of the University of California.
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

/* This tests the GeneralizedContent which fetches a _meta info object and segmented
 * content with a single segment. This requires a local running NFD.
 */

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_PROTOBUF = 1.
#include <ndn-cpp/ndn-cpp-config.h>
#if NDN_CPP_HAVE_PROTOBUF

#include <cstdlib>
#include <unistd.h>
#include <ndn-cpp-tools/usersync/generalized-content.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;
using namespace ndntools;

static void
onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix, bool* enabled);

static void
publishAndFetch
  (const ptr_lib::shared_ptr<const Name>& registeredPrefix,
   uint64_t registeredPrefixId, Name* prefix, MemoryContentCache* contentCache,
   KeyChain* keyChain, Name* certificateName, Face* consumerFace, bool* enabled);

static void
printMetaInfoAndContent
  (const ptr_lib::shared_ptr<ContentMetaInfo>& metaInfo,
   const Blob& content, bool* enabled);

static void
onError
  (GeneralizedContent::ErrorCode errorCode, const string& message, bool* enabled);

int main(int argc, char** argv)
{
  try {
    KeyChain keyChain;
    Name certificateName = keyChain.getDefaultCertificateName();
    // The default Face will connect using a Unix socket, or to "localhost".
    Face producerFace;
    producerFace.setCommandSigningInfo(keyChain, certificateName);
    MemoryContentCache contentCache(&producerFace);
    bool enabled = true;
    Face consumerFace;

    // On register success, this will call publishAndFetch to continue.
    Name prefix("/ndn/testuser/flume/channel/1/content/10");
    contentCache.registerPrefix
      (prefix, bind(&onRegisterFailed, _1, &enabled), 
       (OnRegisterSuccess)bind
        (&publishAndFetch, _1, _2, &prefix, &contentCache, &keyChain,
         &certificateName, &consumerFace, &enabled));
    
    while (enabled) {
      producerFace.processEvents();
      consumerFace.processEvents();
      // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
      usleep(10000);
    }
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}

/**
 * This is called to print an error if the MemoryContentCache can't register
 * with the local forwarder.
 * @param prefix The prefix Name to register.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix, bool* enabled)
{
  *enabled = false;
  cout << "Failed to register prefix " << prefix->toUri() << endl;
}

/**
 * This is called when MemoryContentCache registers with the local forwarder so
 * that it is ready for us to publish and fetch.
 * @param registeredPrefix The prefix given to registerPrefix.
 * @param registeredPrefixId The value returned by registerPrefix.
 * @param prefix The name prefix for the content.
 * @param contentCache The MemoryContentCache created in main().
 * @param keyChain The KeyChain from main, used to sign the Data packets.
 * @param certificateName The certificate name used to sign the Data packets.
 * @param consumerFace The Face created in main, used to fetch.
 * @param enabled On success or error, set *enabled = false.
 */
static void
publishAndFetch
  (const ptr_lib::shared_ptr<const Name>& registeredPrefix,
   uint64_t registeredPrefixId, Name* prefix, MemoryContentCache* contentCache,
   KeyChain* keyChain, Name* certificateName, Face* consumerFace, bool* enabled)
{
  try {
    // Set up the producer.
    string content = "My content";
    ContentMetaInfo metaInfo;
    string metaInfoText = "Hi there!";
    metaInfo.setContentType("text/html")
      .setTimestamp(1477681379)
      .setHasSegments(true)
      .setOther(Blob((const uint8_t*)&metaInfoText[0], metaInfoText.size()));

    // Set the contentSegmentSize so that we publish two segments for testing.
    size_t contentSegmentSize = content.size() / 2;
    GeneralizedContent::publish
      (*contentCache, *prefix, 40000, keyChain, *certificateName, metaInfo,
       Blob((const uint8_t*)&content[0], content.size()), contentSegmentSize);

    // Fetch the _metaInfo and content.
    GeneralizedContent::fetch
      (*consumerFace, *prefix, 0, bind(&printMetaInfoAndContent, _1, _2, enabled),
       bind(&onError, _1, _2, enabled));
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
}

/**
 * This is called when the _meta info is received and, if necessary, all the
 * segments are received. Print the meta info and content.
 * @param encodedMessage The repeated TLV-encoded ChannelStatus.
 * @param enabled On success or error, set *enabled = false.
 */
static void
printMetaInfoAndContent
  (const ptr_lib::shared_ptr<ContentMetaInfo>& metaInfo,
   const Blob& content, bool* enabled)
{
  *enabled = false;
  cout << "Received _meta info:" << endl;
  cout << "ContentType: " << metaInfo->getContentType() << endl;
  cout << "Timestamp: " << metaInfo->getTimestamp() << endl;
  cout << "HasSegments: " << (metaInfo->getHasSegments() ? "true" : "false") << endl;
  cout << "Other: " <<
    (metaInfo->getOther().isNull() ? string("<none>") : metaInfo->getOther().toRawStr()) <<
    endl;
  if (!content.isNull())
    cout << "Content: " << content.toRawStr() << endl;
}

/**
 * This is called to print an error from GeneralizedContent.
 * @param errorCode The error code.
 * @param message The error message.
 * @param enabled On success or error, set *enabled = false.
 */
static void
onError
  (GeneralizedContent::ErrorCode errorCode, const string& message, bool* enabled)
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
