/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
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

#include "gtest/gtest.h"
#include <ndn-cpp/face.hpp>
#include <sstream>
#if NDN_CPP_HAVE_TIME_H
#include <time.h>
#endif
#if NDN_CPP_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <ndn-cpp/security/identity/memory-identity-storage.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>
#include <ndn-cpp/security/policy/self-verify-policy-manager.hpp>
#include <ndn-cpp/security/key-chain.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static MillisecondsSince1970
getNowMilliseconds()
{
  struct timeval t;
  // Note: configure.ac requires gettimeofday.
  gettimeofday(&t, 0);
  return t.tv_sec * 1000.0 + t.tv_usec / 1000.0;
}

class CallbackCounter
{
public:
  CallbackCounter()
  {
    onDataCallCount_ = 0;
    onTimeoutCallCount_ = 0;
  }

  void
  onData(const ptr_lib::shared_ptr<const Interest>& interest,
         const ptr_lib::shared_ptr<Data>& data)
  {
    interest_ = *interest;
    data_ = *data;
    ++onDataCallCount_;
  }

  void
  onTimeout(const ptr_lib::shared_ptr<const Interest>& interest)
  {
    interest_ = *interest;
    ++onTimeoutCallCount_;
  }

  int onDataCallCount_;
  int onTimeoutCallCount_;
  Interest interest_;
  Data data_;
};

class RegisterCounter
{
public:
  RegisterCounter(KeyChain& keyChain)
  : keyChain_(keyChain)
  {
    onInterestCallCount_ = 0;
    onRegisterFailedCallCount_ = 0;
  }

  void
  onInterest
    (const ptr_lib::shared_ptr<const Name>& prefix,
     const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport,
     uint64_t interestFilterId)
  {
    ++onInterestCallCount_;

    Data data(interest->getName());
    string content("SUCCESS");
    data.setContent((const uint8_t *)&content[0], content.size());
    keyChain_.sign(data, keyChain_.getDefaultCertificateName());
    Blob encodedData = data.wireEncode();
    transport.send(*encodedData);
  }

  void
  onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
  {
    ++onRegisterFailedCallCount_;
  }

  KeyChain& keyChain_;
  int onInterestCallCount_;
  int onRegisterFailedCallCount_;
};

// Returns a CallbackCounter object so we can test data callback and timeout behavior.
CallbackCounter
runExpressNameTest(Face& face, const string& interestName, Milliseconds timeout = 10000)
{
  Name name(interestName);
  CallbackCounter counter;
  face.expressInterest
    (name, bind(&CallbackCounter::onData, &counter, _1, _2),
     bind(&CallbackCounter::onTimeout, &counter, _1));

  MillisecondsSince1970 startTime = getNowMilliseconds();
  while (getNowMilliseconds() - startTime < timeout &&
         counter.onDataCallCount_ == 0 && counter.onTimeoutCallCount_ == 0) {
    face.processEvents();
    // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
    usleep(10000);
  }

  return counter;
}

class TestFaceInterestMethods : public ::testing::Test {
public:
  TestFaceInterestMethods()
  : face("localhost")
  {
  }

  virtual void
  TearDown()
  {
    face.shutdown();
  }

  Face face;
};

TEST_F(TestFaceInterestMethods, AnyInterest)
{
  string uri = "/";
  CallbackCounter counter = runExpressNameTest(face, uri);
  ASSERT_TRUE(counter.onTimeoutCallCount_ == 0) << "Timeout on expressed interest";

  // check that the callback was correct
  ASSERT_EQ(counter.onDataCallCount_, 1) << "Expected 1 onData callback, got " << counter.onDataCallCount_;

  // just check that the interest was returned correctly?
  const Interest& callbackInterest = counter.interest_;
  ASSERT_TRUE(callbackInterest.getName().equals(Name(uri))) << "Interest returned on callback had different name";
}

/*
TODO: Replace this with a test that connects to a Face on localhost
def test_specific_interest(self):
  uri = "/ndn/edu/ucla/remap/ndn-js-test/howdy.txt/%FD%052%A1%DF%5E%A4"
  (dataCallback, timeoutCallback) = self.run_express_name_test(uri)
  self.assertTrue(timeoutCallback.call_count == 0, 'Unexpected timeout on expressed interest')

  // check that the callback was correct
  self.assertEqual(dataCallback.call_count, 1, 'Expected 1 onData callback, got '+str(dataCallback.call_count))

  onDataArgs = dataCallback.call_args[0] # the args are returned as ([ordered arguments], [keyword arguments])

  // just check that the interest was returned correctly?
  callbackInterest = onDataArgs[0]
  self.assertTrue(callbackInterest.getName().equals(Name(uri)), 'Interest returned on callback had different name')
*/

TEST_F(TestFaceInterestMethods, Timeout)
{
  string uri = "/test/timeout";
  CallbackCounter counter = runExpressNameTest(face, uri);

  // we're expecting a timeout callback, and only 1
  ASSERT_EQ(counter.onDataCallCount_, 0) << "Data callback called for invalid interest";

  ASSERT_TRUE(counter.onTimeoutCallCount_ == 1) << "Expected 1 timeout call, got " << counter.onTimeoutCallCount_;

  // just check that the interest was returned correctly?
  const Interest& callbackInterest = counter.interest_;
  ASSERT_TRUE(callbackInterest.getName().equals(Name(uri))) << "Interest returned on callback had different name";
}

TEST_F(TestFaceInterestMethods, RemovePending)
{
  Name name("/ndn/edu/ucla/remap/");
  CallbackCounter counter;
  uint64_t interestID = face.expressInterest
    (name, bind(&CallbackCounter::onData, &counter, _1, _2),
     bind(&CallbackCounter::onTimeout, &counter, _1));

  face.removePendingInterest(interestID);

  Milliseconds timeout = 10000;
  MillisecondsSince1970 startTime = getNowMilliseconds();
  while (getNowMilliseconds() - startTime < timeout &&
         counter.onDataCallCount_ == 0 && counter.onTimeoutCallCount_ == 0) {
    face.processEvents();
    // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
    usleep(10000);
  }

  ASSERT_EQ(counter.onDataCallCount_, 0) << "Should not have called data callback after interest was removed";
  ASSERT_TRUE(counter.onTimeoutCallCount_ == 0) << "Should not have called timeout callback after interest was removed";
}

TEST_F(TestFaceInterestMethods, MaxNdnPacketSize)
{
  // Construct an interest whose encoding is one byte larger than getMaxNdnPacketSize.
  const size_t targetSize = Face::getMaxNdnPacketSize() + 1;
  // Start with an interest which is almost the right size.
  uint8_t componentValue[targetSize];
  Interest interest;
  interest.getName().append(componentValue, targetSize);
  size_t initialSize = interest.wireEncode().size();
  // Now replace the component with the desired size which trims off the extra encoding.
  interest.setName
    (Name().append(componentValue, targetSize - (initialSize - targetSize)));
  size_t interestSize = interest.wireEncode().size();
  ASSERT_EQ(targetSize, interestSize) << "Wrong interest size for MaxNdnPacketSize";
  
  CallbackCounter counter;
  ASSERT_THROW
    (face.expressInterest
     (interest, bind(&CallbackCounter::onData, &counter, _1, _2),
      bind(&CallbackCounter::onTimeout, &counter, _1)),
     runtime_error) <<
    "expressInterest didn't throw an exception when the interest size exceeds getMaxNdnPacketSize()";
}

#if 0
class TestFaceRegisterMethods : public ::testing::Test {
public:
  TestFaceRegisterMethods()
  {
  }

  virtual void
  TearDown()
  {
    faceIn.shutdown();
    faceOut.shutdown();
  }

  Face faceIn;
  Face faceOut;
  KeyChain keyChain;
};

TEST_F(TestFaceRegisterMethods, RegisterPrefixResponse)
{
  // gotta sign it (WAT)
  Name prefixName("/test");
  faceIn.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

  RegisterCounter registerCounter(keyChain);

  faceIn.registerPrefix
    (prefixName,
     bind(&RegisterCounter::onInterest, &registerCounter, _1, _2, _3, _4),
     bind(&RegisterCounter::onRegisterFailed, &registerCounter, _1));

  // give the 'server' time to register the interest
  MillisecondsSince1970 startTime = getNowMilliseconds();
  while (getNowMilliseconds() - startTime < 1000 &&
         registerCounter.onInterestCallCount_ == 0 &&
         registerCounter.onRegisterFailedCallCount_ == 0) {
    faceIn.processEvents();
    usleep(10000);
  }

  CallbackCounter counter;

  // now express an interest on this new face, and see if onInterest is called
  Name interestName = Name(prefixName).append("hello");
  faceOut.expressInterest
    (interestName, bind(&CallbackCounter::onData, &counter, _1, _2),
     bind(&CallbackCounter::onTimeout, &counter, _1));

  startTime = getNowMilliseconds();
  while (getNowMilliseconds() - startTime < 10000 &&
         counter.onDataCallCount_ == 0 && counter.onTimeoutCallCount_ == 0) {
    faceIn.processEvents();
    faceOut.processEvents();
    usleep(10000);
  }

  ASSERT_EQ(registerCounter.onRegisterFailedCallCount_, 0) << "Failed to register prefix at all";

  ASSERT_EQ(registerCounter.onInterestCallCount_, 1) << "Expected 1 onInterest callback, got " << registerCounter.onInterestCallCount_;

  ASSERT_EQ(counter.onDataCallCount_, 1) << "Expected 1 onData callback, got " << counter.onDataCallCount_;

  // check the message content
  Data& data = counter.data_;
  string content("SUCCESS");
  Blob expectedBlob((const uint8_t *)&content[0], content.size());
  ASSERT_TRUE(expectedBlob.equals(data.getContent())) << "Data received on face does not match expected format";
}
#endif

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
