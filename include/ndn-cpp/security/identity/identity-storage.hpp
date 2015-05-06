/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_IDENTITY_STORAGE_HPP
#define NDN_IDENTITY_STORAGE_HPP

#include "../../name.hpp"
#include "../security-common.hpp"

namespace ndn {

class IdentityCertificate;
class Data;

/**
 * IdentityStorage is a base class for the storage of identity, public keys and certificates.
 * Private keys are stored in PrivateKeyStorage.
 * This is an abstract base class.  A subclass must implement the methods.
 */
class IdentityStorage {
public:
  IdentityStorage();

  /**
   * The virtual Destructor.
   */
  virtual
  ~IdentityStorage() {}

  /**
   * Check if the specified identity already exists.
   * @param identityName The identity name.
   * @return true if the identity exists, otherwise false.
   */
  virtual bool
  doesIdentityExist(const Name& identityName) = 0;

  /**
   * Add a new identity. Do nothing if the identity already exists.
   * @param identityName The identity name to be added.
   */
  virtual void
  addIdentity(const Name& identityName) = 0;

  /**
   * Revoke the identity.
   * @return true if the identity was revoked, false if not.
   */
  virtual bool
  revokeIdentity() = 0;

  /**
   * Generate a name for a new key belonging to the identity.
   * @param identityName The identity name.
   * @param useKsk If true, generate a KSK name, otherwise a DSK name.
   * @return The generated key name.
   */
  Name
  getNewKeyName(const Name& identityName, bool useKsk);

  /**
   * Check if the specified key already exists.
   * @param keyName The name of the key.
   * @return true if the key exists, otherwise false.
   */
  virtual bool
  doesKeyExist(const Name& keyName) = 0;

  /**
   * Add a public key to the identity storage. Also call addIdentity to ensure
   * that the identityName for the key exists.
   * @param keyName The name of the public key to be added.
   * @param keyType Type of the public key to be added.
   * @param publicKeyDer A blob of the public key DER to be added.
   */
  virtual void
  addKey(const Name& keyName, KeyType keyType, const Blob& publicKeyDer) = 0;

  /**
   * Get the public key DER blob from the identity storage.
   * @param keyName The name of the requested public key.
   * @return The DER Blob.  If not found, return a Blob with a null pointer.
   */
  virtual Blob
  getKey(const Name& keyName) = 0;

  /**
   * Activate a key.  If a key is marked as inactive, its private part will not be used in packet signing.
   * @param keyName name of the key
   */
  virtual void
  activateKey(const Name& keyName) = 0;

  /**
   * Deactivate a key. If a key is marked as inactive, its private part will not be used in packet signing.
   * @param keyName name of the key
   */
  virtual void
  deactivateKey(const Name& keyName) = 0;

  /**
   * Check if the specified certificate already exists.
   * @param certificateName The name of the certificate.
   * @return true if the certificate exists, otherwise false.
   */
  virtual bool
  doesCertificateExist(const Name& certificateName) = 0;

  /**
   * Add a certificate to the identity storage.
   * @param certificate The certificate to be added.  This makes a copy of the certificate.
   */
  virtual void
  addCertificate(const IdentityCertificate& certificate) = 0;

  /**
   * Get a certificate from the identity storage.
   * @param certificateName The name of the requested certificate.
   * @param allowAny (optional) If false, only a valid certificate will be returned,
   *   otherwise validity is disregarded. If omitted, allowAny is false.
   * @return The requested certificate.  If not found, return a shared_ptr with a null pointer.
   */
  virtual ptr_lib::shared_ptr<IdentityCertificate>
  getCertificate(const Name &certificateName, bool allowAny = false) = 0;


  /*****************************************
   *           Get/Set Default             *
   *****************************************/

  /**
   * Get the default identity.
   * @return The name of default identity.
   * @throws SecurityException if the default identity is not set.
   */
  virtual Name
  getDefaultIdentity() = 0;

  /**
   * Get the default key name for the specified identity.
   * @param identityName The identity name.
   * @return The default key name.
   * @throws SecurityException if the default key name for the identity is not set.
   */
  virtual Name
  getDefaultKeyNameForIdentity(const Name& identityName) = 0;

  /**
   * Get the default certificate name for the specified identity.
   * @param identityName The identity name.
   * @return The default certificate name.
   * @throws SecurityException if the default key name for the identity is not
   * set or the default certificate name for the key name is not set.
   */
  Name
  getDefaultCertificateNameForIdentity(const Name& identityName);

  /**
   * Get the default certificate name for the specified key.
   * @param keyName The key name.
   * @return The default certificate name.
   * @throws SecurityException if the default certificate name for the key name
   * is not set.
   */
  virtual Name
  getDefaultCertificateNameForKey(const Name& keyName) = 0;

  /**
   * Append all the key names of a particular identity to the nameList.
   * @param identityName The identity name to search for.
   * @param nameList Append result names to nameList.
   * @param isDefault If true, add only the default key name. If false, add only
   * the non-default key names.
   */
  virtual void
  getAllKeyNamesOfIdentity
    (const Name& identityName, std::vector<Name>& nameList, bool isDefault) = 0;

  /**
   * Set the default identity.  If the identityName does not exist, then clear the default identity
   * so that getDefaultIdentity() throws an exception.
   * @param identityName The default identity name.
   */
  virtual void
  setDefaultIdentity(const Name& identityName) = 0;

  /**
   * Set the default key name for the specified identity.
   * @param keyName The key name.
   * @param identityNameCheck (optional) The identity name to check the keyName.
   */
  virtual void
  setDefaultKeyNameForIdentity(const Name& keyName, const Name& identityNameCheck = Name()) = 0;

  /**
   * Set the default key name for the specified identity.
   * @param keyName The key name.
   * @param certificateName The certificate name.
   */
  virtual void
  setDefaultCertificateNameForKey(const Name& keyName, const Name& certificateName) = 0;

  /*****************************************
   *            Delete Methods             *
   *****************************************/

  /**
   * Delete a certificate.
   * @param certificateName The certificate name.
   */
  virtual void
  deleteCertificateInfo(const Name& certificateName) = 0;

  /**
   * Delete a public key and related certificates.
   * @param keyName The key name.
   */
  virtual void
  deletePublicKeyInfo(const Name& keyName) = 0;

  /**
   * Delete an identity and related public keys and certificates.
   * @param identity The identity name.
   */
  virtual void
  deleteIdentityInfo(const Name& identity) = 0;

private:
  static bool lastTimestampIsInitialized_;
  static uint64_t lastTimestamp_;
};

}

#endif
