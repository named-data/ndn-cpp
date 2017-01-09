/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2016-2017 Regents of the University of California.
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
#include <ndn-cpp/generic-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
GenericSignature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new GenericSignature(*this));
}

const Blob&
GenericSignature::getSignature() const
{
  return signature_;
}

void
GenericSignature::setSignature(const Blob& signature)
{
  signature_ = signature;
  ++changeCount_;
}

void
GenericSignature::get(SignatureLite& signatureLite) const
{
  // Initialize unused fields.
  signatureLite.clear();

  signatureLite.setType(ndn_SignatureType_Generic);
  signatureLite.setSignature(signature_);
  signatureLite.setSignatureInfoEncoding(signatureInfoEncoding_, typeCode_);
}

void
GenericSignature::set(const SignatureLite& signatureLite)
{
  // The caller should already have checked the type, but check again.
  if (signatureLite.getType() != ndn_SignatureType_Generic)
    throw runtime_error("signatureLite is not the expected type Generic");

  setSignature(Blob(signatureLite.getSignature()));
  setSignatureInfoEncoding
    (Blob(signatureLite.getSignatureInfoEncoding()),
     signatureLite.getGenericTypeCode());
}

uint64_t
GenericSignature::getChangeCount() const
{
  return changeCount_;
}

}
