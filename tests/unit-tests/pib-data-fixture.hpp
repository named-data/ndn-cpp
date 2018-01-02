/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN unit-tests by Adeola Bannis.
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/security/pib/pib-data-fixture.hpp
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

#ifndef NDN_PIB_DATA_FIXTURE_HPP
#define NDN_PIB_DATA_FIXTURE_HPP

#include <ndn-cpp/security/pib/pib-impl.hpp>

class PibDataFixture
{
public:
  PibDataFixture();

  ndn::PibImpl *pib;

  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id1Key1Cert1;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id1Key1Cert2;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id1Key2Cert1;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id1Key2Cert2;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id2Key1Cert1;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id2Key1Cert2;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id2Key2Cert1;
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2> id2Key2Cert2;

  ndn::Name id1;
  ndn::Name id2;

  ndn::Name id1Key1Name;
  ndn::Name id1Key2Name;
  ndn::Name id2Key1Name;
  ndn::Name id2Key2Name;

  ndn::Blob id1Key1;
  ndn::Blob id1Key2;
  ndn::Blob id2Key1;
  ndn::Blob id2Key2;
};


#endif
