/**
 * Copyright (C) 2015-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From jNDN TestRegistrationCallbacks by Andrew Brown <andrew.brown@intel.com>
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

class RegisterCounter
{
public:
  RegisterCounter()
  {
    onRegisterFailedCallCount_ = 0;
    onRegisterSuccessCallCount_ = 0;
  }

  void
  onRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix)
  {
    ++onRegisterFailedCallCount_;
  }

  void
  onRegisterSuccess
    (const ptr_lib::shared_ptr<const Name>& prefix, uint64_t registeredPrefixId)
  {
    ++onRegisterSuccessCallCount_;
  }

  int onRegisterFailedCallCount_;
  int onRegisterSuccessCallCount_;
};

class TestRegistrationCallbacks : public ::testing::Test {
public:
  TestRegistrationCallbacks()
  {
    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());
  }

  virtual void
  TearDown()
  {
    face.shutdown();
  }

  Face face;
  KeyChain keyChain;
};

TEST_F(TestRegistrationCallbacks, RegistrationCallbacks)
{
  RegisterCounter counter;

  face.registerPrefix
    (Name("/test/register/callbacks"), OnInterestCallback(),
     bind(&RegisterCounter::onRegisterFailed, &counter, _1),
     bind(&RegisterCounter::onRegisterSuccess, &counter, _1, _2));

  double timeout = 10000;
  MillisecondsSince1970 startTime = getNowMilliseconds();
  while (getNowMilliseconds() - startTime < timeout &&
         counter.onRegisterFailedCallCount_ == 0 &&
         counter.onRegisterSuccessCallCount_ == 0) {
    face.processEvents();
    // We need to sleep for a few milliseconds so we don't use 100% of the CPU.
    usleep(10000);
  }

  ASSERT_EQ(1, counter.onRegisterSuccessCallCount_) <<
    "Expected 1 onRegisterSuccess callback, got " <<
    counter.onRegisterSuccessCallCount_;
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
