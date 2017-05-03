/**
 * Copyright (C) 2017 Regents of the University of California.
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
#include <ndn-cpp/lite/security/ec-public-key-lite.hpp>
#include <ndn-cpp/lite/security/rsa-public-key-lite.hpp>
#include <ndn-cpp/security/pib/pib-key.hpp>
#include <ndn-cpp/security/tpm/tpm.hpp>
#include <ndn-cpp/security/tpm/tpm-key-handle.hpp>
#include <ndn-cpp/security/tpm/tpm-back-end-memory.hpp>

using namespace std;
using namespace ndn;

class TestBackEnds : public ::testing::Test {
public:
  TestBackEnds()
  {
    backEndList[0] = &backEndMemory;
    // TODO: Add other back ends.
  }

  TpmBackEndMemory backEndMemory;

  TpmBackEnd* backEndList[1];
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
      (identityName, EcdsaKeyParams());
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
  TpmBackEnd& tpm = backEndMemory;

  Name identityName("/Test/KeyName");

  set<Name> keyNames;
  for (int i = 0; i < 100; i++) {
    ptr_lib::shared_ptr<TpmKeyHandle> key =
      tpm.createKey(identityName, RsaKeyParams());
    Name keyName = key->getKeyName();
    tpm.deleteKey(keyName);
    ASSERT_TRUE(!!keyNames.insert(keyName).second);
  }
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

