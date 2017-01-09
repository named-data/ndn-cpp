/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2017 Regents of the University of California.
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

#include <stdexcept>
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

  size_t keyIdx = 0;
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
  string idString("ID-CERT");
  bool foundIdString = false;
  size_t idCertComponentIndex = certificateName.size() - 1;
  for (; idCertComponentIndex + 1 > 0; --idCertComponentIndex) {
    if (certificateName.get(idCertComponentIndex).toEscapedString() == idString) {
      foundIdString = true;
      break;
    }
  }

  if (!foundIdString)
    throw runtime_error
      ("Incorrect identity certificate name " + certificateName.toUri());

  Name tempName = certificateName.getSubName(0, idCertComponentIndex);
  string keyString("KEY");
  bool foundKeyString = false;
  size_t keyComponentIndex = 0;
  for (; keyComponentIndex < tempName.size(); keyComponentIndex++) {
    if (tempName.get(keyComponentIndex).toEscapedString() == keyString) {
      foundKeyString = true;
      break;
    }
  }

  if (!foundKeyString)
    throw runtime_error
      ("Incorrect identity certificate name " + certificateName.toUri());

  return tempName
    .getSubName(0, keyComponentIndex)
    .append(tempName.getSubName
            (keyComponentIndex + 1, tempName.size() - keyComponentIndex - 1));
}

void
IdentityCertificate::wireDecode(const Blob& input, WireFormat& wireFormat)
{
  Certificate::wireDecode(input, wireFormat);
  setPublicKeyName();
}

}
