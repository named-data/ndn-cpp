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

#include <ndn-cpp/common.hpp>
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

}

