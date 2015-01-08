/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Yingdi Yu <yingdi@cs.ucla.edu>
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

#include <math.h>
#include "../../c/util/time.h"
#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/identity/identity-storage.hpp>

using namespace std;

namespace ndn {

IdentityStorage::IdentityStorage()
{
  if (!lastTimestampIsInitialized_) {
    lastTimestampIsInitialized_ = true;
    lastTimestamp_ = ::floor(ndn_getNowMilliseconds() / 1000.0);
  }
}

Name
IdentityStorage::getNewKeyName (const Name& identityName, bool useKsk)
{
  uint64_t timestamp = ::floor(ndn_getNowMilliseconds() / 1000.0);
  while (timestamp <= lastTimestamp_)
    // Make the timestamp unique.
    timestamp += 1;
  lastTimestamp_ = timestamp;

  // Get the number of seconds as a string.
  ostringstream oss;
  oss << timestamp;

  string keyIdStr;

  if (useKsk)
    keyIdStr = ("KSK-" + oss.str());
  else
    keyIdStr = ("DSK-" + oss.str());

  Name keyName = Name(identityName).append(keyIdStr);

  if (doesKeyExist(keyName))
    throw SecurityException("Key name already exists");

  return keyName;
}

Name
IdentityStorage::getDefaultCertificateNameForIdentity (const Name& identityName)
{
  Name keyName = getDefaultKeyNameForIdentity(identityName);
  return getDefaultCertificateNameForKey(keyName);
}

bool IdentityStorage::lastTimestampIsInitialized_ = false;
uint64_t IdentityStorage::lastTimestamp_;

}
