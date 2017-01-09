/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/group-manager.t.cpp
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
#include <ndn-cpp/encoding/tlv-wire-format.hpp>
#include <ndn-cpp/encrypt/encrypted-content.hpp>
#include <ndn-cpp/encrypt/schedule.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/sqlite3-group-manager-db.hpp>
#include <ndn-cpp/encrypt/group-manager.hpp>

using namespace std;
using namespace ndn;

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

string
toIsoString(MillisecondsSince1970 msSince1970)
{
  return Schedule::toIsoString(msSince1970);
}

static uint8_t SIG_INFO[] = {
  0x16, 0x1b, // SignatureInfo
      0x1b, 0x01, // SignatureType
          0x01,
      0x1c, 0x16, // KeyLocator
          0x07, 0x14, // Name
              0x08, 0x04,
                  0x74, 0x65, 0x73, 0x74,
              0x08, 0x03,
                  0x6b, 0x65, 0x79,
              0x08, 0x07,
                  0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72
};

static uint8_t SIG_VALUE[] = {
  0x17, 0x80, // SignatureValue
    0x2f, 0xd6, 0xf1, 0x6e, 0x80, 0x6f, 0x10, 0xbe, 0xb1, 0x6f, 0x3e, 0x31, 0xec,
    0xe3, 0xb9, 0xea, 0x83, 0x30, 0x40, 0x03, 0xfc, 0xa0, 0x13, 0xd9, 0xb3, 0xc6,
    0x25, 0x16, 0x2d, 0xa6, 0x58, 0x41, 0x69, 0x62, 0x56, 0xd8, 0xb3, 0x6a, 0x38,
    0x76, 0x56, 0xea, 0x61, 0xb2, 0x32, 0x70, 0x1c, 0xb6, 0x4d, 0x10, 0x1d, 0xdc,
    0x92, 0x8e, 0x52, 0xa5, 0x8a, 0x1d, 0xd9, 0x96, 0x5e, 0xc0, 0x62, 0x0b, 0xcf,
    0x3a, 0x9d, 0x7f, 0xca, 0xbe, 0xa1, 0x41, 0x71, 0x85, 0x7a, 0x8b, 0x5d, 0xa9,
    0x64, 0xd6, 0x66, 0xb4, 0xe9, 0x8d, 0x0c, 0x28, 0x43, 0xee, 0xa6, 0x64, 0xe8,
    0x55, 0xf6, 0x1c, 0x19, 0x0b, 0xef, 0x99, 0x25, 0x1e, 0xdc, 0x78, 0xb3, 0xa7,
    0xaa, 0x0d, 0x14, 0x58, 0x30, 0xe5, 0x37, 0x6a, 0x6d, 0xdb, 0x56, 0xac, 0xa3,
    0xfc, 0x90, 0x7a, 0xb8, 0x66, 0x9c, 0x0e, 0xf6, 0xb7, 0x64, 0xd1
};

class TestGroupManager : public ::testing::Test {
public:
  TestGroupManager()
  {
    string policyConfigDirectory = getPolicyConfigDirectory();

    dKeyDatabaseFilePath = policyConfigDirectory + "/manager-d-key-test.db";
    remove(dKeyDatabaseFilePath.c_str());

    eKeyDatabaseFilePath = policyConfigDirectory + "/manager-e-key-test.db";
    remove(eKeyDatabaseFilePath.c_str());

    intervalDatabaseFilePath = policyConfigDirectory + "/manager-interval-test.db";
    remove(intervalDatabaseFilePath.c_str());

    groupKeyDatabaseFilePath = policyConfigDirectory + "/manager-group-key-test.db";
    remove(groupKeyDatabaseFilePath.c_str());

    RsaKeyParams params;
    DecryptKey memberDecryptKey = RsaAlgorithm::generateKey(params);
    decryptKeyBlob = memberDecryptKey.getKeyBits();
    EncryptKey memberEncryptKey = RsaAlgorithm::deriveEncryptKey(decryptKeyBlob);
    encryptKeyBlob = memberEncryptKey.getKeyBits();

    // Generate the certificate.
    certificate.setName(Name("/ndn/memberA/KEY/ksk-123/ID-CERT/123"));
    PublicKey contentPublicKey(encryptKeyBlob);
    certificate.setPublicKeyInfo(contentPublicKey);
    certificate.setNotBefore(0);
    certificate.setNotAfter(0);
    certificate.encode();

    Blob signatureInfoBlob(SIG_INFO, sizeof(SIG_INFO));
    Blob signatureValueBlob(SIG_VALUE, sizeof(SIG_VALUE));

    ptr_lib::shared_ptr<Signature> signature =
      TlvWireFormat::get()->decodeSignatureInfoAndValue
        (signatureInfoBlob.buf(), signatureInfoBlob.size(), 
         signatureValueBlob.buf(), signatureValueBlob.size());
    certificate.setSignature(*signature);

    certificate.wireEncode();

    // Set up the keyChain.
    ptr_lib::shared_ptr<MemoryIdentityStorage> identityStorage
      (new MemoryIdentityStorage());
    ptr_lib::shared_ptr<MemoryPrivateKeyStorage> privateKeyStorage
      (new MemoryPrivateKeyStorage());
    keyChain.reset(new KeyChain
      (ptr_lib::make_shared<IdentityManager>(identityStorage, privateKeyStorage),
       ptr_lib::make_shared<NoVerifyPolicyManager>()));
    Name identityName("TestGroupManager");
    keyChain->createIdentityAndCertificate(identityName);
    keyChain->getIdentityManager()->setDefaultIdentity(identityName);
  }

  virtual void
  TearDown()
  {
    remove(dKeyDatabaseFilePath.c_str());
    remove(eKeyDatabaseFilePath.c_str());
    remove(intervalDatabaseFilePath.c_str());
    remove(groupKeyDatabaseFilePath.c_str());
  }

  void
  setManager(GroupManager& manager)
  {
    // Set up the first schedule.
    Schedule schedule1;
    ptr_lib::shared_ptr<RepetitiveInterval> interval11(new RepetitiveInterval
      (fromIsoString("20150825T000000"),
       fromIsoString("20150827T000000"), 5, 10, 2,
       RepetitiveInterval::RepeatUnit::DAY));
    ptr_lib::shared_ptr<RepetitiveInterval> interval12(new RepetitiveInterval
      (fromIsoString("20150825T000000"),
       fromIsoString("20150827T000000"), 6, 8, 1,
       RepetitiveInterval::RepeatUnit::DAY));
    ptr_lib::shared_ptr<RepetitiveInterval> interval13(new RepetitiveInterval
      (fromIsoString("20150827T000000"),
       fromIsoString("20150827T000000"), 7, 8));
    schedule1.addWhiteInterval(interval11);
    schedule1.addWhiteInterval(interval12);
    schedule1.addBlackInterval(interval13);

    // Set up the second schedule.
    Schedule schedule2;
    ptr_lib::shared_ptr<RepetitiveInterval> interval21(new RepetitiveInterval
      (fromIsoString("20150825T000000"),
       fromIsoString("20150827T000000"), 9, 12, 1,
       RepetitiveInterval::RepeatUnit::DAY));
    ptr_lib::shared_ptr<RepetitiveInterval> interval22(new RepetitiveInterval
      (fromIsoString("20150827T000000"),
       fromIsoString("20150827T000000"), 6, 8));
    ptr_lib::shared_ptr<RepetitiveInterval> interval23(new RepetitiveInterval
      (fromIsoString("20150827T000000"),
       fromIsoString("20150827T000000"), 2, 4));
    schedule2.addWhiteInterval(interval21);
    schedule2.addWhiteInterval(interval22);
    schedule2.addBlackInterval(interval23);

    // Add them to the group manager database.
    manager.addSchedule("schedule1", schedule1);
    manager.addSchedule("schedule2", schedule2);

    // Make some adaptions to certificate.
    Blob dataBlob = certificate.wireEncode();

    Data memberA;
    memberA.wireDecode(dataBlob, *TlvWireFormat::get());
    memberA.setName(Name("/ndn/memberA/KEY/ksk-123/ID-CERT/123"));
    Data memberB;
    memberB.wireDecode(dataBlob, *TlvWireFormat::get());
    memberB.setName(Name("/ndn/memberB/KEY/ksk-123/ID-CERT/123"));
    Data memberC;
    memberC.wireDecode(dataBlob, *TlvWireFormat::get());
    memberC.setName(Name("/ndn/memberC/KEY/ksk-123/ID-CERT/123"));

    // Add the members to the database.
    manager.addMember("schedule1", memberA);
    manager.addMember("schedule1", memberB);
    manager.addMember("schedule2", memberC);
  }

  string dKeyDatabaseFilePath;
  string eKeyDatabaseFilePath;
  string intervalDatabaseFilePath;
  string groupKeyDatabaseFilePath;
  Blob decryptKeyBlob;
  Blob encryptKeyBlob;
  IdentityCertificate certificate;
  ptr_lib::shared_ptr<KeyChain> keyChain;
};

TEST_F(TestGroupManager, CreateDKeyData)
{
  // Create the group manager.
  GroupManager manager
    (Name("Alice"), Name("data_type"),
     ptr_lib::make_shared<Sqlite3GroupManagerDb>(dKeyDatabaseFilePath), 2048, 1,
     keyChain.get());

  Blob newCertificateBlob = certificate.wireEncode();
  IdentityCertificate newCertificate;
  newCertificate.wireDecode(newCertificateBlob);

  // Encrypt the D-KEY.
  ptr_lib::shared_ptr<Data> data = manager.createDKeyData
    ("20150825T000000", "20150827T000000", Name("/ndn/memberA/KEY"),
     decryptKeyBlob, newCertificate.getPublicKeyInfo().getKeyDer());

  // Verify the encrypted D-KEY.
  Blob dataContent = data->getContent();

  // Get the nonce key.
  // dataContent is a sequence of the two EncryptedContent.
  EncryptedContent encryptedNonce;
  encryptedNonce.wireDecode(dataContent);
  ASSERT_EQ(0, encryptedNonce.getInitialVector().size());
  ASSERT_EQ(ndn_EncryptAlgorithmType_RsaOaep, encryptedNonce.getAlgorithmType());

  Blob blobNonce = encryptedNonce.getPayload();
  EncryptParams decryptParams(ndn_EncryptAlgorithmType_RsaOaep);
  Blob nonce = RsaAlgorithm::decrypt(decryptKeyBlob, blobNonce, decryptParams);

  // Get the D-KEY.
  // Use the size of encryptedNonce to find the start of encryptedPayload.
  size_t encryptedNonceSize = encryptedNonce.wireEncode().size();
  EncryptedContent encryptedPayload;
  encryptedPayload.wireDecode
    (dataContent.buf() + encryptedNonceSize,
     dataContent.size() - encryptedNonceSize);
  ASSERT_EQ(16, encryptedPayload.getInitialVector().size());
  ASSERT_EQ(ndn_EncryptAlgorithmType_AesCbc, encryptedPayload.getAlgorithmType());

  decryptParams.setAlgorithmType(ndn_EncryptAlgorithmType_AesCbc);
  decryptParams.setInitialVector(encryptedPayload.getInitialVector());
  Blob blobPayload = encryptedPayload.getPayload();
  Blob largePayload = AesAlgorithm::decrypt(nonce, blobPayload, decryptParams);

  ASSERT_TRUE(largePayload.equals(decryptKeyBlob));
}

TEST_F(TestGroupManager, CreateEKeyData)
{
  // Create the group manager.
  GroupManager manager
    (Name("Alice"), Name("data_type"),
     ptr_lib::make_shared<Sqlite3GroupManagerDb>(eKeyDatabaseFilePath), 1024, 1,
     keyChain.get());
  setManager(manager);

  ptr_lib::shared_ptr<Data> data = manager.createEKeyData
    ("20150825T090000", "20150825T110000", encryptKeyBlob);
  ASSERT_EQ("/Alice/READ/data_type/E-KEY/20150825T090000/20150825T110000",
            data->getName().toUri());

  Blob contentBlob = data->getContent();
  ASSERT_TRUE(encryptKeyBlob.equals(contentBlob));
}

TEST_F(TestGroupManager, CalculateInterval)
{
  // Create the group manager.
  GroupManager manager
    (Name("Alice"), Name("data_type"),
     ptr_lib::make_shared<Sqlite3GroupManagerDb>(intervalDatabaseFilePath),
     1024, 1, keyChain.get());
  setManager(manager);

  map<Name, Blob> memberKeys;
  Interval result;

  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T093000");
  result = manager.calculateInterval(timePoint1, memberKeys);
  ASSERT_EQ("20150825T090000", toIsoString(result.getStartTime()));
  ASSERT_EQ("20150825T100000", toIsoString(result.getEndTime()));

  MillisecondsSince1970 timePoint2 = fromIsoString("20150827T073000");
  result = manager.calculateInterval(timePoint2, memberKeys);
  ASSERT_EQ("20150827T070000", toIsoString(result.getStartTime()));
  ASSERT_EQ("20150827T080000", toIsoString(result.getEndTime()));

  MillisecondsSince1970 timePoint3 = fromIsoString("20150827T043000");
  result = manager.calculateInterval(timePoint3, memberKeys);
  ASSERT_EQ(false, result.isValid());

  MillisecondsSince1970 timePoint4 = fromIsoString("20150827T053000");
  result = manager.calculateInterval(timePoint4, memberKeys);
  ASSERT_EQ("20150827T050000", toIsoString(result.getStartTime()));
  ASSERT_EQ("20150827T060000", toIsoString(result.getEndTime()));
}

TEST_F(TestGroupManager, GetGroupKey)
{
  // Create the group manager.
  GroupManager manager
    (Name("Alice"), Name("data_type"),
     ptr_lib::make_shared<Sqlite3GroupManagerDb>(groupKeyDatabaseFilePath),
     1024, 1, keyChain.get());
  setManager(manager);

  // Get the data list from the group manager.
  MillisecondsSince1970 timePoint1 = fromIsoString("20150825T093000");
  vector<ptr_lib::shared_ptr<Data> > result;
  manager.getGroupKey(timePoint1, result);

  ASSERT_EQ(4, result.size());

  // The first data packet contains the group's encryption key (public key).
  Data* data = result[0].get();
  ASSERT_EQ
    ("/Alice/READ/data_type/E-KEY/20150825T090000/20150825T100000",
     data->getName().toUri());
  EncryptKey groupEKey(data->getContent());

  // Get the second data packet and decrypt.
  data = result[1].get();
  ASSERT_EQ
    ("/Alice/READ/data_type/D-KEY/20150825T090000/20150825T100000/FOR/ndn/memberA/ksk-123",
     data->getName().toUri());

  /////////////////////////////////////////////////////// Start decryption.
  Blob dataContent = data->getContent();

  // Get the nonce key.
  // dataContent is a sequence of the two EncryptedContent.
  EncryptedContent encryptedNonce;
  encryptedNonce.wireDecode(dataContent);
  ASSERT_EQ(0, encryptedNonce.getInitialVector().size());
  ASSERT_EQ(ndn_EncryptAlgorithmType_RsaOaep, encryptedNonce.getAlgorithmType());

  EncryptParams decryptParams(ndn_EncryptAlgorithmType_RsaOaep);
  Blob blobNonce = encryptedNonce.getPayload();
  Blob nonce = RsaAlgorithm::decrypt(decryptKeyBlob, blobNonce, decryptParams);

  // Get the payload.
  // Use the size of encryptedNonce to find the start of encryptedPayload.
  size_t encryptedNonceSize = encryptedNonce.wireEncode().size();
  EncryptedContent encryptedPayload;
  encryptedPayload.wireDecode
    (dataContent.buf() + encryptedNonceSize,
     dataContent.size() - encryptedNonceSize);
  ASSERT_EQ(16, encryptedPayload.getInitialVector().size());
  ASSERT_EQ(ndn_EncryptAlgorithmType_AesCbc, encryptedPayload.getAlgorithmType());

  decryptParams.setAlgorithmType(ndn_EncryptAlgorithmType_AesCbc);
  decryptParams.setInitialVector(encryptedPayload.getInitialVector());
  Blob blobPayload = encryptedPayload.getPayload();
  Blob largePayload = AesAlgorithm::decrypt(nonce, blobPayload, decryptParams);

  // Get the group D-KEY.
  DecryptKey groupDKey(largePayload);

  /////////////////////////////////////////////////////// End decryption.

  // Check the D-KEY.
  EncryptKey derivedGroupEKey = RsaAlgorithm::deriveEncryptKey
    (groupDKey.getKeyBits());
  ASSERT_TRUE(groupEKey.getKeyBits().equals(derivedGroupEKey.getKeyBits()));

  // Check the third data packet.
  data = result[2].get();
  ASSERT_EQ
    ("/Alice/READ/data_type/D-KEY/20150825T090000/20150825T100000/FOR/ndn/memberB/ksk-123",
     data->getName().toUri());

  // Check the fourth data packet.
  data = result[3].get();
  ASSERT_EQ
    ("/Alice/READ/data_type/D-KEY/20150825T090000/20150825T100000/FOR/ndn/memberC/ksk-123",
     data->getName().toUri());

  // Check invalid time stamps for getting the group key.
  MillisecondsSince1970 timePoint2 = fromIsoString("20150826T083000");
  vector<ptr_lib::shared_ptr<Data> > tempResult;
  manager.getGroupKey(timePoint2, tempResult);
  ASSERT_EQ(0, tempResult.size());

  MillisecondsSince1970 timePoint3 = fromIsoString("20150827T023000");
  manager.getGroupKey(timePoint3, tempResult);
  ASSERT_EQ(0, tempResult.size());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

