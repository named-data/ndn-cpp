/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/pib/pib.hpp
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

#ifndef NDN_PIB_HPP
#define NDN_PIB_HPP

#include <stdexcept>
#include "pib-identity-container.hpp"

// Give friend access to the tests.
class TestKeyChain_Management_Test;

namespace ndn {

class PibImpl;

/**
 * In general, a PIB (Public Information Base) stores the public portion of a
 * user's cryptography keys. The format and location of stored information is
 * indicated by the PIB locator. A PIB is designed to work with a TPM (Trusted
 * Platform Module) which stores private keys. There is a one-to-one association
 * between a PIB and a TPM, and therefore the TPM locator is recorded by the PIB
 * to enforce this association and prevent one from operating on mismatched PIB
 * and TPM.
 *
 * Information in the PIB is organized in a hierarchy of
 * Identity-Key-Certificate. At the top level, this Pib class provides access to
 * identities, and allows setting a default identity. Properties of an identity
 * (such as PibKey objects) can be accessed after obtaining a PibIdentity object.
 * (Likewise, CertificateV2 objects can be obtained from a PibKey object.)
 *
 * Note: A Pib instance is created and managed only by the KeyChain, and is
 * returned by the KeyChain getPib() method.
 */
class Pib {
public:
  /**
   * A Pib::Error extends runtime_error and represents a semantic error in PIB
   * processing.
   */
  class Error : public std::runtime_error
  {
  public:
    Error(const std::string& what)
    : std::runtime_error(what)
    {
    }
  };

  /**
   * Get the scheme of the PIB locator.
   * @return The scheme string.
   */
  std::string
  getScheme() const { return scheme_; }

  /**
   * Get the PIB locator.
   * @return The PIB locator.
   */
  std::string
  getPibLocator() const {return scheme_ + ":" + location_; }

  /**
   * Set the corresponding TPM information to tpmLocator.
   * If the tpmLocator is different from the existing one, the PIB will be reset.
   * Otherwise, nothing will be changed.
   */
  void
  setTpmLocator(const std::string& tpmLocator);

  /**
   * Get the TPM Locator.
   * @throws Pib::Error if the TPM locator is empty.
   */
  std::string
  getTpmLocator();

  /**
   * Get the identity with name identityName.
   * @param identityName The name of the identity.
   * @return The PibIdentity object.
   * @throws Pib::Error if the identity does not exist.
   */
  ptr_lib::shared_ptr<PibIdentity>
  getIdentity(const Name& identityName);

  /**
   * Get the default identity.
   * @return The PibIdentity object.
   * @throws Pib::Error if there is no default identity.
   */
  ptr_lib::shared_ptr<PibIdentity>&
  getDefaultIdentity();

  /**
   * Append all the identity names to the nameList.
   * @param nameList Append a copy of each name to nameList.
   */
  void
  getAllIdentityNames(std::vector<Name>& nameList);

private:
  friend class KeyChain;
  // Give friend access to the tests.
  friend class ::TestKeyChain_Management_Test;

  /*
   * Create a Pib instance. This constructor should only be called by KeyChain.
   * @param scheme The scheme for the PIB.
   * @param location The location for the PIB.
   * @param pibImpl The PIB backend implementation.
   */
  Pib(const std::string& scheme, const std::string& location,
      const ptr_lib::shared_ptr<PibImpl>& pibImpl);

  /**
   * Reset the content in the PIB, including a reset of the TPM locator.
   */
  void
  reset();

  /**
   * Add an identity with name identityName. Create the identity if it does not
   * exist.
   * @param identityName The name of the identity, which is copied.
   * @return The PibIdentity object.
   */
  ptr_lib::shared_ptr<PibIdentity>
  addIdentity(const Name& identityName);

  /**
   * Remove the identity with name identityName, and its related keys and
   * certificates. If the default identity is being removed, no default identity
   * will be selected.  If the identity does not exist, do nothing.
   * @param identityName The name of the identity.
   */
  void
  removeIdentity(const Name& identityName);

  /**
   * Set the identity with name identityName as the default identity.
   * Create the identity if it does not exist.
   * @param identityName The name of the identity.
   * @return The PibIdentity object of the default identity.
   */
  ptr_lib::shared_ptr<PibIdentity>
  setDefaultIdentity(const Name& identityName);

  // Disable the copy constructor and assignment operator.
  Pib(const Pib& other);
  Pib& operator=(const Pib& other);

  std::string scheme_;
  std::string location_;

  ptr_lib::shared_ptr<PibIdentity> defaultIdentity_;

  PibIdentityContainer identities_;

  ptr_lib::shared_ptr<PibImpl> pibImpl_;
};

}

#endif
