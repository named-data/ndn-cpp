/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2015 Regents of the University of California.
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

#ifndef NDN_MEMORY_IDENTITY_STORAGE_HPP
#define NDN_MEMORY_IDENTITY_STORAGE_HPP

#include <vector>
#include <map>
#include "identity-storage.hpp"

namespace ndn {

/**
 * MemoryIdentityStorage extends IdentityStorage and implements its methods to store identity, public key and certificate objects in memory.
 * The application must get the objects through its own means and add the objects to the MemoryIdentityStorage object.
 * To use permanent file-based storage, see BasicIdentityStorage.
 */
class MemoryIdentityStorage : public IdentityStorage {
public:
  /**
   * The virtual Destructor.
   */
  virtual
  ~MemoryIdentityStorage();

  /**
   * Check if the specified identity already exists.
   * @param identityName The identity name.
   * @return true if the identity exists, otherwise false.
   */
  virtual bool
  doesIdentityExist(const Name& identityName);

  /**
   * Add a new identity. Do nothing if the identity already exists.
   * @param identityName The identity name to be added.
   */
  virtual void
  addIdentity(const Name& identityName);

  /**
   * Revoke the identity.
   * @return true if the identity was revoked, false if not.
   */
  virtual bool
  revokeIdentity();

  /**
   * Check if the specified key already exists.
   * @param keyName The name of the key.
   * @return true if the key exists, otherwise false.
   */
  virtual bool
  doesKeyExist(const Name& keyName);

  /**
   * Add a public key to the identity storage. Also call addIdentity to ensure
   * that the identityName for the key exists.
   * @param keyName The name of the public key to be added.
   * @param keyType Type of the public key to be added.
   * @param publicKeyDer A blob of the public key DER to be added.
   */
  virtual void
  addKey(const Name& keyName, KeyType keyType, const Blob& publicKeyDer);

  /**
   * Get the public key DER blob from the identity storage.
   * @param keyName The name of the requested public key.
   * @return The DER Blob.  If not found, return a Blob with a null pointer.
   */
  virtual Blob
  getKey(const Name& keyName);

  /**
   * Activate a key.  If a key is marked as inactive, its private part will not be used in packet signing.
   * @param keyName name of the key
   */
  virtual void
  activateKey(const Name& keyName);

  /**
   * Deactivate a key. If a key is marked as inactive, its private part will not be used in packet signing.
   * @param keyName name of the key
   */
  virtual void
  deactivateKey(const Name& keyName);

  /**
   * Check if the specified certificate already exists.
   * @param certificateName The name of the certificate.
   * @return true if the certificate exists, otherwise false.
   */
  virtual bool
  doesCertificateExist(const Name& certificateName);

  /**
   * Add a certificate to the identity storage.
   * @param certificate The certificate to be added.  This makes a copy of the certificate.
   */
  virtual void
  addCertificate(const IdentityCertificate& certificate);

  /**
   * Get a certificate from the identity storage.
   * @param certificateName The name of the requested certificate.
   * @param allowAny (optional) If false, only a valid certificate will be returned,
   *   otherwise validity is disregarded. If omitted, allowAny is false.
   * @return The requested certificate.  If not found, return a shared_ptr with a null pointer.
   */
  virtual ptr_lib::shared_ptr<IdentityCertificate>
  getCertificate(const Name &certificateName, bool allowAny = false);


  /*****************************************
   *           Get/Set Default             *
   *****************************************/

  /**
   * Get the default identity.
   * @return The name of default identity.
   * @throws SecurityException if the default identity is not set.
   */
  virtual Name
  getDefaultIdentity();

  /**
   * Get the default key name for the specified identity.
   * @param identityName The identity name.
   * @return The default key name.
   * @throws SecurityException if the default key name for the identity is not set.
   */
  virtual Name
  getDefaultKeyNameForIdentity(const Name& identityName);

  /**
   * Get the default certificate name for the specified key.
   * @param keyName The key name.
   * @return The default certificate name.
   * @throws SecurityException if the default certificate name for the key name
   * is not set.
   */
  virtual Name
  getDefaultCertificateNameForKey(const Name& keyName);

  /**
   * Append all the key names of a particular identity to the nameList.
   * @param identityName The identity name to search for.
   * @param nameList Append result names to nameList.
   * @param isDefault If true, add only the default key name. If false, add only
   * the non-default key names.
   */
  virtual void
  getAllKeyNamesOfIdentity
    (const Name& identityName, std::vector<Name>& nameList, bool isDefault);

  /**
   * Set the default identity.  If the identityName does not exist, then clear the default identity
   * so that getDefaultIdentity() throws an exception.
   * @param identityName The default identity name.
   */
  virtual void
  setDefaultIdentity(const Name& identityName);

  /**
   * Set the default key name for the specified identity.
   * @param keyName The key name.
   * @param identityNameCheck (optional) The identity name to check the keyName.
   */
  virtual void
  setDefaultKeyNameForIdentity(const Name& keyName, const Name& identityNameCheck = Name());

  /**
   * Set the default key name for the specified identity.
   * @param keyName The key name.
   * @param certificateName The certificate name.
   */
  virtual void
  setDefaultCertificateNameForKey(const Name& keyName, const Name& certificateName);

  /*****************************************
   *            Delete Methods             *
   *****************************************/

  /**
   * Delete a certificate.
   * @param certificateName The certificate name.
   */
  virtual void
  deleteCertificateInfo(const Name& certificateName);

  /**
   * Delete a public key and related certificates.
   * @param keyName The key name.
   */
  virtual void
  deletePublicKeyInfo(const Name& keyName);

  /**
   * Delete an identity and related public keys and certificates.
   * @param identity The identity name.
   */
  virtual void
  deleteIdentityInfo(const Name& identity);

private:
  class KeyRecord {
  public:
    KeyRecord(KeyType keyType, const Blob &keyDer)
    : keyType_(keyType), keyDer_(keyDer)
    {
    }

    const KeyType getKeyType() const { return keyType_; }

    const Blob& getKeyDer() { return keyDer_; }

  private:
    KeyType keyType_;
    Blob keyDer_;
  };

  std::vector<std::string> identityStore_; /**< A list of name URI. */
  std::string defaultIdentity_;            /**< The default identity in identityStore_, or "" if not defined. */
  std::map<std::string, ptr_lib::shared_ptr<KeyRecord> > keyStore_; /**< The map key is the keyName.toUri() */
  std::map<std::string, Blob> certificateStore_;                    /**< The map key is the certificateName.toUri() */
};

}

#endif
