/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
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
  setDigestAlgorithm(Blob(signatureStruct.digestAlgorithm));
  setWitness(Blob(signatureStruct.witness));
  setSignature(Blob(signatureStruct.signature));
  publisherPublicKeyDigest_.get().set(signatureStruct.publisherPublicKeyDigest);
  keyLocator_.get().set(signatureStruct.keyLocator);
}

}
