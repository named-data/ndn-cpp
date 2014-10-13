/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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

#ifndef NDN_BASIC_IDENTITY_STORAGE_H
#define NDN_BASIC_IDENTITY_STORAGE_H

// Only compile if ndn-cpp-config.h defines NDN_CPP_HAVE_SQLITE3.
#include <ndn-cpp/ndn-cpp-config.h>
#ifdef NDN_CPP_HAVE_SQLITE3

#include <sqlite3.h>
#include "../../common.hpp"
#include "identity-storage.hpp"

namespace ndn
{

/**
 * BasicIdentityStorage extends IdentityStorage to implement a basic storage of identity, public keys and certificates
 * using SQLite.
 */
class BasicIdentityStorage : public IdentityStorage {
public:
  BasicIdentityStorage();

  /**
   * The virtual Destructor.
   */
  virtual
  ~BasicIdentityStorage();

  /**
   * Check if the specified identity already exists.
   * @param identityName The identity name.
   * @return true if the identity exists, otherwise false.
   */
  virtual bool
  doesIdentityExist(const Name& identityName);

  /**
   * Add a new identity. An exception will be thrown if the identity already exists.
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
   * Add a public key to the identity storage.
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
   * Get the KeyType of the public key with the given keyName.
   * @param keyName The name of the requested public key.
   * @return The KeyType, for example KEY_TYPE_RSA.
   */
  virtual KeyType
  getKeyType(const Name& keyName);

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
   * Add a certificate in to the identity storage without checking if the identity and key exists.
   * @param certificate The certificate to be added.
   */
  void
  addAnyCertificate (const IdentityCertificate& certificate);

  /**
   * Add a certificate to the identity storage.
   * @param certificate The certificate to be added.  This makes a copy of the certificate.
   */
  virtual void
  addCertificate(const IdentityCertificate& certificate);

  /**
   * Get a certificate from the identity storage.
   * @param certificateName The name of the requested certificate.
   * @param allowAny If false, only a valid certificate will be returned, otherwise validity is disregarded.
   * @return The requested certificate.  If not found, return a shared_ptr with a null pointer.
   */
  virtual ptr_lib::shared_ptr<Data>
  getCertificate(const Name &certificateName, bool allowAny = false);


  /*****************************************
   *           Get/Set Default             *
   *****************************************/

  /**
   * Get the default identity.
   * @return The name of default identity.
   * @throw SecurityException if the default identity is not set.
   */
  virtual Name
  getDefaultIdentity();

  /**
   * Get the default key name for the specified identity.
   * @param identityName The identity name.
   * @return The default key name.
   * @throw SecurityException if the default key name for the identity is not set.
   */
  virtual Name
  getDefaultKeyNameForIdentity(const Name& identityName);

  /**
   * Get the default certificate name for the specified key.
   * @param keyName The key name.
   * @return The default certificate name.
   * @throw SecurityException if the default certificate name for the key name
   * is not set.
   */
  virtual Name
  getDefaultCertificateNameForKey(const Name& keyName);

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

private:

  void
  updateKeyStatus(const Name& keyName, bool isActive);

  sqlite3 *database_;
};

}

#endif // NDN_CPP_HAVE_SQLITE3

#endif
