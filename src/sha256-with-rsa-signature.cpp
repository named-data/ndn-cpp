/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#include "c/data.h"
#include <ndn-cpp/sha256-with-rsa-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature> 
Sha256WithRsaSignature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new Sha256WithRsaSignature(*this));
}

void 
Sha256WithRsaSignature::get(struct ndn_Signature& signatureStruct) const 
{
  digestAlgorithm_.get(signatureStruct.digestAlgorithm);
  witness_.get(signatureStruct.witness);
  signature_.get(signatureStruct.signature);  
  publisherPublicKeyDigest_.get().get(signatureStruct.publisherPublicKeyDigest);
  keyLocator_.get().get(signatureStruct.keyLocator);
}

void 
Sha256WithRsaSignature::set(const struct ndn_Signature& signatureStruct)
{
  digestAlgorithm_ = Blob(signatureStruct.digestAlgorithm);
  witness_ = Blob(signatureStruct.witness);
  setSignature(Blob(signatureStruct.signature));
  publisherPublicKeyDigest_.get().set(signatureStruct.publisherPublicKeyDigest);
  keyLocator_.get().set(signatureStruct.keyLocator);
}

}
