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
#include <ndn-cpp/common.hpp>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/key-locator.hpp>
#include "c/key-locator.h"

using namespace std;

namespace ndn {

void
KeyLocator::get(struct ndn_KeyLocator& keyLocatorStruct) const
{
  keyLocatorStruct.type = type_;
  keyData_.get(keyLocatorStruct.keyData);
  keyName_.get().get(keyLocatorStruct.keyName);
  keyLocatorStruct.keyNameType = keyNameType_;
}

void
KeyLocator::set(const struct ndn_KeyLocator& keyLocatorStruct)
{
  setType(keyLocatorStruct.type);
  setKeyData(Blob(keyLocatorStruct.keyData));
  if (keyLocatorStruct.type == ndn_KeyLocatorType_KEYNAME) {
    keyName_.get().set(keyLocatorStruct.keyName);
    keyNameType_ = keyLocatorStruct.keyNameType;
  }
  else {
    keyName_.get().clear();
    keyNameType_ = (ndn_KeyNameType)-1;
  }
}

bool
KeyLocator::canGetFromSignature(const Signature* signature)
{
  return dynamic_cast<const Sha256WithRsaSignature *>(signature) ||
         dynamic_cast<const Sha256WithEcdsaSignature *>(signature);
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

  throw runtime_error
    ("KeyLocator::getFromSignature: Signature type does not have a KeyLocator");
}

}

