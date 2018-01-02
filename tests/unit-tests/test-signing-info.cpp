/**
 * Copyright (C) 2014-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/signing-info.t.cpp
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
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/security/signing-info.hpp>

using namespace std;
using namespace ndn;

class TestSigningInfo : public ::testing::Test {
};

TEST_F(TestSigningInfo, Basic)
{
  Name identityName("/my-identity");
  Name keyName("/my-key");
  Name certificateName("/my-cert");

  SigningInfo info;

  ASSERT_EQ(SigningInfo::SIGNER_TYPE_NULL, info.getSignerType());
  ASSERT_EQ(Name(), info.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, info.getDigestAlgorithm());

  info.setSigningIdentity(identityName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_ID, info.getSignerType());
  ASSERT_EQ(identityName, info.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, info.getDigestAlgorithm());

  SigningInfo infoId(SigningInfo::SIGNER_TYPE_ID, identityName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_ID, infoId.getSignerType());
  ASSERT_EQ(identityName, infoId.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoId.getDigestAlgorithm());

  info.setSigningKeyName(keyName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_KEY, info.getSignerType());
  ASSERT_EQ(keyName, info.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, info.getDigestAlgorithm());

  SigningInfo infoKey(SigningInfo::SIGNER_TYPE_KEY, keyName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_KEY, infoKey.getSignerType());
  ASSERT_EQ(keyName, infoKey.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoKey.getDigestAlgorithm());

  info.setSigningCertificateName(certificateName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_CERT, info.getSignerType());
  ASSERT_EQ(certificateName, info.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, info.getDigestAlgorithm());

  SigningInfo infoCert(SigningInfo::SIGNER_TYPE_CERT, certificateName);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_CERT, infoCert.getSignerType());
  ASSERT_EQ(certificateName, infoCert.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoCert.getDigestAlgorithm());

  info.setSha256Signing();
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_SHA256, info.getSignerType());
  ASSERT_EQ(Name(), info.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, info.getDigestAlgorithm());

  SigningInfo infoSha256(SigningInfo::SIGNER_TYPE_SHA256);
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_SHA256, infoSha256.getSignerType());
  ASSERT_EQ(Name(), infoSha256.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoSha256.getDigestAlgorithm());
}

TEST_F(TestSigningInfo, FromString)
{
  SigningInfo infoDefault("");
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_NULL, infoDefault.getSignerType());
  ASSERT_EQ(Name(), infoDefault.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoDefault.getDigestAlgorithm());

  SigningInfo infoId("id:/my-identity");
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_ID, infoId.getSignerType());
  ASSERT_EQ(Name("/my-identity"), infoId.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoId.getDigestAlgorithm());

  SigningInfo infoKey("key:/my-key");
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_KEY, infoKey.getSignerType());
  ASSERT_EQ(Name("/my-key"), infoKey.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoKey.getDigestAlgorithm());

  SigningInfo infoCert("cert:/my-cert");
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_CERT, infoCert.getSignerType());
  ASSERT_EQ(Name("/my-cert"), infoCert.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoCert.getDigestAlgorithm());

  SigningInfo infoSha("id:/localhost/identity/digest-sha256");
  ASSERT_EQ(SigningInfo::SIGNER_TYPE_SHA256, infoSha.getSignerType());
  ASSERT_EQ(Name(), infoSha.getSignerName());
  ASSERT_EQ(DIGEST_ALGORITHM_SHA256, infoSha.getDigestAlgorithm());
}

TEST_F(TestSigningInfo, ToString)
{
  ASSERT_EQ("", SigningInfo().toString());

  ASSERT_EQ("id:/my-identity",
            SigningInfo(SigningInfo::SIGNER_TYPE_ID, "/my-identity").toString());
  ASSERT_EQ("key:/my-key",
            SigningInfo(SigningInfo::SIGNER_TYPE_KEY, "/my-key").toString());
  ASSERT_EQ("cert:/my-cert",
            SigningInfo(SigningInfo::SIGNER_TYPE_CERT, "/my-cert").toString());
  ASSERT_EQ("id:/localhost/identity/digest-sha256",
            SigningInfo(SigningInfo::SIGNER_TYPE_SHA256).toString());
}

TEST_F(TestSigningInfo, Chaining)
{
  SigningInfo info = SigningInfo()
    .setSigningIdentity("/identity")
    .setSigningKeyName("/key/name")
    .setSigningCertificateName("/cert/name")
    .setPibIdentity(ptr_lib::shared_ptr<PibIdentity>())
    .setPibKey(ptr_lib::shared_ptr<PibKey>())
    .setSha256Signing()
    .setDigestAlgorithm(DIGEST_ALGORITHM_SHA256);

  ASSERT_EQ("id:/localhost/identity/digest-sha256", info.toString());
}

int
main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
