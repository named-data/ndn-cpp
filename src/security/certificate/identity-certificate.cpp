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

#include <ndn-cpp/security/security-exception.hpp>
#include <ndn-cpp/security/certificate/identity-certificate.hpp>

using namespace std;

namespace ndn {

IdentityCertificate::IdentityCertificate(const Data& data)
  : Certificate(data)
{
  if (!isCorrectName(data.getName()))
    throw SecurityException("Wrong Identity Certificate Name!");

  setPublicKeyName();
}

IdentityCertificate::IdentityCertificate(const IdentityCertificate& identityCertificate)
  : Certificate(identityCertificate)
  , publicKeyName_(identityCertificate.publicKeyName_)
{
}

IdentityCertificate::~IdentityCertificate()
{
}

bool
IdentityCertificate::isCorrectName(const Name& name)
{
  int i = name.size() - 1;

  string idString("ID-CERT");
  for (; i >= 0; i--) {
    if(name.get(i).toEscapedString() == idString)
      break;
  }

  if (i < 0)
    return false;

  int keyIdx = 0;
  string keyString("KEY");
  for (; keyIdx < name.size(); keyIdx++) {
    if(name.get(keyIdx).toEscapedString() == keyString)
      break;
  }

  if (keyIdx >= name.size())
    return false;

  return true;
}

Data&
IdentityCertificate::setName(const Name& name)
{
  if (!isCorrectName(name))
    throw SecurityException("Wrong Identity Certificate Name!");

  Data::setName(name);
  setPublicKeyName();
  return *this;
}

void
IdentityCertificate::setPublicKeyName()
{
  publicKeyName_ = certificateNameToPublicKeyName(getName());
}

bool
IdentityCertificate::isIdentityCertificate(const Certificate& certificate)
{
  return isCorrectName(certificate.getName());
}

Name
IdentityCertificate::certificateNameToPublicKeyName(const Name& certificateName)
{
  int i = certificateName.size() - 1;
  string idString("ID-CERT");
  for (; i >= 0; i--) {
    if (certificateName.get(i).toEscapedString() == idString)
      break;
  }

  Name tmpName = certificateName.getSubName(0, i);
  string keyString("KEY");
  for (i = 0; i < tmpName.size(); i++) {
    if (tmpName.get(i).toEscapedString() == keyString)
      break;
  }

  return tmpName.getSubName(0, i).append(tmpName.getSubName(i + 1, tmpName.size() - i - 1));
}

void
IdentityCertificate::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  Certificate::wireDecode(input, wireFormat);
  setPublicKeyName();
}

}
