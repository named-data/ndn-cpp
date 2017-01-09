/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/producer.t.cpp
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
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <ndn-cpp/security/identity/memory-identity-storage.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/encryptor.hpp>
#include <ndn-cpp/encrypt/encrypted-content.hpp>
#include <ndn-cpp/encrypt/schedule.hpp>
#include <ndn-cpp/encrypt/sqlite3-producer-db.hpp>
#include <ndn-cpp/encrypt/producer.hpp>

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

static bool
fileExists(const string& filePath)
{
  ifstream stream(filePath.c_str());
  bool result = (bool)stream;
  stream.close();
  return result;
}

static string
getPolicyConfigDirectory()
{
  string policyConfigDirectory = "policy_config";
  // Check if expected files are in this directory.
  if (!fileExists(policyConfigDirectory + "/regex_ruleset.conf")) {
    // Maybe we are running "make check" from the ndn-cpp root.  There may be
    //   a way to tell "make check" to run from tests/unit-tests, but for
    //   now just set policyConfigDirectory explicitly.
    policyConfigDirectory = "tests/unit-tests/policy_config";

    if(!fileExists(policyConfigDirectory + "/regex_ruleset.conf"))
      throw runtime_error("Cannot find the directory for policy-config");
  }

  return policyConfigDirectory;
}

static MillisecondsSince1970
fromIsoString(const string& dateString)
{
  return Schedule::fromIsoString(dateString);
}

static uint8_t DATA_CONTENT[] = {
    0xcb, 0xe5, 0x6a, 0x80, 0x41, 0x24, 0x58, 0x23,
    0x84, 0x14, 0x15, 0x61, 0x80, 0xb9, 0x5e, 0xbd,
    0xce, 0x32, 0xb4, 0xbe, 0xbc, 0x91, 0x31, 0xd6,
    0x19, 0x00, 0x80, 0x8b, 0xfa, 0x00, 0x05, 0x9c
};

class TestProducer : public ::testing::Test {
public:
  TestProducer()
  {
    string policyConfigDirectory = getPolicyConfigDirectory();

    databaseFilePath = policyConfigDirectory + "/test.db";
    remove(databaseFilePath.c_str());

    // Set up the keyChain.
    ptr_lib::shared_ptr<MemoryIdentityStorage> identityStorage
      (new MemoryIdentityStorage());
    ptr_lib::shared_ptr<MemoryPrivateKeyStorage> privateKeyStorage
      (new MemoryPrivateKeyStorage());
    keyChain.reset(new KeyChain
      (ptr_lib::make_shared<IdentityManager>(identityStorage, privateKeyStorage),
       ptr_lib::make_shared<NoVerifyPolicyManager>()));
    Name identityName("TestProducer");
    certificateName = keyChain->createIdentityAndCertificate(identityName);
    keyChain->getIdentityManager()->setDefaultIdentity(identityName);
  }

  virtual void
  TearDown()
  {
    remove(databaseFilePath.c_str());
  }

  void
  createEncryptionKey(Name eKeyName, const Name& timeMarker)
  {
    RsaKeyParams params;
    eKeyName = Name(eKeyName);
    eKeyName.append(timeMarker);

    Blob dKeyBlob = RsaAlgorithm::generateKey(params).getKeyBits();
    Blob eKeyBlob = RsaAlgorithm::deriveEncryptKey(dKeyBlob).getKeyBits();
    decryptionKeys[eKeyName] = dKeyBlob;

    ptr_lib::shared_ptr<Data> keyData(new Data(eKeyName));
    keyData->setContent(eKeyBlob);
    keyChain->sign(*keyData, certificateName);
    encryptionKeys[eKeyName] = keyData;
  }

  void
  checkEncryptionKeys
    (const vector<ptr_lib::shared_ptr<Data> >& result, 
     MillisecondsSince1970 testTime, const Name::Component roundedTime,
     int expectedExpressInterestCallCount, const int* expressInterestCallCount,
     Blob* contentKey, Name cKeyName, ptr_lib::shared_ptr<ProducerDb> testDb)
  {
    ASSERT_EQ(expectedExpressInterestCallCount, *expressInterestCallCount);

    ASSERT_EQ(true, testDb->hasContentKey(testTime));
    (*contentKey) = testDb->getContentKey(testTime);

    EncryptParams params(ndn_EncryptAlgorithmType_RsaOaep);
    for (size_t i = 0; i < result.size(); ++i) {
      const Data& key = *result[i];
      const Name& keyName = key.getName();
      ASSERT_EQ(cKeyName, keyName.getSubName(0, 6));
      ASSERT_EQ(keyName.get(6), roundedTime);
      ASSERT_EQ(keyName.get(7), Encryptor::getNAME_COMPONENT_FOR());
      ASSERT_EQ(true, decryptionKeys.find(keyName.getSubName(8)) != decryptionKeys.end());

      Blob decryptionKey = decryptionKeys[keyName.getSubName(8)];
      ASSERT_EQ(true, decryptionKey.size() != 0);
      const Blob& encryptedKeyEncoding = key.getContent();

      EncryptedContent content;
      content.wireDecode(encryptedKeyEncoding);
      Blob encryptedKey = content.getPayload();
      Blob retrievedKey = RsaAlgorithm::decrypt
        (decryptionKey, encryptedKey, params);

      ASSERT_TRUE(contentKey->equals(retrievedKey));
    }

    ASSERT_EQ(3, result.size());
  }

  void
  keySearchOnEncryptedKeys
    (const vector<ptr_lib::shared_ptr<Data> >& result, int* requestCount,
     Name cKeyName, Name timeMarkerThirdHop, Name expectedInterest)
  {
    ASSERT_EQ(3, *requestCount);
    ASSERT_EQ(1, result.size());

    const Data& keyData = *result[0];
    const Name& keyName = keyData.getName();
    ASSERT_EQ(cKeyName, keyName.getSubName(0, 4));
    ASSERT_EQ(timeMarkerThirdHop.get(0), keyName.get(4));
    ASSERT_EQ(Encryptor::getNAME_COMPONENT_FOR(), keyName.get(5));
    ASSERT_EQ(expectedInterest.append(timeMarkerThirdHop),
              keyName.getSubName(6));
  }

  void
  keyTimeoutOnEncryptedKeys
    (const vector<ptr_lib::shared_ptr<Data> >& result, int* timeoutCount)
  {
    ASSERT_EQ(4, *timeoutCount);
    ASSERT_EQ(0, result.size());
  }

  string databaseFilePath;
  ptr_lib::shared_ptr<KeyChain> keyChain;
  Name certificateName;

  map<Name, Blob> decryptionKeys;
  map<Name, ptr_lib::shared_ptr<Data> > encryptionKeys;
};

TEST_F(TestProducer, ContentKeyRequest)
{
  Name prefix("/prefix");
  Name suffix("/a/b/c");
  Name expectedInterest(prefix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_READ());
  expectedInterest.append(suffix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_E_KEY());

  Name cKeyName(prefix);
  cKeyName.append(Encryptor::getNAME_COMPONENT_SAMPLE());
  cKeyName.append(suffix);
  cKeyName.append(Encryptor::getNAME_COMPONENT_C_KEY());

  Name timeMarker("20150101T100000/20150101T120000");
  MillisecondsSince1970 testTime1 = fromIsoString("20150101T100001");
  MillisecondsSince1970 testTime2 = fromIsoString("20150101T110001");
  Name::Component testTimeRounded1("20150101T100000");
  Name::Component testTimeRounded2("20150101T110000");
  Name::Component testTimeComponent2("20150101T110001");

  // Create content keys required for this test case:
  for (size_t i = 0; i < suffix.size(); ++i) {
    createEncryptionKey(expectedInterest, timeMarker);
    expectedInterest = expectedInterest.getPrefix(-2).append
      (Encryptor::getNAME_COMPONENT_E_KEY());
  }

  int expressInterestCallCount = 0;

  // Prepare a TestFace to instantly answer calls to expressInterest.
  class TestFace : public Face {
  public:
    TestFace(TestProducer* parent, const Name& timeMarker,
             int* expressInterestCallCount)
    : Face("localhost"),
      parent_(parent),
      timeMarker_(timeMarker),
      expressInterestCallCount_(expressInterestCallCount)
    {}

    virtual uint64_t
    expressInterest
      (const Interest& interest, const OnData& onData,
       const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
       WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
    {
      ++(*expressInterestCallCount_);

      Name interestName(interest.getName());
      interestName.append(timeMarker_);
      if (parent_->encryptionKeys.find(interestName) == parent_->encryptionKeys.end())
        throw runtime_error
          ("TestFace::expressInterest: Can't find " + interestName.toUri());
      onData(ptr_lib::make_shared<Interest>(interest),
             parent_->encryptionKeys[interestName]);

      return 0;
    }

  private:
    TestProducer* parent_;
    Name timeMarker_;
    int *expressInterestCallCount_;
  };

  TestFace face(this, timeMarker, &expressInterestCallCount);

  // Verify that the content key is correctly encrypted for each domain, and
  // the produce method encrypts the provided data with the same content key.
  ptr_lib::shared_ptr<ProducerDb> testDb(new Sqlite3ProducerDb(databaseFilePath));
  Producer producer(prefix, suffix, &face, keyChain.get(), testDb);
  Blob contentKey;

  // An initial test to confirm that keys are created for this time slot.
  Name contentKeyName1 = producer.createContentKey
    (testTime1,
     bind(&TestProducer::checkEncryptionKeys, this, _1, testTime1,
          testTimeRounded1, 3, &expressInterestCallCount, &contentKey, cKeyName,
          testDb));

  // Verify that we do not repeat the search for e-keys. The total
  //   expressInterestCallCount should be the same.
  Name contentKeyName2 = producer.createContentKey
    (testTime2,
     bind(&TestProducer::checkEncryptionKeys, this, _1, testTime2,
          testTimeRounded2, 3, &expressInterestCallCount, &contentKey, cKeyName,
          testDb));

  // Confirm content key names are correct
  ASSERT_EQ(cKeyName, contentKeyName1.getPrefix(-1));
  ASSERT_EQ(testTimeRounded1, contentKeyName1.get(6));
  ASSERT_EQ(cKeyName, contentKeyName2.getPrefix(-1));
  ASSERT_EQ(testTimeRounded2, contentKeyName2.get(6));

  // Confirm that produce encrypts with the correct key and has the right name.
  Data testData;
  producer.produce(testData, testTime2, Blob(DATA_CONTENT, sizeof(DATA_CONTENT)));

  const Name& producedName = testData.getName();
  ASSERT_EQ(cKeyName.getPrefix(-1), producedName.getSubName(0, 5));
  ASSERT_EQ(testTimeComponent2, producedName.get(5));
  ASSERT_EQ(Encryptor::getNAME_COMPONENT_FOR(), producedName.get(6));
  ASSERT_EQ(cKeyName, producedName.getSubName(7, 6));
  ASSERT_EQ(testTimeRounded2, producedName.get(13));

  const Blob& dataBlob = testData.getContent();

  EncryptedContent dataContent;
  dataContent.wireDecode(dataBlob);
  const Blob& encryptedData = dataContent.getPayload();
  const Blob& initialVector = dataContent.getInitialVector();

  EncryptParams params(ndn_EncryptAlgorithmType_AesCbc, 16);
  params.setInitialVector(initialVector);
  Blob decryptTest = AesAlgorithm::decrypt(contentKey, encryptedData, params);
  ASSERT_TRUE(decryptTest.equals(Blob(DATA_CONTENT, sizeof(DATA_CONTENT))));
}

TEST_F(TestProducer, ContentKeySearch)
{
  Name timeMarkerFirstHop("20150101T070000/20150101T080000");
  Name timeMarkerSecondHop("20150101T080000/20150101T090000");
  Name timeMarkerThirdHop("20150101T100000/20150101T110000");

  Name prefix("/prefix");
  Name suffix("/suffix");
  Name expectedInterest(prefix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_READ());
  expectedInterest.append(suffix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_E_KEY());

  Name cKeyName(prefix);
  cKeyName.append(Encryptor::getNAME_COMPONENT_SAMPLE());
  cKeyName.append(suffix);
  cKeyName.append(Encryptor::getNAME_COMPONENT_C_KEY());

  MillisecondsSince1970 testTime = fromIsoString("20150101T100001");

  // Create content keys required for this test case:
  createEncryptionKey(expectedInterest, timeMarkerFirstHop);
  createEncryptionKey(expectedInterest, timeMarkerSecondHop);
  createEncryptionKey(expectedInterest, timeMarkerThirdHop);

  int requestCount = 0;

  // Prepare a TestFace to instantly answer calls to expressInterest.
  class TestFace : public Face {
  public:
    TestFace(TestProducer* parent, const Name& timeMarkerFirstHop,
             const Name& timeMarkerSecondHop, const Name& timeMarkerThirdHop,
             const Name& expectedInterest, int* requestCount)
    : Face("localhost"),
      parent_(parent),
      timeMarkerFirstHop_(timeMarkerFirstHop),
      timeMarkerSecondHop_(timeMarkerSecondHop),
      timeMarkerThirdHop_(timeMarkerThirdHop),
      expectedInterest_(expectedInterest),
      requestCount_(requestCount)
    {}

    virtual uint64_t
    expressInterest
      (const Interest& interest, const OnData& onData,
       const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
       WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
    {
      if (expectedInterest_ != interest.getName())
        throw runtime_error("TestFace::expressInterest: Not the expectedInterest_");

      bool gotInterestName = false;
      Name interestName;
      for (size_t i = 0; i < 3; ++i) {
        interestName = Name(interest.getName());
        if (i == 0)
          interestName.append(timeMarkerFirstHop_);
        else if (i == 1)
          interestName.append(timeMarkerSecondHop_);
        else if (i == 2)
          interestName.append(timeMarkerThirdHop_);

        // matchesName will check the Exclude.
        if (interest.matchesName(interestName)) {
          gotInterestName = true;
          ++(*requestCount_);
          break;
        }
      }

      if (gotInterestName)
        onData(ptr_lib::make_shared<Interest>(interest),
               parent_->encryptionKeys[interestName]);

      return 0;
    }

  private:
    TestProducer* parent_;
    Name timeMarkerFirstHop_;
    Name timeMarkerSecondHop_;
    Name timeMarkerThirdHop_;
    Name expectedInterest_;
    int* requestCount_;
  };

  TestFace face
    (this, timeMarkerFirstHop, timeMarkerSecondHop, timeMarkerThirdHop,
     expectedInterest, &requestCount);

  // Verify that if a key is found, but not within the right time slot, the
  // search is refined until a valid time slot is found.
  ptr_lib::shared_ptr<ProducerDb> testDb(new Sqlite3ProducerDb(databaseFilePath));
  Producer producer(prefix, suffix, &face, keyChain.get(), testDb);
  producer.createContentKey
    (testTime,
     bind(&TestProducer::keySearchOnEncryptedKeys, this, _1, &requestCount,
          cKeyName, timeMarkerThirdHop, expectedInterest));
}

TEST_F(TestProducer, ContentKeyTimeout)
{
  Name prefix("/prefix");
  Name suffix("/suffix");
  Name expectedInterest(prefix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_READ());
  expectedInterest.append(suffix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_E_KEY());

  MillisecondsSince1970 testTime = fromIsoString("20150101T100001");

  int timeoutCount = 0;

  // Prepare a TestFace to instantly answer calls to expressInterest.
  class TestFace : public Face {
  public:
    TestFace(const Name& expectedInterest, int* timeoutCount)
    : Face("localhost"),
      expectedInterest_(expectedInterest),
      timeoutCount_(timeoutCount)
    {}

    virtual uint64_t
    expressInterest
      (const Interest& interest, const OnData& onData,
       const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
       WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
    {
      if (expectedInterest_ != interest.getName())
        throw runtime_error("TestFace::expressInterest: Not the expectedInterest_");
      ++(*timeoutCount_);
      onTimeout(ptr_lib::make_shared<Interest>(interest));

      return 0;
    }

  private:
    Name expectedInterest_;
    int* timeoutCount_;
  };

  TestFace face(expectedInterest, &timeoutCount);

  // Verify that if no response is received, the producer appropriately times
  // out. The result vector should not contain elements that have timed out.
  ptr_lib::shared_ptr<ProducerDb> testDb(new Sqlite3ProducerDb(databaseFilePath));
  Producer producer(prefix, suffix, &face, keyChain.get(), testDb);
  producer.createContentKey
    (testTime,
     bind(&TestProducer::keyTimeoutOnEncryptedKeys, this, _1, &timeoutCount));
}

TEST_F(TestProducer, ProducerWithLink)
{
  Name prefix("/prefix");
  Name suffix("/suffix");
  Name expectedInterest = prefix;
  expectedInterest.append(Encryptor::getNAME_COMPONENT_READ());
  expectedInterest.append(suffix);
  expectedInterest.append(Encryptor::getNAME_COMPONENT_E_KEY());

  MillisecondsSince1970 testTime = fromIsoString("20150101T100001");

  int timeoutCount = 0;

  // Prepare a TestFace to instantly answer calls to expressInterest.
  class TestFace : public Face {
  public:
    TestFace(const Name& expectedInterest, int* timeoutCount)
    : Face("localhost"),
      expectedInterest_(expectedInterest),
      timeoutCount_(timeoutCount)
    {}

    virtual uint64_t
    expressInterest
      (const Interest& interest, const OnData& onData,
       const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
       WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
    {
      if (expectedInterest_ != interest.getName())
        throw runtime_error("TestFace::expressInterest: Not the expectedInterest_");
      if (interest.getLink()->getDelegations().size() != 3)
        throw runtime_error
          ("TestFace::expressInterest: The Interest link does not the expected number of delegates");
      ++(*timeoutCount_);
      onTimeout(ptr_lib::make_shared<Interest>(interest));

      return 0;
    }

  private:
    Name expectedInterest_;
    int* timeoutCount_;
  };

  TestFace face(expectedInterest, &timeoutCount);

  // Verify that if no response is received, the producer appropriately times
  // out. The result vector should not contain elements that have timed out.
  Link link;
  link.addDelegation(10,  Name("/test1"));
  link.addDelegation(20,  Name("/test2"));
  link.addDelegation(100, Name("/test3"));
  keyChain->sign(link);
  ptr_lib::shared_ptr<ProducerDb> testDb(new Sqlite3ProducerDb(databaseFilePath));
  Producer producer(prefix, suffix, &face, keyChain.get(), testDb, 3, link);
  producer.createContentKey
    (testTime,
     bind(&TestProducer::keyTimeoutOnEncryptedKeys, this, _1, &timeoutCount));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
