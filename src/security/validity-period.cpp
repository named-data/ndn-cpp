/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx src/security https://github.com/named-data/ndn-cxx
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
#include <stdexcept>
#include <ndn-cpp/sha256-with-ecdsa-signature.hpp>
#include <ndn-cpp/sha256-with-rsa-signature.hpp>
#include <ndn-cpp/security/validity-period.hpp>

using namespace std;

namespace ndn {

bool
ValidityPeriod::isValid(MillisecondsSince1970 time) const
{
  if (time < 0.0)
    // Round up to the nearest second like in setPeriod.
    time = round(ceil(round(ndn_getNowMilliseconds()) / 1000.0) * 1000.0);

  return validityPeriod_.isValid(time);
}
  
bool
ValidityPeriod::canGetFromSignature(const Signature* signature)
{
  return dynamic_cast<const Sha256WithRsaSignature *>(signature) ||
         dynamic_cast<const Sha256WithEcdsaSignature *>(signature);
}

ValidityPeriod&
ValidityPeriod::getFromSignature(Signature* signature)
{
  {
    Sha256WithRsaSignature *castSignature =
      dynamic_cast<Sha256WithRsaSignature *>(signature);
    if (castSignature)
      return castSignature->getValidityPeriod();
  }
  {
    Sha256WithEcdsaSignature *castSignature =
      dynamic_cast<Sha256WithEcdsaSignature *>(signature);
    if (castSignature)
      return castSignature->getValidityPeriod();
  }

  throw runtime_error
    ("ValidityPeriod::getFromSignature: Signature type does not have a ValidityPeriod");
}


}
