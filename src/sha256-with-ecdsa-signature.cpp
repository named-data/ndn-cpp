/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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

#include "c/data.h"
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
Sha256WithEcdsaSignature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new Sha256WithEcdsaSignature(*this));
}

void
Sha256WithEcdsaSignature::get(struct ndn_Signature& signatureStruct) const
{
  // Initialize unused fields.
  ndn_Blob_initialize(&signatureStruct.digestAlgorithm, 0, 0);
  ndn_Blob_initialize(&signatureStruct.witness, 0, 0);
  ndn_PublisherPublicKeyDigest_initialize(&signatureStruct.publisherPublicKeyDigest);

  signature_.get(signatureStruct.signature);
  keyLocator_.get().get(signatureStruct.keyLocator);
}

void
Sha256WithEcdsaSignature::set(const struct ndn_Signature& signatureStruct)
{
  setSignature(Blob(signatureStruct.signature));
  keyLocator_.get().set(signatureStruct.keyLocator);
}

}
