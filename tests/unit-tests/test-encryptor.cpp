/**
 * Copyright (C) 2016-2017 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/encryptor.t.cpp
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
#include <ndn-cpp/encrypt/encrypted-content.hpp>
#include <ndn-cpp/encrypt/algo/aes-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/rsa-algorithm.hpp>
#include <ndn-cpp/encrypt/algo/encryptor.hpp>

using namespace std;
using namespace ndn;

class SymmetricEncryptInput {
public:
  virtual ~SymmetricEncryptInput() {}
  virtual string testName() = 0;
  virtual Name keyName() = 0;
  virtual EncryptParams encryptParams() = 0;
  virtual Blob plainText() = 0;
  virtual Blob key() = 0;
  virtual Blob encryptedContent() = 0;
};

class TestDataAesEcb : public SymmetricEncryptInput {
public:
  string testName() { return "TestDataAesEcb"; }
  Name keyName() { return Name("/test"); }
  EncryptParams encryptParams() {
    return EncryptParams(ndn_EncryptAlgorithmType_AesEcb);
  }
  Blob plainText() {
    uint8_t value[] = {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
      0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x73
    };
    return Blob(value, sizeof(value));
  }
  Blob key() {
    uint8_t value[] = {
      0xdd, 0x60, 0x77, 0xec, 0xa9, 0x6b, 0x23, 0x1b,
      0x40, 0x6b, 0x5a, 0xf8, 0x7d, 0x3d, 0x55, 0x32
    };
    return Blob(value, sizeof(value));
  }
  Blob encryptedContent() {
    uint8_t value[] = {
      0x82, 0x2f,
        0x1c, 0x08,
          0x07, 0x06,
            0x08, 0x04, 0x74, 0x65, 0x73, 0x74,
        0x83, 0x01,
          0x00,
        0x84, 0x20,
          0x13, 0x80, 0x1a, 0xc0, 0x4c, 0x75, 0xa7, 0x7f,
          0x43, 0x5e, 0xd7, 0xa6, 0x3f, 0xd3, 0x68, 0x94,
          0xe2, 0xcf, 0x54, 0xb1, 0xc2, 0xce, 0xad, 0x9b,
          0x56, 0x6e, 0x1c, 0xe6, 0x55, 0x1d, 0x79, 0x04
    };
    return Blob(value, sizeof(value));
  }
};

class TestDataAesCbc : public SymmetricEncryptInput {
  string testName() { return "TestDataAesCbc"; }
  Name keyName() { return Name("/test"); }
  EncryptParams encryptParams() {
    EncryptParams result(ndn_EncryptAlgorithmType_AesCbc);
    uint8_t value[] = {
        0x73, 0x6f, 0x6d, 0x65, 0x72, 0x61, 0x6e, 0x64,
        0x6f, 0x6d, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72
    };
    result.setInitialVector(Blob(value, sizeof(value)));
    return result;
  }
  Blob plainText() {
    uint8_t value[] = {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
      0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x73
    };
    return Blob(value, sizeof(value));
  }
  Blob key() {
    uint8_t value[] = {
      0xdd, 0x60, 0x77, 0xec, 0xa9, 0x6b, 0x23, 0x1b,
      0x40, 0x6b, 0x5a, 0xf8, 0x7d, 0x3d, 0x55, 0x32
    };
    return Blob(value, sizeof(value));
  }
  Blob encryptedContent() {
    uint8_t value[] = {
      0x82, 0x41, // EncryptedContent
        0x1c, 0x08, // KeyLocator /test
          0x07, 0x06,
            0x08, 0x04, 0x74, 0x65, 0x73, 0x74,
        0x83, 0x01, // EncryptedAlgorithm
          0x01, // AlgorithmAesCbc
        0x85, 0x10,
          0x73, 0x6f, 0x6d, 0x65, 0x72, 0x61, 0x6e, 0x64,
          0x6f, 0x6d, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72,
        0x84, 0x20, // EncryptedPayLoad
          0x6a, 0x6b, 0x58, 0x9c, 0x30, 0x3b, 0xd9, 0xa6,
          0xed, 0xd2, 0x12, 0xef, 0x29, 0xad, 0xc3, 0x60,
          0x1f, 0x1b, 0x6b, 0xc7, 0x03, 0xff, 0x53, 0x52,
          0x82, 0x6d, 0x82, 0x73, 0x05, 0xf9, 0x03, 0xdc
    };
    return Blob(value, sizeof(value));
  }
};

class AsymmetricEncryptInput {
public:
  virtual string testName() = 0;
  virtual ndn_EncryptAlgorithmType type() = 0;
};

class TestDataRsaOaep : public AsymmetricEncryptInput {
public:
  string testName() { return "TestDataRsaOaep"; }
  ndn_EncryptAlgorithmType type() { return ndn_EncryptAlgorithmType_RsaOaep; }
};

class TestDataRsaPkcs : public AsymmetricEncryptInput {
public:
  string testName() { return "TestDataRsaPkcs"; }
  ndn_EncryptAlgorithmType type() { return ndn_EncryptAlgorithmType_RsaPkcs; }
};

class TestEncryptor : public ::testing::Test {
public:
  TestEncryptor() {
    encryptorAesTestInputs.push_back(ptr_lib::make_shared<TestDataAesEcb>());
    encryptorAesTestInputs.push_back(ptr_lib::make_shared<TestDataAesCbc>());

    encryptorRsaTestInputs.push_back(ptr_lib::make_shared<TestDataRsaOaep>());
    encryptorRsaTestInputs.push_back(ptr_lib::make_shared<TestDataRsaPkcs>());
  }

  vector<ptr_lib::shared_ptr<SymmetricEncryptInput> > encryptorAesTestInputs;
  vector<ptr_lib::shared_ptr<AsymmetricEncryptInput> > encryptorRsaTestInputs;
};

TEST_F(TestEncryptor, ContentSymmetricEncrypt)
{
  for (size_t i = 0; i < encryptorAesTestInputs.size(); ++i) {
    SymmetricEncryptInput& input = *encryptorAesTestInputs[i];

    Data data;
    Encryptor::encryptData
      (data, input.plainText(), input.keyName(), input.key(), input.encryptParams());

    ASSERT_EQ
      (Name("/FOR").append(input.keyName()), data.getName()) << input.testName();

    ASSERT_TRUE(input.encryptedContent().equals(data.getContent())) << input.testName();

    EncryptedContent content;
    content.wireDecode(data.getContent());
    Blob decryptedOutput = AesAlgorithm::decrypt
      (input.key(), content.getPayload(), input.encryptParams());

    ASSERT_TRUE(input.plainText().equals(decryptedOutput)) << input.testName();
  }
}

TEST_F(TestEncryptor, ContentAsymmetricEncryptSmall)
{
  for (size_t i = 0; i < encryptorRsaTestInputs.size(); ++i) {
    AsymmetricEncryptInput& input = *encryptorRsaTestInputs[i];

    uint8_t rawContentValue[] = {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
      0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x73
    };
    Blob rawContent(rawContentValue, sizeof(rawContentValue));

    Data data;
    RsaKeyParams rsaParams(1024);

    Name keyName("test");

    DecryptKey decryptKey = RsaAlgorithm::generateKey(rsaParams);
    EncryptKey encryptKey = RsaAlgorithm::deriveEncryptKey(decryptKey.getKeyBits());

    Blob eKey = encryptKey.getKeyBits();
    Blob dKey = decryptKey.getKeyBits();

    EncryptParams encryptParams(input.type());

    Encryptor::encryptData(data, rawContent, keyName, eKey, encryptParams);

    ASSERT_EQ(Name("/FOR").append(keyName), data.getName()) << input.testName();

    EncryptedContent extractContent;
    extractContent.wireDecode(data.getContent());
    ASSERT_EQ(keyName, extractContent.getKeyLocator().getKeyName())
      << input.testName();
    ASSERT_EQ(0, extractContent.getInitialVector().size()) << input.testName();
    ASSERT_EQ(input.type(), extractContent.getAlgorithmType()) << input.testName();

    Blob recovered = extractContent.getPayload();
    Blob decrypted = RsaAlgorithm::decrypt(dKey, recovered, encryptParams);
    ASSERT_TRUE(rawContent.equals(decrypted)) << input.testName();
  }
}

TEST_F(TestEncryptor, ContentAsymmetricEncryptLarge)
{
  for (size_t i = 0; i < encryptorRsaTestInputs.size(); ++i) {
    AsymmetricEncryptInput& input = *encryptorRsaTestInputs[i];

    uint8_t largeContentValue[] = {
      0x73, 0x5a, 0xbd, 0x47, 0x0c, 0xfe, 0xf8, 0x7d,
      0x2e, 0x17, 0xaa, 0x11, 0x6f, 0x23, 0xc5, 0x10,
      0x23, 0x36, 0x88, 0xc4, 0x2a, 0x0f, 0x9a, 0x72,
      0x54, 0x31, 0xa8, 0xb3, 0x51, 0x18, 0x9f, 0x0e,
      0x1b, 0x93, 0x62, 0xd9, 0xc4, 0xf5, 0xf4, 0x3d,
      0x61, 0x9a, 0xca, 0x05, 0x65, 0x6b, 0xc6, 0x41,
      0xf9, 0xd5, 0x1c, 0x67, 0xc1, 0xd0, 0xd5, 0x6f,
      0x7b, 0x70, 0xb8, 0x8f, 0xdb, 0x19, 0x68, 0x7c,
      0xe0, 0x2d, 0x04, 0x49, 0xa9, 0xa2, 0x77, 0x4e,
      0xfc, 0x60, 0x0d, 0x7c, 0x1b, 0x93, 0x6c, 0xd2,
      0x61, 0xc4, 0x6b, 0x01, 0xe9, 0x12, 0x28, 0x6d,
      0xf5, 0x78, 0xe9, 0x99, 0x0b, 0x9c, 0x4f, 0x90,
      0x34, 0x3e, 0x06, 0x92, 0x57, 0xe3, 0x7a, 0x8f,
      0x13, 0xc7, 0xf3, 0xfe, 0xf0, 0xe2, 0x59, 0x48,
      0x15, 0xb9, 0xdb, 0x77, 0x07, 0x1d, 0x6d, 0xb5,
      0x65, 0x17, 0xdf, 0x76, 0x6f, 0xb5, 0x43, 0xde,
      0x71, 0xac, 0xf1, 0x22, 0xbf, 0xb2, 0xe5, 0xd9,
      0x22, 0xf1, 0x67, 0x76, 0x71, 0x0c, 0xff, 0x99,
      0x7b, 0x94, 0x9b, 0x24, 0x20, 0x80, 0xe3, 0xcc,
      0x06, 0x4a, 0xed, 0xdf, 0xec, 0x50, 0xd5, 0x87,
      0x3d, 0xa0, 0x7d, 0x9c, 0xe5, 0x13, 0x10, 0x98,
      0x14, 0xc3, 0x90, 0x10, 0xd9, 0x25, 0x9a, 0x59,
      0xe9, 0x37, 0x26, 0xfd, 0x87, 0xd7, 0xf4, 0xf9,
      0x11, 0x91, 0xad, 0x5c, 0x00, 0x95, 0xf5, 0x2b,
      0x37, 0xf7, 0x4e, 0xb4, 0x4b, 0x42, 0x7c, 0xb3,
      0xad, 0xd6, 0x33, 0x5f, 0x0b, 0x84, 0x57, 0x7f,
      0xa7, 0x07, 0x73, 0x37, 0x4b, 0xab, 0x2e, 0xfb,
      0xfe, 0x1e, 0xcb, 0xb6, 0x4a, 0xc1, 0x21, 0x5f,
      0xec, 0x92, 0xb7, 0xac, 0x97, 0x75, 0x20, 0xc9,
      0xd8, 0x9e, 0x93, 0xd5, 0x12, 0x7a, 0x64, 0xb9,
      0x4c, 0xed, 0x49, 0x87, 0x44, 0x5b, 0x4f, 0x90,
      0x34, 0x3e, 0x06, 0x92, 0x57, 0xe3, 0x7a, 0x8f,
      0x13, 0xc7, 0xf3, 0xfe, 0xf0, 0xe2, 0x59, 0x48,
      0x15, 0xb9, 0xdb, 0x77, 0x07, 0x1d, 0x6d, 0xb5,
      0x65, 0x17, 0xdf, 0x76, 0x6f, 0xb5, 0x43, 0xde,
      0x71, 0xac, 0xf1, 0x22, 0xbf, 0xb2, 0xe5, 0xd9
    };
    Blob largeContent(largeContentValue, sizeof(largeContentValue));

    Data data;
    RsaKeyParams rsaParams(1024);

    Name keyName("test");

    DecryptKey decryptKey = RsaAlgorithm::generateKey(rsaParams);
    EncryptKey encryptKey = RsaAlgorithm::deriveEncryptKey(decryptKey.getKeyBits());

    Blob eKey = encryptKey.getKeyBits();
    Blob dKey = decryptKey.getKeyBits();

    EncryptParams encryptParams(input.type());
    Encryptor::encryptData(data, largeContent, keyName, eKey, encryptParams);

    ASSERT_EQ(Name("/FOR").append(keyName), data.getName()) << input.testName();

    Blob largeDataContent = data.getContent();

    // largeDataContent is a sequence of the two EncryptedContent.
    EncryptedContent encryptedNonce;
    encryptedNonce.wireDecode(largeDataContent);
    ASSERT_EQ(keyName, encryptedNonce.getKeyLocator().getKeyName()) << input.testName();
    ASSERT_EQ(0, encryptedNonce.getInitialVector().size()) << input.testName();
    ASSERT_EQ(input.type(), encryptedNonce.getAlgorithmType()) << input.testName();

    // Use the size of encryptedNonce to find the start of encryptedPayload.
    size_t encryptedNonceSize = encryptedNonce.wireEncode().size();
    EncryptedContent encryptedPayload;
    encryptedPayload.wireDecode
      (largeDataContent.buf() + encryptedNonceSize,
       largeDataContent.size() - encryptedNonceSize);
    Name nonceKeyName(keyName);
    nonceKeyName.append("nonce");
    ASSERT_EQ(nonceKeyName, encryptedPayload.getKeyLocator().getKeyName())
      << input.testName();
    ASSERT_EQ(16, encryptedPayload.getInitialVector().size()) << input.testName();
    ASSERT_EQ(ndn_EncryptAlgorithmType_AesCbc, encryptedPayload.getAlgorithmType())
      << input.testName();

    ASSERT_TRUE(encryptedNonce.wireEncode().size() +
                encryptedPayload.wireEncode().size() ==
                largeDataContent.size()) << input.testName();

    Blob blobNonce = encryptedNonce.getPayload();
    Blob nonce = RsaAlgorithm::decrypt(dKey, blobNonce, encryptParams);

    encryptParams.setAlgorithmType(ndn_EncryptAlgorithmType_AesCbc);
    encryptParams.setInitialVector(encryptedPayload.getInitialVector());
    Blob bufferPayload = encryptedPayload.getPayload();
    Blob largePayload = AesAlgorithm::decrypt(nonce, bufferPayload, encryptParams);

    ASSERT_TRUE(largeContent.equals(largePayload)) << input.testName();
  }
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
