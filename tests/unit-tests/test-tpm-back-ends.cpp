/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/tpm/back-end.t.cpp
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
#include <unistd.h>
#include <fstream>
#include <cstdio>
#include <ndn-cpp/lite/security/ec-public-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/pib/pib-key.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-file.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-osx.hpp>

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

class TestBackEnds : public ::testing::Test {
public:
  TestBackEnds()
  {
    backEndMemory.reset(new TpmBackEndMemory());
    backEndList[0] = backEndMemory.get();

    string locationPath = getPolicyConfigDirectory() + "/ndnsec-key-file";
    ::system(("rm -rf \"" + locationPath + "\"").c_str());
    backEndFile.reset(new TpmBackEndFile(locationPath));
    backEndList[1] = backEndFile.get();

#if NDN_CPP_HAVE_OSX_SECURITY
    backEndOsx.reset(new TpmBackEndOsx());
    backEndList[2] = backEndOsx.get();
#endif
  }

  ptr_lib::shared_ptr<TpmBackEndMemory> backEndMemory;
  ptr_lib::shared_ptr<TpmBackEndFile> backEndFile;
#if NDN_CPP_HAVE_OSX_SECURITY
  ptr_lib::shared_ptr<TpmBackEndOsx> backEndOsx;
#endif

#if NDN_CPP_HAVE_OSX_SECURITY
  TpmBackEnd* backEndList[3];
#else
  TpmBackEnd* backEndList[2];
#endif
};

TEST_F(TestBackEnds, KeyManagement)
{
  for (size_t i = 0; i < sizeof(backEndList) / sizeof(backEndList[0]); ++i) {
    TpmBackEnd& tpm = *backEndList[i];

    Name identityName("/Test/KeyName");
    Name::Component keyId("1");
    Name keyName = PibKey::constructKeyName(identityName, keyId);

    // The key should not exist.
    ASSERT_EQ(false, tpm.hasKey(keyName));
    ASSERT_TRUE(!tpm.getKeyHandle(keyName));

    // Create a key, which should exist.
    ASSERT_TRUE(!!tpm.createKey(identityName, RsaKeyParams(keyId)));
    ASSERT_TRUE(tpm.hasKey(keyName));
    ASSERT_TRUE(!!tpm.getKeyHandle(keyName));

    // Create a key with the same name, which should throw an error.
    ASSERT_THROW(tpm.createKey(identityName, RsaKeyParams(keyId)), Tpm::Error);

    // Delete the key, then it should not exist.
    tpm.deleteKey(keyName);
    ASSERT_EQ(false, tpm.hasKey(keyName));
    ASSERT_TRUE(!tpm.getKeyHandle(keyName));
  }
}

TEST_F(TestBackEnds, RsaSigning)
{
  for (size_t i = 0; i < sizeof(backEndList) / sizeof(backEndList[0]); ++i) {
    TpmBackEnd& tpm = *backEndList[i];

    // Create an RSA key.
    Name identityName("/Test/KeyName");

    ptr_lib::shared_ptr<TpmKeyHandle> key = tpm.createKey(identityName, RsaKeyParams());
    Name keyName = key->getKeyName();

    const uint8_t content[] = {0x01, 0x02, 0x03, 0x04};
    Blob signature = key->sign(DIGEST_ALGORITHM_SHA256, content, sizeof(content));

    RsaPublicKeyLite publicKey;
    publicKey.decode(key->derivePublicKey());

    bool result = publicKey.verifyWithSha256
      (signature.buf(), signature.size(), content, sizeof(content));
    ASSERT_EQ(true, result);

    tpm.deleteKey(keyName);
    ASSERT_EQ(false, tpm.hasKey(keyName));
  }
}

TEST_F(TestBackEnds, RsaDecryption)
{
  for (size_t i = 0; i < sizeof(backEndList) / sizeof(backEndList[0]); ++i) {
    TpmBackEnd& tpm = *backEndList[i];

    // Create an rsa key.
    Name identityName("/Test/KeyName");

    ptr_lib::shared_ptr<TpmKeyHandle> key = tpm.createKey(identityName, RsaKeyParams());
    Name keyName = key->getKeyName();

    const uint8_t content[] = {0x01, 0x02, 0x03, 0x04};

    RsaPublicKeyLite publicKey;
    publicKey.decode(key->derivePublicKey());

    uint8_t cipherText[1000];
    size_t cipherTextLength;
    publicKey.encrypt
      (content, sizeof(content), ndn_EncryptAlgorithmType_RsaOaep, cipherText,
       cipherTextLength);

    Blob plainText = key->decrypt(cipherText, cipherTextLength);

    ASSERT_TRUE(plainText.equals(Blob(content, sizeof(content))));

    tpm.deleteKey(keyName);
    ASSERT_EQ(false, tpm.hasKey(keyName));
  }
}

TEST_F(TestBackEnds, EcdsaSigning)
{
  for (size_t i = 0; i < sizeof(backEndList) / sizeof(backEndList[0]); ++i) {
    TpmBackEnd& tpm = *backEndList[i];

    // Create an EC key.
    Name identityName("/Test/Ec/KeyName");

    ptr_lib::shared_ptr<TpmKeyHandle> key = tpm.createKey
      (identityName, EcKeyParams());
    Name ecKeyName = key->getKeyName();

    const uint8_t content[] = {0x01, 0x02, 0x03, 0x04};
    Blob signature = key->sign(DIGEST_ALGORITHM_SHA256, content, sizeof(content));

    EcPublicKeyLite publicKey;
    publicKey.decode(key->derivePublicKey());

    bool result = publicKey.verifyWithSha256
      (signature.buf(), signature.size(), content, sizeof(content));
    ASSERT_EQ(true, result);

    tpm.deleteKey(ecKeyName);
    ASSERT_EQ(false, tpm.hasKey(ecKeyName));
  }
}

// TODO: ImportExport

TEST_F(TestBackEnds, RandomKeyId)
{
  TpmBackEnd& tpm = *backEndMemory;

  Name identityName("/Test/KeyName");

  set<Name> keyNames;
  for (int i = 0; i < 100; i++) {
    ptr_lib::shared_ptr<TpmKeyHandle> key =
      tpm.createKey(identityName, RsaKeyParams());
    Name keyName = key->getKeyName();
    ASSERT_TRUE(keyNames.insert(keyName).second);
  }
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

