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
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/hmac-with-sha256-signature.hpp>
#include <ndn-cpp/key-locator.hpp>

using namespace std;

namespace ndn {

void
KeyLocator::get(KeyLocatorLite& keyLocatorLite) const
{
  keyLocatorLite.setType(type_);
  keyLocatorLite.setKeyData(keyData_);
  keyName_.get().get(keyLocatorLite.getKeyName());
}

void
KeyLocator::set(const KeyLocatorLite& keyLocatorLite)
{
  setType(keyLocatorLite.getType());
  setKeyData(Blob(keyLocatorLite.getKeyData()));
  if (keyLocatorLite.getType() == ndn_KeyLocatorType_KEYNAME)
    keyName_.get().set(keyLocatorLite.getKeyName());
  else
    keyName_.get().clear();
}

bool
KeyLocator::canGetFromSignature(const Signature* signature)
{
  return dynamic_cast<const Sha256WithRsaSignature *>(signature) ||
         dynamic_cast<const Sha256WithEcdsaSignature *>(signature) ||
         dynamic_cast<const HmacWithSha256Signature *>(signature);
}

const KeyLocator&
KeyLocator::getFromSignature(const Signature* signature)
{
  {
    const Sha256WithRsaSignature *castSignature =
      dynamic_cast<const Sha256WithRsaSignature *>(signature);
    if (castSignature)
      return castSignature->getKeyLocator();
  }
  {
    const Sha256WithEcdsaSignature *castSignature =
      dynamic_cast<const Sha256WithEcdsaSignature *>(signature);
    if (castSignature)
      return castSignature->getKeyLocator();
  }
  {
    const HmacWithSha256Signature *castSignature =
      dynamic_cast<const HmacWithSha256Signature *>(signature);
    if (castSignature)
      return castSignature->getKeyLocator();
  }

  throw runtime_error
    ("KeyLocator::getFromSignature: Signature type does not have a KeyLocator");
}

bool
KeyLocator::equals(const KeyLocator& other) const
{
  if (type_ != other.type_)
    return false;

  if (type_ == ndn_KeyLocatorType_KEYNAME) {
    if (!getKeyName().equals(other.getKeyName()))
      return false;
  }
  else if (type_ == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST) {
    if (!getKeyData().equals(other.getKeyData()))
      return false;
  }

  return true;
}

}

