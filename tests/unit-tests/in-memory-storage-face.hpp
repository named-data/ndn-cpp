/**
 * Copyright (C) 2018 Regents of the University of California.
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

#ifndef NDN_IN_MEMORY_STORAGE_FACE_HPP
#define NDN_IN_MEMORY_STORAGE_FACE_HPP

#include <ndn-cpp/face.hpp>
#include <ndn-cpp/in-memory-storage/in-memory-storage-retaining.hpp>
#include "../../src/impl/interest-filter-table.hpp"
#include "../../src/impl/delayed-call-table.hpp"

/**
 * InMemoryStorageFace extends Face to hold an InMemoryStorageRetaining and
 * use it in expressInterest to instantly reply to an Interest. It also allows
 * calls to registerPrefix to remember an OnInterestCallback. This also keeps a
 * local DelayedCallTable (to use for callLater) so that you can call its
 * setNowOffsetMilliseconds_ for testing.
 */
class InMemoryStorageFace : public ndn::Face
{
public:
  InMemoryStorageFace(ndn::InMemoryStorageRetaining* storage)
  : Face("localhost"), storage_(storage)
  {
  }

  virtual uint64_t
  expressInterest
    (const ndn::Interest& interest, const ndn::OnData& onData,
     const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack,
     ndn::WireFormat& wireFormat = *ndn::WireFormat::getDefaultWireFormat());

  virtual uint64_t
  registerPrefix
    (const ndn::Name& prefix, const ndn::OnInterestCallback& onInterest,
     const ndn::OnRegisterFailed& onRegisterFailed,
     const ndn::OnRegisterSuccess& onRegisterSuccess,
     const ndn::ForwardingFlags& flags = ndn::ForwardingFlags(),
     ndn::WireFormat& wireFormat = *ndn::WireFormat::getDefaultWireFormat());

  virtual void
  putData
    (const ndn::Data& data,
     ndn::WireFormat& wireFormat = *ndn::WireFormat::getDefaultWireFormat());

  virtual void
  callLater
    (ndn::Milliseconds delayMilliseconds, const ndn::Face::Callback& callback);

  virtual void
  processEvents();

  /**
   * For each entry from calls to registerPrefix where the Interest matches the
   * prefix, call its OnInterest callback.
   * @param interest The Interest to receive and possibly call the
   * OnInterest callback.
   */
  void
  receive(const ndn::ptr_lib::shared_ptr<ndn::Interest> interest);

  std::vector<ndn::ptr_lib::shared_ptr<ndn::Interest> > sentInterests_;
  std::vector<ndn::ptr_lib::shared_ptr<ndn::Data> > sentData_;
  // Use delayedCallTable_ here so that we can call setNowOffsetMilliseconds_().
  ndn::DelayedCallTable delayedCallTable_;

private:
  ndn::InterestFilterTable interestFilterTable_;
  ndn::InMemoryStorageRetaining* storage_;
};

#endif
