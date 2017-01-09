/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
Sha256WithEcdsaSignature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new Sha256WithEcdsaSignature(*this));
}

const Blob&
Sha256WithEcdsaSignature::getSignature() const
{
  return signature_;
}

void
Sha256WithEcdsaSignature::setSignature(const Blob& signature)
{
  signature_ = signature;
  ++changeCount_;
}

void
Sha256WithEcdsaSignature::get(SignatureLite& signatureLite) const
{
  signatureLite.setType(ndn_SignatureType_Sha256WithEcdsaSignature);
  signatureLite.setSignature(signature_);
  keyLocator_.get().get(signatureLite.getKeyLocator());
  validityPeriod_.get().get(signatureLite.getValidityPeriod());
}

void
Sha256WithEcdsaSignature::set(const SignatureLite& signatureLite)
{
  // The caller should already have checked the type, but check again.
  if (signatureLite.getType() != ndn_SignatureType_Sha256WithEcdsaSignature)
    throw runtime_error("signatureLite is not the expected type Sha256WithEcdsaSignature");

  setSignature(Blob(signatureLite.getSignature()));
  keyLocator_.get().set(signatureLite.getKeyLocator());
  validityPeriod_.get().set(signatureLite.getValidityPeriod());
}

uint64_t
Sha256WithEcdsaSignature::getChangeCount() const
{
  // Make sure each of the checkChanged is called.
  bool changed = keyLocator_.checkChanged();
  changed = validityPeriod_.checkChanged() || changed;
  if (changed)
    // A child object has changed, so update the change count.
    // This method can be called on a const object, but we want to be able to
    //   update the changeCount_.
    ++const_cast<Sha256WithEcdsaSignature*>(this)->changeCount_;

  return changeCount_;
}

}
