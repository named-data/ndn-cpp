/**
 * Copyright (C) 2017 Regents of the University of California.
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
#include "identity-management-fixture.hpp"

class ValidatorFixture : public IdentityManagementFixture
{
public:
  ValidatorFixture
    (const ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy>& policy,
     const ndn::ptr_lib::shared_ptr<ndn::CertificateFetcher>& certificateFetcher)
  : validator_(policy, certificateFetcher),
    policy_(policy),
    // Set maxLifetime to 100 days.
    cache_(100 * 24 * 3600 * 1000.0)
  {
  }

  // TODO: face_
  // TODO: processInterest_
  ndn::Validator validator_;
  ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy> policy_;

  ndn::CertificateCacheV2 cache_;
};

class HierarchicalValidatorFixture : public ValidatorFixture {
public:
  HierarchicalValidatorFixture
    (const ndn::ptr_lib::shared_ptr<ndn::ValidationPolicy>& policy,
     const ndn::ptr_lib::shared_ptr<ndn::CertificateFetcher>& certificateFetcher)
  : ValidatorFixture(policy, certificateFetcher)
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
