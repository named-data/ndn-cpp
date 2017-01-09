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
#include <ndn-cpp/hmac-with-sha256-signature.hpp>

using namespace std;

namespace ndn {

ptr_lib::shared_ptr<Signature>
HmacWithSha256Signature::clone() const
{
  return ptr_lib::shared_ptr<Signature>(new HmacWithSha256Signature(*this));
}

const Blob&
HmacWithSha256Signature::getSignature() const
{
  return signature_;
}

void
HmacWithSha256Signature::setSignature(const Blob& signature)
{
  signature_ = signature;
  ++changeCount_;
}

void
HmacWithSha256Signature::get(SignatureLite& signatureLite) const
{
  signatureLite.setType(ndn_SignatureType_HmacWithSha256Signature);
  signatureLite.setSignature(signature_);
  keyLocator_.get().get(signatureLite.getKeyLocator());
}

void
HmacWithSha256Signature::set(const SignatureLite& signatureLite)
{
  // The caller should already have checked the type, but check again.
  if (signatureLite.getType() != ndn_SignatureType_HmacWithSha256Signature)
    throw runtime_error("signatureLite is not the expected type HmacWithSha256Signature");

  setSignature(Blob(signatureLite.getSignature()));
  keyLocator_.get().set(signatureLite.getKeyLocator());
}

uint64_t
HmacWithSha256Signature::getChangeCount() const
{
  // Make sure each of the checkChanged is called.
  bool changed = keyLocator_.checkChanged();
  if (changed)
    // A child object has changed, so update the change count.
    // This method can be called on a const object, but we want to be able to
    //   update the changeCount_.
    ++const_cast<HmacWithSha256Signature*>(this)->changeCount_;

  return changeCount_;
}

}
