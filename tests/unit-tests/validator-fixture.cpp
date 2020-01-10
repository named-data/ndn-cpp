/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/validator-fixture.cpp
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

#include "validator-fixture.hpp"

using namespace std;
using namespace ndn;
using namespace ndn::func_lib;

ValidatorFixture::ValidatorFixture
  (const ptr_lib::shared_ptr<ValidationPolicy>& policy)
: validator_(policy, ptr_lib::make_shared<CertificateFetcherFromNetwork>(face_)),
  policy_(policy),
  // Set maxLifetime to 100 days.
  cache_(100 * 24 * 3600 * 1000.0)
{
  face_.processInterest_ =
    bind(&ValidatorFixture::processInterestFromCache, this, _1, _2, _3, _4);
}

void
ValidatorFixture::processInterestFromCache
  (const ndn::Interest& interest, const ndn::OnData& onData,
   const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack)
{
  ptr_lib::shared_ptr<CertificateV2> certificate = cache_.find(interest);
  if (certificate)
    onData(ptr_lib::make_shared<Interest>(interest), certificate);
  else
    onTimeout(ptr_lib::make_shared<Interest>(interest));
}

uint64_t
ValidatorFixture::TestFace::expressInterest
  (const Interest& interest, const OnData& onData,
   const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
   WireFormat& wireFormat)
{
  // This makes a copy of the interest.
  sentInterests_.push_back(interest);

  if (processInterest_)
    processInterest_(interest, onData, onTimeout, onNetworkNack);
  else
    onTimeout(ptr_lib::make_shared<Interest>(interest));

  return 0;
}
