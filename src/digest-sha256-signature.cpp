/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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
#include <ndn-cpp/digest-sha256-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
DigestSha256Signature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new DigestSha256Signature(*this));
}

const Blob&
DigestSha256Signature::getSignature() const
{
  return signature_;
}

void
DigestSha256Signature::setSignature(const Blob& signature)
{
  signature_ = signature;
  ++changeCount_;
}

void
DigestSha256Signature::get(struct ndn_Signature& signatureStruct) const
{
  // Initialize unused fields.
  ndn_Signature_clear(&signatureStruct);

  signatureStruct.type = ndn_SignatureType_DigestSha256Signature;
  signature_.get(signatureStruct.signature);
}

void
DigestSha256Signature::set(const struct ndn_Signature& signatureStruct)
{
  // The caller should already have checked the type, but check again.
  if (signatureStruct.type != ndn_SignatureType_DigestSha256Signature)
    throw runtime_error("signatureStruct is not the expected type DigestSha256Signature");

  setSignature(Blob(signatureStruct.signature));
}

}
