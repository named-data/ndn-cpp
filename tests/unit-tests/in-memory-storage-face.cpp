/**
 * Copyright (C) 2018-2020 Regents of the University of California.
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

#include "in-memory-storage-face.hpp"

using namespace std;
using namespace ndn;

uint64_t
InMemoryStorageFace::expressInterest
  (const Interest& interest, const OnData& onData,
   const OnTimeout& onTimeout, const OnNetworkNack& onNetworkNack,
   WireFormat& wireFormat)
{
  // Make a copy of the interest.
  sentInterests_.push_back(ptr_lib::make_shared<Interest>(interest));

  ptr_lib::shared_ptr<Data> data = storage_->find(interest);
  if (data) {
    sentData_.push_back(ptr_lib::make_shared<Data>(*data));
    onData(ptr_lib::make_shared<Interest>(interest), data);
  }
  else
    onTimeout(ptr_lib::make_shared<Interest>(interest));

  return 0;
}

uint64_t
InMemoryStorageFace::registerPrefix
  (const Name& prefix, const OnInterestCallback& onInterest,
   const OnRegisterFailed& onRegisterFailed,
   const OnRegisterSuccess& onRegisterSuccess,
   const RegistrationOptions& registrationOptions, WireFormat& wireFormat)
{
  interestFilterTable_.setInterestFilter
    (0, ptr_lib::make_shared<InterestFilter>(prefix), onInterest, this);

  if (onRegisterSuccess)
    onRegisterSuccess(ptr_lib::make_shared<Name>(prefix), 0);
  return 0;
}

void
InMemoryStorageFace::putData(const Data& data, WireFormat& wireFormat)
{
  sentData_.push_back(ptr_lib::make_shared<Data>(data));
}

void
InMemoryStorageFace::callLater(Milliseconds delayMilliseconds, const Face::Callback& callback)
{
  delayedCallTable_.callLater(delayMilliseconds, callback);
}

void
InMemoryStorageFace::processEvents()
{
  delayedCallTable_.callTimedOut();
}

void
InMemoryStorageFace::receive(const ptr_lib::shared_ptr<Interest> interest)
{
  vector<ptr_lib::shared_ptr<InterestFilterTable::Entry> > matchedFilters;
  interestFilterTable_.getMatchedFilters(*interest, matchedFilters);
  for (size_t i = 0; i < matchedFilters.size(); ++i) {
    InterestFilterTable::Entry &entry = *matchedFilters[i];
    entry.getOnInterest()
      (entry.getPrefix(), interest, entry.getFace(),
       entry.getInterestFilterId(), entry.getFilter());
  }
}
