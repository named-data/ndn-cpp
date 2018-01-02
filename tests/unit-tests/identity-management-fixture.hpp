/**
 * Copyright (C) 2017-2018 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From ndn-cxx unit tests:
 * https://github.com/named-data/ndn-cxx/blob/master/tests/unit-tests/identity-management-fixture.hpp
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

#ifndef NDN_IDENTITY_MANAGEMENT_FIXTURE_HPP
#define NDN_IDENTITY_MANAGEMENT_FIXTURE_HPP

#include <set>
#include <ndn-cpp/security/key-chain.hpp>

class IdentityManagementFixture
{
public:
  IdentityManagementFixture()
  : keyChain_("pib-memory:", "tpm-memory:")
  {
  }

  ~IdentityManagementFixture();

  bool
  saveCertificateToFile(const ndn::Data& data, const std::string& filePath);

  /**
   * Add an identity for the identityName.
   * @param identityName The name of the identity.
   * @param params (optional) The key parameters if a key needs to be generated
   * for the identity. If omitted, use getDefaultKeyParams().
   * @return The created PibIdentity instance.
   */
  ndn::ptr_lib::shared_ptr<ndn::PibIdentity>
  addIdentity
    (const ndn::Name& identityName, 
     const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

  /**
   *  Save the identity's certificate to a file.
   *  @param identity The PibIdentity.
   *  @param filePath The file path, which should be writable.
   *  @return True if successful.
   */
  bool
  saveCertificate(ndn::PibIdentity identity, const std::string& filePath);

  /**
   * Issue a certificate for subIdentityName signed by issuer. If the identity
   * does not exist, it is created. A new key is generated as the default key
   * for the identity. A default certificate for the key is signed by the
   * issuer using its default certificate.
   * @param subIdentityName The name to issue the certificate for.
   * @param issuer The identity of the signer.
   * @param params (optional) The key parameters if a key needs to be generated
   * for the identity. If omitted, use getDefaultKeyParams().
   * @return The sub identity.
   */
  ndn::ptr_lib::shared_ptr<ndn::PibIdentity>
  addSubCertificate
    (const ndn::Name& subIdentityName, 
     const ndn::ptr_lib::shared_ptr<ndn::PibIdentity>& issuer,
     const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

  /**
   * Add a self-signed certificate made from the key and issuer ID.
   * @param key The key for the certificate.
   * @param issuerId The issuer ID name component for the certificate name.
   * @return The new certificate.
   */
  ndn::ptr_lib::shared_ptr<ndn::CertificateV2>
  addCertificate
    (ndn::ptr_lib::shared_ptr<ndn::PibKey>& key, const std::string& issuerId);

  ndn::KeyChain keyChain_;

private:
  std::set<ndn::Name> identityNames_;
  std::set<std::string> certificateFiles_;
};


#endif
