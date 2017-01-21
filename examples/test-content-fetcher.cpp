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

/* This tests the ContentFetcher which fetches a _meta info object and segmented
 * content with a single segment. This uses a TestFace which responds
 * immediately with test Data packets so that it does not need an external
 * forwarder.
 */

#include <cstdlib>
#include "../tools/usersync/content-fetcher.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static void
printMetaInfoAndContent
  (const ptr_lib::shared_ptr<ContentMetaInfo>& metaInfo,
   const Blob& content);

static void
onError(ContentFetcher::ErrorCode errorCode, const string& message);

int main(int argc, char** argv)
{
  try {
  // Prepare a TestFace to instantly answer calls to expressInterest. A real
  // application would use a normal Face object connected to the network.
  class TestFace : public Face {
    public:
      TestFace(const Name& prefix)
      : Face("localhost"), prefix_(prefix)
      {}

      virtual uint64_t
      expressInterest
        (const Interest& interest, const OnData& onData,
         const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
         WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
      {
        if (!prefix_.isPrefixOf(interest.getName())) {
          // Wrong prefix.
          onTimeout(ptr_lib::make_shared<Interest>(interest));
          return 0;
        }

        string content = "My content.";
        if (interest.getName().size() == prefix_.size() + 1 &&
            interest.getName().get(prefix_.size()).toEscapedString() == "_meta") {
          // Make and return the _meta Data packet.
          ContentMetaInfo metaInfo;
          string text = "Hi there!";
          metaInfo.setContentType("text/html")
            .setTimestamp(1477681379)
            .setContentSize(content.size())
            .setOther(Blob((const uint8_t*)&text.front(), text.size()));

          ptr_lib::shared_ptr<Data> data(new Data(interest.getName()));
          data->setContent(metaInfo.wireEncode());

          onData(ptr_lib::make_shared<Interest>(interest), data);
        }
        // When SegmentFetcher expresses the first interest, it doesn't have the segment.
        else if (interest.getName().size() == prefix_.size() ||
            (interest.getName().size() == prefix_.size() + 1 &&
             interest.getName().get(prefix_.size()).toEscapedString() == "%00")) {
          // Make and return the only segment.
          ptr_lib::shared_ptr<Data> data(new Data(interest.getName()));
          if (data->getName().size() == prefix_.size())
            data->getName().appendSegment(0);
          data->setContent(Blob((const uint8_t*)&content.front(), content.size()));
          data->getMetaInfo().setFinalBlockId(Name().appendSegment(0).get(0));

          onData(ptr_lib::make_shared<Interest>(interest), data);
        }
        else
          onTimeout(ptr_lib::make_shared<Interest>(interest));

        return 0;
      }

    private:
      Name prefix_;
    };

    Name prefix("/ndn/testuser/flume/channel/1/content/10");
    TestFace face(prefix);
    ContentFetcher::fetch
      (face, prefix, 0, bind(&printMetaInfoAndContent, _1, _2),
       bind(&onError, _1, _2));
  } catch (std::exception& e) {
    cout << "exception: " << e.what() << endl;
  }
  return 0;
}

/**
 * This is called when the _meta info is received and, if necessary, all the
 * segments are received. Print the meta info and content.
 * @param encodedMessage The repeated TLV-encoded ChannelStatus.
 */
static void
printMetaInfoAndContent
  (const ptr_lib::shared_ptr<ContentMetaInfo>& metaInfo,
   const Blob& content)
{
  cout << "Received _meta info." << endl;
  cout << "ContentType: " << metaInfo->getContentType() << endl;
  cout << "Timestamp: " << metaInfo->getTimestamp() << endl;
  cout << "ContentSize: " << metaInfo->getContentSize() << endl;
  cout << "Other: " <<
    (metaInfo->getOther().isNull() ? string("<none>") : metaInfo->getOther().toRawStr()) <<
    endl;
  if (!content.isNull())
    cout << "Content: " << content.toRawStr() << endl;
}

/**
 * This is called to print an error from ContentFetcher.
 * @param errorCode The error code.
 * @param message The error message.
 */
static void
onError(ContentFetcher::ErrorCode errorCode, const string& message)
{
  cout << message << endl;
}
