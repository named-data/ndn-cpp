/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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
#include "c/data.h"
#include <ndn-cpp/sha256-with-rsa-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
Sha256WithRsaSignature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new Sha256WithRsaSignature(*this));
}

const Blob&
Sha256WithRsaSignature::getDigestAlgorithm() const
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("The Digest Algorithm is for the NDNx wire format and is deprecated. To enable while you upgrade your code to use NDN-TLV, set WireFormat::ENABLE_NDNX = true");

  return digestAlgorithm_;
}

const Blob&
Sha256WithRsaSignature::getWitness() const
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("The Witness is for the NDNx wire format and is deprecated. To enable while you upgrade your code to use NDN-TLV, set WireFormat::ENABLE_NDNX = true");

  return witness_;
}

const Blob&
Sha256WithRsaSignature::getSignature() const
{
  return signature_;
}

void
Sha256WithRsaSignature::setDigestAlgorithm(const Blob& digestAlgorithm)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("The Digest Algorithm is for the NDNx wire format and is deprecated. To enable while you upgrade your code to use NDN-TLV, set WireFormat::ENABLE_NDNX = true");

  digestAlgorithm_ = digestAlgorithm;
  ++changeCount_;
}

void
Sha256WithRsaSignature::setWitness(const Blob& witness)
{
  if (!WireFormat::ENABLE_NDNX)
    throw runtime_error
      ("The Witness is for the NDNx wire format and is deprecated. To enable while you upgrade your code to use NDN-TLV, set WireFormat::ENABLE_NDNX = true");

  witness_ = witness;
  ++changeCount_;
}

void
Sha256WithRsaSignature::setSignature(const Blob& signature)
{
  signature_ = signature;
  ++changeCount_;
}

void
Sha256WithRsaSignature::get(struct ndn_Signature& signatureStruct) const
{
  signatureStruct.type = ndn_SignatureType_Sha256WithRsaSignature;
  digestAlgorithm_.get(signatureStruct.digestAlgorithm);
  witness_.get(signatureStruct.witness);
  signature_.get(signatureStruct.signature);
  publisherPublicKeyDigest_.get().get(signatureStruct.publisherPublicKeyDigest);
  keyLocator_.get().get(signatureStruct.keyLocator);
}

void
Sha256WithRsaSignature::set(const struct ndn_Signature& signatureStruct)
{
  // The caller should already have checked the type, but check again.
  if (signatureStruct.type != ndn_SignatureType_Sha256WithRsaSignature)
    throw runtime_error("signatureStruct is not the expected type Sha256WithRsaSignature");

  digestAlgorithm_ = Blob(signatureStruct.digestAlgorithm);
  witness_ = Blob(signatureStruct.witness);
  setSignature(Blob(signatureStruct.signature));
  publisherPublicKeyDigest_.get().set(signatureStruct.publisherPublicKeyDigest);
  keyLocator_.get().set(signatureStruct.keyLocator);
}

}
