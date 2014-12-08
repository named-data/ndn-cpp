/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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

Name
IdentityStorage::getNewKeyName (const Name& identityName, bool useKsk)
{
  MillisecondsSince1970 ti = ::ndn_getNowMilliseconds();
  // Get the number of seconds.
  ostringstream oss;
  oss << (uint64_t)floor(ti / 1000.0);

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

}
