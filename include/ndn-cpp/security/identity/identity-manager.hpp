/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013-2019 Regents of the University of California.
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

#ifndef NDN_IDENTITY_MANAGER_HPP
#define NDN_IDENTITY_MANAGER_HPP

#include "../certificate/identity-certificate.hpp"
#include "../../interest.hpp"
#include "identity-storage.hpp"
#include "../certificate/public-key.hpp"
#include "private-key-storage.hpp"

namespace ndn {

class ConfigFile;

/**
 * An IdentityManager is the interface of operations related to identity, keys, and certificates.
 */
class IdentityManager {
public:
  /**
   * Create a new IdentityManager to use the given IdentityStorage and
   * PrivateKeyStorage.
   * @param identityStorage An object of a subclass of IdentityStorage.
   * @param privateKeyStorage An object of a subclass of PrivateKeyStorage.
   */
  IdentityManager
    (const ptr_lib::shared_ptr<IdentityStorage>& identityStorage,
     const ptr_lib::shared_ptr<PrivateKeyStorage>& privateKeyStorage);

  /**
   * Create a new IdentityManager to use the given IdentityStorage and
   * the default PrivateKeyStorage for your system, which is
   * OSXPrivateKeyStorage for OS X, otherwise FilePrivateKeyStorage.
   * @param identityStorage An object of a subclass of IdentityStorage.
   */
  IdentityManager(const ptr_lib::shared_ptr<IdentityStorage>& identityStorage);

  /**
   * Create a new IdentityManager to use BasicIdentityStorage and
   * the default PrivateKeyStorage for your system, which is
   * OSXPrivateKeyStorage for OS X, otherwise FilePrivateKeyStorage.
   * @param identityStorage An object of a subclass of IdentityStorage.
   */
  IdentityManager();

  /**
   * Create an identity by creating a pair of Key-Signing-Key (KSK) for this
   * identity and a self-signed certificate of the KSK. If a key pair or
   * certificate for the identity already exists, use it.
   * @param identityName The name of the identity.
   * @param params The key parameters if a key needs to be generated for the identity.
   * @return The name of the default certificate of the identity.
   */
  Name
  createIdentityAndCertificate(const Name& identityName, const KeyParams& params);

  /**
   * Create an identity by creating a pair of Key-Signing-Key (KSK) for this
   * identity and a self-signed certificate of the KSK.
   * @deprecated Use createIdentityAndCertificate which returns the
   * certificate name instead of the key name. You can use
   * IdentityCertificate.certificateNameToPublicKeyName to convert the
   * certificate name to the key name.
   * @param identityName The name of the identity.
   * @param params The key parameters if a key needs to be generated for the identity.
   * @return The key name of the auto-generated KSK of the identity.
   */
  Name
  DEPRECATED_IN_NDN_CPP createIdentity
    (const Name& identityName, const KeyParams& params)
  {
    return IdentityCertificate::certificateNameToPublicKeyName
      (createIdentityAndCertificate(identityName, params));
  }

  /**
   * Use the keyName to get the public key from the identity storage and
   * prepare an unsigned identity certificate.
   * @param keyName The key name, e.g., `/<identity_name>/ksk-123456`.
   * @param signingIdentity The signing identity.
   * @param notBefore See IdentityCertificate.
   * @param notAfter See IdentityCertificate.
   * @param subjectDescription A list of CertificateSubjectDescription. See
   * IdentityCertificate. If empty, this adds a an ATTRIBUTE_NAME based on the
   * keyName.
   * @param certPrefix (optional) The prefix before the `KEY` component. If 0, this
   * infers the certificate name according to the relation between the
   * signingIdentity and the subject identity. If the signingIdentity is a
   * prefix of the subject identity, `KEY` will be inserted after the
   * signingIdentity, otherwise `KEY` is inserted after subject identity (i.e.,
   * before `ksk-...`).
   * @return The unsigned IdentityCertificate, or a null shared_ptr if the
   * inputs are invalid.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  prepareUnsignedIdentityCertificate
    (const Name& keyName, const Name& signingIdentity,
     MillisecondsSince1970 notBefore, MillisecondsSince1970 notAfter,
     std::vector<CertificateSubjectDescription>& subjectDescription,
     const Name* certPrefix = 0);

  /**
   * Prepare an unsigned identity certificate.
   * @param keyName The key name, e.g., `/<identity_name>/ksk-123456`.
   * @param publicKey The public key to sign.
   * @param signingIdentity The signing identity.
   * @param notBefore See IdentityCertificate.
   * @param notAfter See IdentityCertificate.
   * @param subjectDescription A list of CertificateSubjectDescription. See
   * IdentityCertificate. If empty, this adds a an ATTRIBUTE_NAME based on the
   * keyName.
   * @param certPrefix (optional) The prefix before the `KEY` component. If 0, this
   * infers the certificate name according to the relation between the
   * signingIdentity and the subject identity. If the signingIdentity is a
   * prefix of the subject identity, `KEY` will be inserted after the
   * signingIdentity, otherwise `KEY` is inserted after subject identity (i.e.,
   * before `ksk-...`).
   * @return The unsigned IdentityCertificate, or a null shared_ptr if the
   * inputs are invalid.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  prepareUnsignedIdentityCertificate
    (const Name& keyName, const PublicKey& publicKey,
     const Name& signingIdentity, MillisecondsSince1970 notBefore,
     MillisecondsSince1970 notAfter,
     std::vector<CertificateSubjectDescription>& subjectDescription,
     const Name* certPrefix = 0);

  /**
   * Delete the identity from the public and private key storage. If the
   * identity to be deleted is the current default system default, this will not
   * delete the identity and will return immediately.
   * @param identityName The name of the identity.
   */
  void
  deleteIdentity(const Name& identityName);

  /**
   * Set the default identity.  If the identityName does not exist, then clear
   * the default identity so that getDefaultIdentity() throws an exception.
   * @param identityName The default identity name.
   */
  void
  setDefaultIdentity(const Name& identityName)
  {
    identityStorage_->setDefaultIdentity(identityName);
  }

  /**
   * Get the default identity.
   * @return The name of default identity.
   * @throws SecurityException if the default identity is not set.
   */
  Name
  getDefaultIdentity()
  {
    return identityStorage_->getDefaultIdentity();
  }

  /**
   * Get the certificate of the default identity.
   * @return The requested certificate. If not found, return a shared_ptr with
   * a null pointer.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getDefaultCertificate()
  {
    return identityStorage_->getDefaultCertificate();
  }

  /**
   * Generate a pair of RSA keys for the specified identity.
   * @param identityName The name of the identity.
   * @param isKsk (optional) true for generating a Key-Signing-Key (KSK), false
   * for a Data-Signing-Key (DSK). If omitted, generate a Data-Signing-Key.
   * @param keySize (optional) The size of the key. If omitted, use a default
   * secure key size.
   * @return The generated key name.
   */
  Name
  generateRSAKeyPair(const Name& identityName, bool isKsk = false, int keySize = 2048);

  /**
   * Generate a pair of ECDSA keys for the specified identity.
   * @param identityName The name of the identity.
   * @param isKsk (optional) true for generating a Key-Signing-Key (KSK), false
   * for a Data-Signing-Key (DSK). If omitted, generate a Data-Signing-Key.
   * @param keySize (optional) The size of the key. If omitted, use a default
   * secure key size.
   * @return The generated key name.
   */
  Name
  generateEcdsaKeyPair(const Name& identityName, bool isKsk = false, int keySize = 256);

  /**
   * Set a key as the default key of an identity. The identity name is inferred
   * from keyName.
   * @param keyName The name of the key.
   * @param identityNameCheck (optional) The identity name to check that the
   * keyName contains the same identity name. If an empty name, it is ignored.
   */
  void
  setDefaultKeyForIdentity(const Name& keyName, const Name& identityNameCheck = Name())
  {
    identityStorage_->setDefaultKeyNameForIdentity(keyName, identityNameCheck);
  }

  /**
   * Get the default key for an identity.
   * @param identityName The name of the identity.
   * @return The default key name.
   * @throws SecurityException if the default key name for the identity is not set.
   */
  Name
  getDefaultKeyNameForIdentity(const Name& identityName)
  {
    return identityStorage_->getDefaultKeyNameForIdentity(identityName);
  }

  /**
   * Generate a pair of RSA keys for the specified identity and set it as default key for the identity.
   * @param identityName The name of the identity.
   * @param isKsk (optional) true for generating a Key-Signing-Key (KSK), false
   * for a Data-Signing-Key (DSK). If omitted, generate a Data-Signing-Key.
   * @param keySize (optional) The size of the key. If omitted, use a default
   * secure key size.
   * @return The generated key name.
   */
  Name
  generateRSAKeyPairAsDefault(const Name& identityName, bool isKsk = false, int keySize = 2048);

  /**
   * Generate a pair of ECDSA keys for the specified identity and set it as default key for the identity.
   * @param identityName The name of the identity.
   * @param isKsk (optional) true for generating a Key-Signing-Key (KSK), false
   * for a Data-Signing-Key (DSK). If omitted, generate a Data-Signing-Key.
   * @param keySize (optional) The size of the key. If omitted, use a default
   * secure key size.
   * @return The generated key name.
   */
  Name
  generateEcdsaKeyPairAsDefault(const Name& identityName, bool isKsk = false, int keySize = 256);

  /**
   * Get the public key with the specified name.
   * @param keyName The name of the key.
   * @return The public key.
   */
  ptr_lib::shared_ptr<PublicKey>
  getPublicKey(const Name& keyName)
  {
    return ptr_lib::shared_ptr<PublicKey>(new PublicKey
      (identityStorage_->getKey(keyName)));
  }

  /**
   * Create an identity certificate for a public key managed by this IdentityManager.
   * @param certificatePrefix The name of public key to be signed.
   * @param signerCertificateName The name of signing certificate.
   * @param notBefore The notBefore value in the validity field of the generated certificate.
   * @param notAfter The notAfter value in validity field of the generated certificate.
   * @return The name of generated identity certificate.
   */
  Name
  createIdentityCertificate
    (const Name& certificatePrefix, const Name& signerCertificateName, const MillisecondsSince1970& notBefore,
     const MillisecondsSince1970& notAfter);

  /**
   * Create an identity certificate for a public key supplied by the caller.
   * @param certificatePrefix The name of public key to be signed.
   * @param publickey The public key to be signed.
   * @param signerCertificateName The name of signing certificate.
   * @param notBefore The notBefore value in the validity field of the generated certificate.
   * @param notAfter The notAfter vallue in validity field of the generated certificate.
   * @return The generated identity certificate.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  createIdentityCertificate
    (const Name& certificatePrefix, const PublicKey& publickey, const Name& signerCertificateName,
     const MillisecondsSince1970& notBefore, const MillisecondsSince1970& notAfter);

  /**
   * Add a certificate into the public key identity storage.
   * @param certificate The certificate to to added.  This makes a copy of the certificate.
   */
  void
  addCertificate(const IdentityCertificate& certificate)
  {
    identityStorage_->addCertificate(certificate);
  }

  /**
   * Set the certificate as the default for its corresponding key.
   * @param certificate The certificate.
   */
  void
  setDefaultCertificateForKey(const IdentityCertificate& certificate);

  /**
   * Add a certificate into the public key identity storage and set the certificate as the default for its corresponding identity.
   * @param certificate The certificate to be added.  This makes a copy of the certificate.
   */
  void
  addCertificateAsIdentityDefault(const IdentityCertificate& certificate);

  /**
   * Add a certificate into the public key identity storage and set the certificate as the default of its corresponding key.
   * @param certificate The certificate to be added.  This makes a copy of the certificate.
   */
  void
  addCertificateAsDefault(const IdentityCertificate& certificate);

  /**
   * Get a certificate with the specified name.
   * @param certificateName The name of the requested certificate.
   * @return the requested certificate.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getCertificate(const Name& certificateName)
  {
    return identityStorage_->getCertificate(certificateName);
  }

  /**
   * Get the default certificate name for the specified identity, which will be used when signing is performed based on identity.
   * @param identityName The name of the specified identity.
   * @return The requested certificate name.
   * @throws SecurityException if the default key name for the identity is not
   * set or the default certificate name for the key name is not set.
   */
  Name
  getDefaultCertificateNameForIdentity(const Name& identityName)
  {
    return identityStorage_->getDefaultCertificateNameForIdentity(identityName);
  }

  /**
   * Get the default certificate name of the default identity, which will be used when signing is based on identity and
   * the identity is not specified.
   * @return The requested certificate name.
   * @throws SecurityException if the default identity is not set or the default
   * key name for the identity is not set or the default certificate name for
   * the key name is not set.
   */
  Name
  getDefaultCertificateName()
  {
    return identityStorage_->getDefaultCertificateNameForIdentity(getDefaultIdentity());
  }

  /**
   * Append all the identity names to the nameList.
   * @param nameList Append result names to nameList.
   * @param isDefault If true, add only the default key name. If false, add only
   * the non-default key names.
   */
  void
  getAllIdentities(std::vector<Name>& nameList, bool isDefault)
  {
    identityStorage_->getAllIdentities(nameList, isDefault);
  }

  /**
   * Append all the key names of a particular identity to the nameList.
   * @param identityName The identity name to search for.
   * @param nameList Append result names to nameList.
   * @param isDefault If true, add only the default key name. If false, add only
   * the non-default key names.
   */
  void
  getAllKeyNamesOfIdentity
    (const Name& identityName, std::vector<Name>& nameList, bool isDefault)
  {
    identityStorage_->getAllKeyNamesOfIdentity(identityName, nameList, isDefault);
  }

  /**
   * Append all the certificate names of a particular key name to the nameList.
   * @param keyName The key name to search for.
   * @param nameList Append result names to nameList.
   * @param isDefault If true, add only the default certificate name. If false,
   * add only the non-default certificate names.
   */
  void
  getAllCertificateNamesOfKey
    (const Name& keyName, std::vector<Name>& nameList, bool isDefault)
  {
    identityStorage_->getAllCertificateNamesOfKey(keyName, nameList, isDefault);
  }

  /**
   * Sign the byte array data based on the certificate name.
   * @param buffer The byte array to be signed.
   * @param bufferLength the length of buffer.
   * @param certificateName The signing certificate name.
   * @return The generated signature.
   */
  ptr_lib::shared_ptr<Signature>
  signByCertificate(const uint8_t* buffer, size_t bufferLength, const Name& certificateName);

  /**
   * Sign the byte array data based on the certificate name.
   * @param buffer The byte array to be signed.
   * @param certificateName The signing certificate name.
   * @return The generated signature.
   */
  ptr_lib::shared_ptr<Signature>
  signByCertificate(const std::vector<uint8_t>& buffer, const Name& certificateName)
  {
    return signByCertificate(&buffer[0], buffer.size(), certificateName);
  }

  /**
   * Sign data packet based on the certificate name.
   * @param data The Data object to sign and update its signature.
   * @param certificateName The Name identifying the certificate which identifies the signing key.
   * @param wireFormat The WireFormat for calling encodeData, or WireFormat::getDefaultWireFormat() if omitted.
   */
  void
  signByCertificate(Data& data, const Name& certificateName, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Append a SignatureInfo to the Interest name, sign the name components and
   * append a final name component with the signature bits.
   * @param interest The Interest object to be signed. This appends name
   * components of SignatureInfo and the signature bits.
   * @param certificateName The certificate name of the key to use for signing.
   * @param wireFormat (optional) A WireFormat object used to encode the input. If omitted,
   * use WireFormat getDefaultWireFormat().
   */
  void
  signInterestByCertificate
    (Interest& interest, const Name& certificateName,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Wire encode the Data object, digest it and set its SignatureInfo to
   * a DigestSha256.
   * @param data The Data object to be signed. This updates its signature and
   * wireEncoding.
   * @param wireFormat (optional) A WireFormat object used to encode the input.
   * If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  signWithSha256
    (Data& data, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Append a SignatureInfo for DigestSha256 to the Interest name, digest the
   * name components and append a final name component with the signature bits
   * (which is the digest).
   * @param interest The Interest object to be signed. This appends name
   * components of SignatureInfo and the signature bits.
   * @param wireFormat (optional) A WireFormat object used to encode the input.
   * If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  signInterestWithSha256
    (Interest& interest, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Generate a self-signed certificate for a public key.
   * @param keyName The name of the public key.
   * @return The generated certificate.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  selfSign(const Name& keyName);

private:
  /**
   * Generate a key pair for the specified identity.
   * @param identityName The name of the specified identity.
   * @param isKsk true for generating a Key-Signing-Key (KSK), false for a Data-Signing-Key (KSK).
   * @param params The parameters of the key.
   * @return The name of the generated key.
   */
  Name
  generateKeyPair(const Name& identityName, bool isKsk, const KeyParams& params);

  static Name
  getKeyNameFromCertificatePrefix(const Name& certificatePrefix);

  /**
   * Return a new Signature object based on the signature algorithm of the
   * public key with keyName (derived from certificateName).
   * @param certificateName The certificate name.
   * @param digestAlgorithm Set digestAlgorithm to the signature algorithm's
   * digest algorithm, e.g. DIGEST_ALGORITHM_SHA256.
   * @return A new object of the correct subclass of Signature.
   */
  ptr_lib::shared_ptr<Signature>
  makeSignatureByCertificate
    (const Name& certificateName, DigestAlgorithm& digestAlgorithm);

  /**
   * Get the IdentityStorage from the pib value in the configuration file if
   * supplied. Otherwise, get the default for this platform.
   * @param config The configuration file to check.
   * @return A new IdentityStorage.
   */
  static ptr_lib::shared_ptr<IdentityStorage>
  getDefaultIdentityStorage(ConfigFile& config);

  /**
   * Get the PrivateKeyStorage from the tpm value in the configuration file if
   * supplied. Otherwise, get the default for this platform.
   * @param config The configuration file to check.
   * @param canonicalTpmLocator Set canonicalTpmLocator to the canonical value
   * including the colon, * e.g. "tpm-file:".
   * @return A new PrivateKeyStorage.
   */
  static ptr_lib::shared_ptr<PrivateKeyStorage>
  getDefaultPrivateKeyStorage
    (ConfigFile& config, std::string& canonicalTpmLocator);

  /**
   * Check that identityStorage_->getTpmLocator() (if defined) matches the
   * canonicalTpmLocator.
   * @param canonicalTpmLocator The canonical locator from
   * getDefaultPrivateKeyStorage().
   * @throws SecurityException if the private key storage does not match.
   */
  void
  checkTpm(const std::string& canonicalTpmLocator);

  ptr_lib::shared_ptr<IdentityStorage> identityStorage_;
  ptr_lib::shared_ptr<PrivateKeyStorage> privateKeyStorage_;
};

}

#endif
