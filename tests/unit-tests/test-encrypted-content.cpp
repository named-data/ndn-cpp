/**
 * Copyright (C) 2016 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-group-encrypt unit tests
 * https://github.com/named-data/ndn-group-encrypt/blob/master/tests/unit-tests/encrypted-content.t.cpp
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

using namespace std;
using namespace ndn;

static uint8_t ENCRYPTED[] = {
0x82, 0x30, // EncryptedContent
  0x1c, 0x16, // KeyLocator
    0x07, 0x14, // Name
      0x08, 0x04,
        0x74, 0x65, 0x73, 0x74, // 'test'
      0x08, 0x03,
        0x6b, 0x65, 0x79, // 'key'
      0x08, 0x07,
        0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72, // 'locator'
  0x83, 0x01, // EncryptedAlgorithm
    0x03,
  0x85, 0x0a, // InitialVector
    0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
  0x84, 0x07, // EncryptedPayload
    0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
};

static uint8_t ENCRYPTED_NO_IV[] = {
0x82, 0x24, // EncryptedContent
  0x1c, 0x16, // KeyLocator
    0x07, 0x14, // Name
      0x08, 0x04,
        0x74, 0x65, 0x73, 0x74, // 'test'
      0x08, 0x03,
        0x6b, 0x65, 0x79, // 'key'
      0x08, 0x07,
        0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72, // 'locator'
  0x83, 0x01, // EncryptedAlgorithm
    0x03,
  0x84, 0x07, // EncryptedPayload
    0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
};

static uint8_t MESSAGE[] = {
  0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
};

static uint8_t IV[] = {
  0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73
};

class TestEncyptedContent : public ::testing::Test {
};

TEST_F(TestEncyptedContent, Constructor)
{
  EncryptedContent content;
  ASSERT_EQ(content.getAlgorithmType(), (ndn_EncryptAlgorithmType)-1);
  ASSERT_EQ(content.getPayload().isNull(), true);
  ASSERT_EQ(content.getInitialVector().isNull(), true);
  ASSERT_EQ(content.getKeyLocator().getType(), (ndn_KeyLocatorType)-1);

  Blob payload(MESSAGE, sizeof(MESSAGE));
  Blob initialVector(IV, sizeof(IV));

  KeyLocator keyLocator;
  keyLocator.setType(ndn_KeyLocatorType_KEYNAME);
  keyLocator.getKeyName().set("/test/key/locator");
  content.setAlgorithmType(ndn_EncryptAlgorithmType_RsaOaep)
    .setKeyLocator(keyLocator).setPayload(payload)
    .setInitialVector(initialVector);

  // Test the copy constructor.
  EncryptedContent sha256RsaContent(content);
  Blob contentPayload = sha256RsaContent.getPayload();
  Blob contentInitialVector = sha256RsaContent.getInitialVector();

  ASSERT_EQ(sha256RsaContent.getAlgorithmType(), ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_TRUE(contentPayload.equals(payload));
  ASSERT_TRUE(contentInitialVector.equals(initialVector));
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getType() != (ndn_KeyLocatorType)-1);
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getKeyName().equals
              (Name("/test/key/locator")));

  Blob encryptedBlob(ENCRYPTED, sizeof(ENCRYPTED));
  Blob encoded = sha256RsaContent.wireEncode();

  ASSERT_TRUE(encryptedBlob.equals(encoded));

  sha256RsaContent = EncryptedContent();
  sha256RsaContent.wireDecode(encryptedBlob);
  contentPayload = sha256RsaContent.getPayload();
  contentInitialVector = sha256RsaContent.getInitialVector();

  ASSERT_EQ(sha256RsaContent.getAlgorithmType(), ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_TRUE(contentPayload.equals(payload));
  ASSERT_TRUE(contentInitialVector.equals(initialVector));
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getType() != (ndn_KeyLocatorType)-1);
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getKeyName().equals
              (Name("/test/key/locator")));

  // Test no IV.
  sha256RsaContent = EncryptedContent();
  sha256RsaContent.setAlgorithmType(ndn_EncryptAlgorithmType_RsaOaep)
    .setKeyLocator(keyLocator).setPayload(payload);
  contentPayload = sha256RsaContent.getPayload();

  ASSERT_EQ(sha256RsaContent.getAlgorithmType(), ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_TRUE(contentPayload.equals(payload));
  ASSERT_TRUE(sha256RsaContent.getInitialVector().isNull());
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getType() != (ndn_KeyLocatorType)-1);
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getKeyName().equals
             (Name("/test/key/locator")));

  encryptedBlob = Blob(ENCRYPTED_NO_IV, sizeof(ENCRYPTED_NO_IV));
  Blob encodedNoIv = sha256RsaContent.wireEncode();

  ASSERT_TRUE(encryptedBlob.equals(encodedNoIv));

  sha256RsaContent = EncryptedContent();
  sha256RsaContent.wireDecode(encryptedBlob);
  Blob contentPayloadNoIV = sha256RsaContent.getPayload();

  ASSERT_EQ(sha256RsaContent.getAlgorithmType(), ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_TRUE(contentPayloadNoIV.equals(payload));
  ASSERT_TRUE(sha256RsaContent.getInitialVector().isNull());
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getType() != (ndn_KeyLocatorType)-1);
  ASSERT_TRUE(sha256RsaContent.getKeyLocator().getKeyName().equals
             (Name("/test/key/locator")));
}

TEST_F(TestEncyptedContent, DecodingError)
{
  EncryptedContent encryptedContent;

  uint8_t encoding1[] = {
    0x1f, 0x30, // Wrong EncryptedContent (0x82, 0x24)
      0x1c, 0x16, // KeyLocator
        0x07, 0x14, // Name
          0x08, 0x04,
            0x74, 0x65, 0x73, 0x74,
          0x08, 0x03,
            0x6b, 0x65, 0x79,
          0x08, 0x07,
            0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72,
      0x83, 0x01, // EncryptedAlgorithm
        0x00,
      0x85, 0x0a, // InitialVector
        0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
      0x84, 0x07, // EncryptedPayload
        0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
  };
  Blob errorBlob1(encoding1, sizeof(encoding1));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob1), runtime_error) <<
    "wireDecode did not throw an exception";

  uint8_t encoding2[] = {
    0x82, 0x30, // EncryptedContent
      0x1d, 0x16, // Wrong KeyLocator (0x1c, 0x16)
        0x07, 0x14, // Name
          0x08, 0x04,
            0x74, 0x65, 0x73, 0x74,
          0x08, 0x03,
            0x6b, 0x65, 0x79,
          0x08, 0x07,
            0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72,
      0x83, 0x01, // EncryptedAlgorithm
        0x00,
      0x85, 0x0a, // InitialVector
        0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
      0x84, 0x07, // EncryptedPayload
        0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
  };
  Blob errorBlob2(encoding2, sizeof(encoding2));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob2), runtime_error) <<
    "wireDecode did not throw an exception";

  uint8_t encoding3[] = {
    0x82, 0x30, // EncryptedContent
      0x1c, 0x16, // KeyLocator
        0x07, 0x14, // Name
          0x08, 0x04,
            0x74, 0x65, 0x73, 0x74,
          0x08, 0x03,
            0x6b, 0x65, 0x79,
          0x08, 0x07,
            0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72,
      0x1d, 0x01, // Wrong EncryptedAlgorithm (0x83, 0x01)
        0x00,
      0x85, 0x0a, // InitialVector
        0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
      0x84, 0x07, // EncryptedPayload
        0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
  };
  Blob errorBlob3(encoding3, sizeof(encoding3));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob3), runtime_error) <<
    "wireDecode did not throw an exception";

  uint8_t encoding4[] = {
    0x82, 0x30, // EncryptedContent
      0x1c, 0x16, // KeyLocator
        0x07, 0x14, // Name
          0x08, 0x04,
            0x74, 0x65, 0x73, 0x74, // 'test'
          0x08, 0x03,
            0x6b, 0x65, 0x79, // 'key'
          0x08, 0x07,
            0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72, // 'locator'
      0x83, 0x01, // EncryptedAlgorithm
        0x00,
      0x1f, 0x0a, // InitialVector (0x84, 0x0a)
        0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
      0x84, 0x07, // EncryptedPayload
        0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
  };
  Blob errorBlob4(encoding4, sizeof(encoding4));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob4), runtime_error) <<
    "wireDecode did not throw an exception";

  uint8_t encoding5[] = {
    0x82, 0x30, // EncryptedContent
      0x1c, 0x16, // KeyLocator
        0x07, 0x14, // Name
          0x08, 0x04,
            0x74, 0x65, 0x73, 0x74, // 'test'
          0x08, 0x03,
            0x6b, 0x65, 0x79, // 'key'
          0x08, 0x07,
            0x6c, 0x6f, 0x63, 0x61, 0x74, 0x6f, 0x72, // 'locator'
      0x83, 0x01, // EncryptedAlgorithm
        0x00,
      0x85, 0x0a, // InitialVector
        0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x62, 0x69, 0x74, 0x73,
      0x21, 0x07, // EncryptedPayload (0x85, 0x07)
        0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74
  };
  Blob errorBlob5(encoding5, sizeof(encoding5));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob5), runtime_error) <<
    "wireDecode did not throw an exception";

  uint8_t encoding6[] = {
    0x82, 0x00 // Empty EncryptedContent
  };
  Blob errorBlob6(encoding6, sizeof(encoding6));
  ASSERT_THROW(encryptedContent.wireDecode(errorBlob6), runtime_error) <<
    "wireDecode did not throw an exception";
}

TEST_F(TestEncyptedContent, SetterGetter)
{
  EncryptedContent content;
  ASSERT_EQ(content.getAlgorithmType(), (ndn_EncryptAlgorithmType)-1);
  ASSERT_EQ(content.getPayload().isNull(), true);
  ASSERT_EQ(content.getInitialVector().isNull(), true);
  ASSERT_EQ(content.getKeyLocator().getType(), (ndn_KeyLocatorType)-1);

  content.setAlgorithmType(ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_EQ(content.getAlgorithmType(), ndn_EncryptAlgorithmType_RsaOaep);
  ASSERT_EQ(content.getPayload().isNull(), true);
  ASSERT_EQ(content.getInitialVector().isNull(), true);
  ASSERT_EQ(content.getKeyLocator().getType(), (ndn_KeyLocatorType)-1);

  KeyLocator keyLocator;
  keyLocator.setType(ndn_KeyLocatorType_KEYNAME);
  keyLocator.getKeyName().set("/test/key/locator");
  content.setKeyLocator(keyLocator);
  ASSERT_TRUE(content.getKeyLocator().getType() != (ndn_KeyLocatorType)-1);
  ASSERT_TRUE(content.getKeyLocator().getKeyName().equals
              (Name("/test/key/locator")));
  ASSERT_EQ(content.getPayload().isNull(), true);
  ASSERT_EQ(content.getInitialVector().isNull(), true);

  Blob payload(MESSAGE, sizeof(MESSAGE));
  content.setPayload(payload);

  Blob contentPayload = content.getPayload();
  ASSERT_TRUE(contentPayload.equals(payload));

  Blob initialVector(IV, sizeof(IV));
  content.setInitialVector(initialVector);

  Blob contentInitialVector = content.getInitialVector();
  ASSERT_TRUE(contentInitialVector.equals(initialVector));

  Blob encoded = content.wireEncode();
  Blob contentBlob(ENCRYPTED, sizeof(ENCRYPTED));

  ASSERT_TRUE(contentBlob.equals(encoded));
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

