/**
 * Copyright (C) 2017-2020 Regents of the University of California.
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
#include <ndn-cpp/security/tpm/tpm-private-key.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-file.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-osx.hpp>
#include "../../src/encoding/base64.hpp"

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

class TestTpmBackEnds : public ::testing::Test {
public:
  TestTpmBackEnds()
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

TEST_F(TestTpmBackEnds, KeyManagement)
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

TEST_F(TestTpmBackEnds, RsaSigning)
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

TEST_F(TestTpmBackEnds, RsaDecryption)
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

TEST_F(TestTpmBackEnds, EcdsaSigning)
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

TEST_F(TestTpmBackEnds, ImportExport)
{
  const char* privateKeyPkcs1Base64 =
"MIIEpAIBAAKCAQEAw0WM1/WhAxyLtEqsiAJgWDZWuzkYpeYVdeeZcqRZzzfRgBQT\
sNozS5t4HnwTZhwwXbH7k3QN0kRTV826Xobws3iigohnM9yTK+KKiayPhIAm/+5H\
GT6SgFJhYhqo1/upWdueojil6RP4/AgavHhopxlAVbk6G9VdVnlQcQ5Zv0OcGi73\
c+EnYD/YgURYGSngUi/Ynsh779p2U69/te9gZwIL5PuE9BiO6I39cL9z7EK1SfZh\
OWvDe/qH7YhD/BHwcWit8FjRww1glwRVTJsA9rH58ynaAix0tcR/nBMRLUX+e3rU\
RHg6UbSjJbdb9qmKM1fTGHKUzL/5pMG6uBU0ywIDAQABAoIBADQkckOIl4IZMUTn\
W8LFv6xOdkJwMKC8G6bsPRFbyY+HvC2TLt7epSvfS+f4AcYWaOPcDu2E49vt2sNr\
cASly8hgwiRRAB3dHH9vcsboiTo8bi2RFvMqvjv9w3tK2yMxVDtmZamzrrnaV3YV\
Q+5nyKo2F/PMDjQ4eUAKDOzjhBuKHsZBTFnA1MFNI+UKj5X4Yp64DFmKlxTX/U2b\
wzVywo5hzx2Uhw51jmoLls4YUvMJXD0wW5ZtYRuPogXvXb/of9ef/20/wU11WFKg\
Xb4gfR8zUXaXS1sXcnVm3+24vIs9dApUwykuoyjOqxWqcHRec2QT2FxVGkFEraze\
CPa4rMECgYEA5Y8CywomIcTgerFGFCeMHJr8nQGqY2V/owFb3k9maczPnC9p4a9R\
c5szLxA9FMYFxurQZMBWSEG2JS1HR2mnjigx8UKjYML/A+rvvjZOMe4M6Sy2ggh4\
SkLZKpWTzjTe07ByM/j5v/SjNZhWAG7sw4/LmPGRQkwJv+KZhGojuOkCgYEA2cOF\
T6cJRv6kvzTz9S0COZOVm+euJh/BXp7oAsAmbNfOpckPMzqHXy8/wpdKl6AAcB57\
OuztlNfV1D7qvbz7JuRlYwQ0cEfBgbZPcz1p18HHDXhwn57ZPb8G33Yh9Omg0HNA\
Imb4LsVuSqxA6NwSj7cpRekgTedrhLFPJ+Ydb5MCgYEAsM3Q7OjILcIg0t6uht9e\
vrlwTsz1mtCV2co2I6crzdj9HeI2vqf1KAElDt6G7PUHhglcr/yjd8uEqmWRPKNX\
ddnnfVZB10jYeP/93pac6z/Zmc3iU4yKeUe7U10ZFf0KkiiYDQd59CpLef/2XScS\
HB0oRofnxRQjfjLc4muNT+ECgYEAlcDk06MOOTly+F8lCc1bA1dgAmgwFd2usDBd\
Y07a3e0HGnGLN3Kfl7C5i0tZq64HvxLnMd2vgLVxQlXGPpdQrC1TH+XLXg+qnlZO\
ivSH7i0/gx75bHvj75eH1XK65V8pDVDEoSPottllAIs21CxLw3N1ObOZWJm2EfmR\
cuHICmsCgYAtFJ1idqMoHxES3mlRpf2JxyQudP3SCm2WpGmqVzhRYInqeatY5sUd\
lPLHm/p77RT7EyxQHTlwn8FJPuM/4ZH1rQd/vB+Y8qAtYJCexDMsbvLW+Js+VOvk\
jweEC0nrcL31j9mF0vz5E6tfRu4hhJ6L4yfWs0gSejskeVB/w8QY4g==";

  for (size_t i = 0; i < sizeof(backEndList) / sizeof(backEndList[0]); ++i) {
    TpmBackEnd& tpm = *backEndList[i];
#if NDN_CPP_HAVE_OSX_SECURITY
    if (&tpm == backEndOsx.get())
      // TODO: Implement TpmBackEndOsx import/export.
      continue;
#endif

    Name keyName("/Test/KeyName/KEY/1");
    tpm.deleteKey(keyName);
    ASSERT_EQ(false, tpm.hasKey(keyName));

    TpmPrivateKey privateKey;
    vector<uint8_t> privateKeyPkcs1Encoding;
    fromBase64(privateKeyPkcs1Base64, privateKeyPkcs1Encoding);
    privateKey.loadPkcs1(&privateKeyPkcs1Encoding[0], privateKeyPkcs1Encoding.size());

    string password("password");
    // Debug: Use toEncryptedPkcs8.
    Blob encryptedPkcs8 = privateKey.toPkcs8();

    // Debug: Use password.
    tpm.importKey(keyName, encryptedPkcs8.buf(), encryptedPkcs8.size(), 0, 0);
    ASSERT_EQ(true, tpm.hasKey(keyName));
    // Can't import the same keyName again.
    ASSERT_THROW
      (tpm.importKey(keyName, encryptedPkcs8.buf(), encryptedPkcs8.size(), 0, 0),
       TpmBackEnd::Error);

    // Debug: Use password.
    Blob exportedKey = tpm.exportKey(keyName, 0, 0);
    ASSERT_EQ(true, tpm.hasKey(keyName));

    TpmPrivateKey privateKey2;
    // Debug: Use loadEncryptedPkcs8.
    privateKey2.loadPkcs8(exportedKey.buf(), exportedKey.size());
    Blob privateKey2Pkcs1Encoding = privateKey2.toPkcs1();
    ASSERT_TRUE(Blob(privateKeyPkcs1Encoding).equals(privateKey2Pkcs1Encoding));

    tpm.deleteKey(keyName);
    ASSERT_EQ(false, tpm.hasKey(keyName));
    // Debug: Use password.
    ASSERT_THROW(tpm.exportKey(keyName, 0, 0), TpmBackEnd::Error);
  }
}

TEST_F(TestTpmBackEnds, RandomKeyId)
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

