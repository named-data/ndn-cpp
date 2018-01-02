/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/validator-fixture.hpp
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

#ifndef NDN_VALIDATOR_FIXTURE_HPP
#define NDN_VALIDATOR_FIXTURE_HPP

#include <ndn-cpp/security/v2/validator.hpp>
#include <ndn-cpp/security/v2/certificate-fetcher-from-network.hpp>
#include "identity-management-fixture.hpp"

/**
 * ValidatorFixture extends IdentityManagementFixture to use the given policy
 * and to set up a test face to answer Interests.
 */
class ValidatorFixture : public IdentityManagementFixture
{
public:
  /**
   * Create a ValidatorFixture to use the given policy. Set the default
   * face_.processInterest_ to use the cache_ to respond to expressInterest. To
   * change this behavior, you can set face_.processInterest_ to your callback,
   * or to null to always time out.
   * @param policy The ValidationPolicy used by validator_.
   */
  ValidatorFixture(const ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy>& policy);

  /**
   * TestFace extends Face to instantly simulate a call to expressInterest.
   * See expressInterest for details.
   */
  class TestFace : public ndn::Face {
  public:
    typedef ndn::func_lib::function<void
      (const ndn::Interest& interest, const ndn::OnData& onData,
       const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack)>
      ProcessInterest;

    TestFace()
    : Face("localhost")
    {}

    /**
     * If processInterest_ is not null, call
     * processInterest_(interest, onData, onTimeout, onNetworkNack) which must
     * call one of the callbacks to simulate the response. Otherwise, just call
     * onTimeout(interest) to simulate a timeout. This adds the interest to
     * sentInterests_ . 
     */
    virtual uint64_t
    expressInterest
      (const ndn::Interest& interest, const ndn::OnData& onData,
       const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack,
       ndn::WireFormat& wireFormat = *ndn::WireFormat::getDefaultWireFormat());

    ProcessInterest processInterest_;
    std::vector<ndn::Interest> sentInterests_;
  };

  void
  processInterestFromCache
    (const ndn::Interest& interest, const ndn::OnData& onData,
     const ndn::OnTimeout& onTimeout, const ndn::OnNetworkNack& onNetworkNack);

  TestFace face_;
  ndn::Validator validator_;
  ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy> policy_;
  ndn::CertificateCacheV2 cache_;
};

class HierarchicalValidatorFixture : public ValidatorFixture {
public:
  HierarchicalValidatorFixture
    (const ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy>& policy)
  : ValidatorFixture(policy)
  {
    identity_ = addIdentity("/Security/V2/ValidatorFixture");
    subIdentity_ = addSubCertificate("/Security/V2/ValidatorFixture/Sub1", identity_);
    subSelfSignedIdentity_ = addIdentity("/Security/V2/ValidatorFixture/Sub1/Sub2");
    otherIdentity_ = addIdentity("/Security/V2/OtherIdentity");

    validator_.loadAnchor
      ("", ndn::CertificateV2(*identity_->getDefaultKey()->getDefaultCertificate()));

    cache_.insert(*identity_->getDefaultKey()->getDefaultCertificate());
    cache_.insert(*subIdentity_->getDefaultKey()->getDefaultCertificate());
    cache_.insert(*subSelfSignedIdentity_->getDefaultKey()->getDefaultCertificate());
    cache_.insert(*otherIdentity_->getDefaultKey()->getDefaultCertificate());
  }

  ndn::ptr_lib::shared_ptr<ndn::PibIdentity> identity_;
  ndn::ptr_lib::shared_ptr<ndn::PibIdentity> subIdentity_;
  ndn::ptr_lib::shared_ptr<ndn::PibIdentity> subSelfSignedIdentity_;
  ndn::ptr_lib::shared_ptr<ndn::PibIdentity> otherIdentity_;
};

#endif
