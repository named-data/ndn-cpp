/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/src/security/signing-info.cpp
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

#include <stdexcept>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/security/signing-info.hpp>

using namespace std;

namespace ndn {

SigningInfo::SigningInfo(const string& signingString)
{
  reset(SIGNER_TYPE_NULL);
  digestAlgorithm_ = DIGEST_ALGORITHM_SHA256;

  if (signingString == "")
    return;

  size_t iColon = signingString.find(':');
  if (iColon == string::npos)
    throw invalid_argument("Invalid signing string cannot represent SigningInfo");

  string scheme = signingString.substr(0, iColon);
  string nameArg = signingString.substr(iColon + 1);

  if (scheme == "id") {
    if (nameArg == getDigestSha256Identity().toUri())
      setSha256Signing();
    else
      setSigningIdentity(nameArg);
  }
  else if (scheme == "key")
    setSigningKeyName(nameArg);
  else if (scheme == "cert")
    setSigningCertificateName(nameArg);
  else
    throw invalid_argument("Invalid signing string scheme");
}

const ptr_lib::shared_ptr<PibIdentity>&
SigningInfo::getPibIdentity() const
{
  if (type_ != SIGNER_TYPE_ID)
    throw runtime_error("getPibIdentity: The signer type is not SIGNER_TYPE_ID");
  return identity_;
}

const ptr_lib::shared_ptr<PibKey>&
SigningInfo::getPibKey() const
{
  if (type_ != SIGNER_TYPE_KEY)
    throw runtime_error("getPibKey: The signer type is not SIGNER_TYPE_KEY");
  return key_;
}

string
SigningInfo::toString() const
{
  if (type_ == SigningInfo::SIGNER_TYPE_NULL)
    return "";
  else if (type_ == SigningInfo::SIGNER_TYPE_ID)
    return "id:" + getSignerName().toUri();
  else if (type_ == SigningInfo::SIGNER_TYPE_KEY)
    return "key:" + getSignerName().toUri();
  else if (type_ == SigningInfo::SIGNER_TYPE_CERT)
    return "cert:" + getSignerName().toUri();
  else if (type_ == SigningInfo::SIGNER_TYPE_SHA256)
    return "id:" + getDigestSha256Identity().toUri();
  else
    // We don't expect this to happen.
    throw invalid_argument("Unknown signer type");
}

void
SigningInfo::reset(SignerType signerType)
{
  if (!(signerType == SIGNER_TYPE_NULL ||
        signerType == SIGNER_TYPE_ID ||
        signerType == SIGNER_TYPE_KEY ||
        signerType == SIGNER_TYPE_CERT ||
        signerType == SIGNER_TYPE_SHA256))
    throw runtime_error("SigningInfo: The signerType is not valid");

  type_ = signerType;
  name_.clear();
  identity_.reset();
  key_.reset();
  validityPeriod_.clear();
}

}
