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

#ifndef NDN_KEY_CHAIN_HPP
#define NDN_KEY_CHAIN_HPP

#include "../data.hpp"
#include "../interest.hpp"
#include "../face.hpp"
#include "identity/identity-manager.hpp"
#include "policy/validation-request.hpp"
#include "key-params.hpp"

namespace ndn {

class PolicyManager;

/**
 * KeyChain is the main class of the security library.
 *
 * The KeyChain class provides a set of interfaces to the security library such as identity management, policy configuration
 * and packet signing and verification.
 * @note This class is an experimental feature.  See the API docs for more detail at
 * http://named-data.net/doc/ndn-ccl-api/key-chain.html .
 */
class KeyChain {
public:
  /**
   * Create a new KeyChain with the given IdentityManager and PolicyManager.
   * @param identityManager An object of a subclass of IdentityManager.
   * @param policyManager An object of a subclass of PolicyManager.
   */
  KeyChain
    (const ptr_lib::shared_ptr<IdentityManager>& identityManager,
     const ptr_lib::shared_ptr<PolicyManager>& policyManager);

  /**
   * Create a new KeyChain with the given IdentityManager and a
   * NoVerifyPolicyManager.
   * @param identityManager An object of a subclass of IdentityManager.
   */
  KeyChain(const ptr_lib::shared_ptr<IdentityManager>& identityManager);

  /**
   * Create a new KeyChain with the the default IdentityManager and a
   * NoVerifyPolicyManager.
   */
  KeyChain();

  /*****************************************
   *          Identity Management          *
   *****************************************/

  /**
   * Create an identity by creating a pair of Key-Signing-Key (KSK) for this identity and a self-signed certificate of the KSK.
   * @param identityName The name of the identity.
   * @param params (optional) The key parameters if a key needs to be generated
   * for the identity. If omitted, use DEFAULT_KEY_PARAMS.
   * @return The key name of the auto-generated KSK of the identity.
   */
  Name
  createIdentity(const Name& identityName, const KeyParams& params = DEFAULT_KEY_PARAMS)
  {
    return identityManager_->createIdentity(identityName, params);
  }

  /**
   * Delete the identity from the public and private key storage. If the
   * identity to be deleted is the current default system default, this will not
   * delete the identity and will return immediately.
   * @param identityName The name of the identity.
   */
  void
  deleteIdentity(const Name& identityName)
  {
    identityManager_->deleteIdentity(identityName);
  }

  /**
   * Get the default identity.
   * @return The name of default identity.
   * @throws SecurityException if the default identity is not set.
   */
  Name
  getDefaultIdentity()
  {
    return identityManager_->getDefaultIdentity();
  }

  /**
   * Get the default certificate name of the default identity.
   * @return The requested certificate name.
   * @throws SecurityException if the default identity is not set or the default
   * key name for the identity is not set or the default certificate name for
   * the key name is not set.
   */
  Name
  getDefaultCertificateName()
  {
    return identityManager_->getDefaultCertificateName();
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
  generateRSAKeyPair(const Name& identityName, bool isKsk = false, int keySize = 2048)
  {
    return identityManager_->generateRSAKeyPair(identityName, isKsk, keySize);
  }

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
  generateEcdsaKeyPair(const Name& identityName, bool isKsk = false, int keySize = 256)
  {
    return identityManager_->generateEcdsaKeyPair(identityName, isKsk, keySize);
  }

  /**
   * Set a key as the default key of an identity.
   * @param keyName The name of the key.
   * @param identityName (optional) the name of the identity. If not specified,
   * the identity name is inferred from the keyName.
   */
  void
  setDefaultKeyForIdentity(const Name& keyName, const Name& identityName = Name())
  {
    return identityManager_->setDefaultKeyForIdentity(keyName, identityName);
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
  generateRSAKeyPairAsDefault(const Name& identityName, bool isKsk = false, int keySize = 2048)
  {
    return identityManager_->generateRSAKeyPairAsDefault(identityName, isKsk, keySize);
  }

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
  generateEcdsaKeyPairAsDefault(const Name& identityName, bool isKsk = false, int keySize = 256)
  {
    return identityManager_->generateEcdsaKeyPairAsDefault(identityName, isKsk, keySize);
  }

  /**
   * Create a public key signing request.
   * @param keyName The name of the key.
   * @returns The signing request data.
   */
  Blob
  createSigningRequest(const Name& keyName)
  {
    return identityManager_->getPublicKey(keyName)->getKeyDer();
  }

  /**
   * Install an identity certificate into the public key identity storage.
   * @param certificate The certificate to to added.
   */
  void
  installIdentityCertificate(const IdentityCertificate& certificate)
  {
    identityManager_->addCertificate(certificate);
  }

  /**
   * Set the certificate as the default for its corresponding key.
   * @param certificate The certificate.
   */
  void
  setDefaultCertificateForKey(const IdentityCertificate& certificate)
  {
    identityManager_->setDefaultCertificateForKey(certificate);
  }

  /**
   * Get a certificate with the specified name.
   * @param certificateName The name of the requested certificate.
   * @return The requested certificate which is valid.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getCertificate(const Name& certificateName)
  {
    return identityManager_->getCertificate(certificateName);
  }

  /**
   * Get a certificate even if the certificate is not valid anymore.
   * @param certificateName The name of the requested certificate.
   * @return The requested certificate.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getAnyCertificate(const Name& certificateName)
  {
    return identityManager_->getAnyCertificate(certificateName);
  }

  /**
   * Get an identity certificate with the specified name.
   * @param certificateName The name of the requested certificate.
   * @return The requested certificate which is valid.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getIdentityCertificate(const Name& certificateName)
  {
    return identityManager_->getCertificate(certificateName);
  }

  /**
   * Get an identity certificate even if the certificate is not valid anymore.
   * @param certificateName The name of the requested certificate.
   * @return The requested certificate.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  getAnyIdentityCertificate(const Name& certificateName)
  {
    return identityManager_->getAnyCertificate(certificateName);
  }

  /**
   * Revoke a key.
   * @param keyName The name of the key that will be revoked.
   */
  void
  revokeKey(const Name & keyName)
  {
    //TODO: Implement
  }

  /**
   * Revoke a certificate.
   * @param certificateName The name of the certificate that will be revoked.
   */
  void
  revokeCertificate(const Name & certificateName)
  {
    //TODO: Implement
  }

  /**
   * Get the identity manager given to or created by the constructor.
   * @return The identity manager.
   */
  const ptr_lib::shared_ptr<IdentityManager>&
  getIdentityManager() { return identityManager_; }

  /*****************************************
   *           Policy Management           *
   *****************************************/

  /**
   * Get the policy manager given to or created by the constructor.
   * @return The policy manager.
   */
  const ptr_lib::shared_ptr<PolicyManager>&
  getPolicyManager() { return policyManager_; }

  /*****************************************
   *              Sign/Verify              *
   *****************************************/

  /**
   * Wire encode the Data object, sign it and set its signature.
   * @param data The Data object to be signed.  This updates its signature and key locator field and wireEncoding.
   * @param certificateName The certificate name of the key to use for signing.
   * @param wireFormat (optional) A WireFormat object used to encode the input. If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  sign(Data& data, const Name& certificateName,
       WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    identityManager_->signByCertificate(data, certificateName, wireFormat);
  }

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
  sign
    (Interest& interest, const Name& certificateName,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    identityManager_->signInterestByCertificate
      (interest, certificateName, wireFormat);
  }

  /**
   * Sign the byte array using a certificate name and return a Signature object.
   * @param buffer The byte array to be signed.
   * @param bufferLength the length of buffer.
   * @param certificateName The certificate name used to get the signing key and which will be put into KeyLocator.
   * @return The Signature.
   */
  ptr_lib::shared_ptr<Signature>
  sign(const uint8_t* buffer, size_t bufferLength, const Name& certificateName)
  {
    return identityManager_->signByCertificate
      (buffer, bufferLength, certificateName);
  }

  /**
   * Sign the byte array using a certificate name and return a Signature object.
   * @param buffer The byte array to be signed.
   * @param certificateName The certificate name used to get the signing key and which will be put into KeyLocator.
   * @return The Signature.
   */
  ptr_lib::shared_ptr<Signature>
  sign(const std::vector<uint8_t>& buffer, const Name& certificateName)
  {
    return sign(&buffer[0], buffer.size(), certificateName);
  }

  /**
   * Wire encode the Data object, sign it and set its signature.
   * @param data The Data object to be signed.  This updates its signature and key locator field and wireEncoding.
   * @param identityName (optional) The identity name for the key to use for signing.  If omitted, infer the signing identity from the data packet name.
   * @param wireFormat (optional) A WireFormat object used to encode the input. If omitted, use WireFormat getDefaultWireFormat().
   */
  void
  signByIdentity(Data& data, const Name& identityName = Name(), WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Sign the byte array using an identity name and return a Signature object.
   * @param buffer The byte array to be signed.
   * @param bufferLength the length of buffer.
   * @param identityName The identity name.
   * @return The Signature.
   */
  ptr_lib::shared_ptr<Signature>
  signByIdentity(const uint8_t* buffer, size_t bufferLength, const Name& identityName);

  /**
   * Sign the byte array using an identity name and return a Signature object.
   * @param buffer The byte array to be signed.
   * @param identityName The identity name.
   * @return The Signature.
   */
  ptr_lib::shared_ptr<Signature>
  signByIdentity(const std::vector<uint8_t>& buffer, const Name& identityName)
  {
    return signByIdentity(&buffer[0], buffer.size(), identityName);
  }

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
    (Data& data, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    identityManager_->signWithSha256(data, wireFormat);
  }

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
  signWithSha256
    (Interest& interest, WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    identityManager_->signInterestWithSha256(interest, wireFormat);
  }

  /**
   * Check the signature on the Data object and call either onVerify or onVerifyFailed.
   * We use callback functions because verify may fetch information to check the signature.
   * @param data The Data object with the signature to check.
   * @param onVerified If the signature is verified, this calls onVerified(data).
   * @param onVerifyFailed If the signature check fails, this calls onVerifyFailed(data).
   */
  void
  verifyData
    (const ptr_lib::shared_ptr<Data>& data, const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed, int stepCount = 0);

  /**
   * Check the signature on the signed interest and call either onVerify or
   * onVerifyFailed. We use callback functions because verify may fetch
   * information to check the signature.
   * @param interest The interest with the signature to check.
   * @param onVerified If the signature is verified, this calls onVerified(interest).
   * @param onVerifyFailed If the signature check fails, this calls onVerifyFailed(interest).
   */
  void
  verifyInterest
    (const ptr_lib::shared_ptr<Interest>& interest,
     const OnVerifiedInterest& onVerified,
     const OnVerifyInterestFailed& onVerifyFailed, int stepCount = 0,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Set the Face which will be used to fetch required certificates.
   * @param face A pointer to the Face object.
   */
  void
  setFace(Face* face) { face_ = face; }

  static const RsaKeyParams DEFAULT_KEY_PARAMS;

private:
  void
  onCertificateData
    (const ptr_lib::shared_ptr<const Interest> &interest, const ptr_lib::shared_ptr<Data> &data, ptr_lib::shared_ptr<ValidationRequest> nextStep);

  void
  onCertificateInterestTimeout
    (const ptr_lib::shared_ptr<const Interest> &interest, int retry, const OnVerifyFailed& onVerifyFailed,
     const ptr_lib::shared_ptr<Data> &data, ptr_lib::shared_ptr<ValidationRequest> nextStep);

  /**
   * This is the same as onCertificateInterestTimeout, but we call
   * onVerifyFailed(originalInterest) if we have too many retries.
   */
  void
  onCertificateInterestTimeoutForVerifyInterest
    (const ptr_lib::shared_ptr<const Interest> &interest, int retry,
     const OnVerifyInterestFailed& onVerifyFailed,
     const ptr_lib::shared_ptr<Interest>& originalInterest,
     ptr_lib::shared_ptr<ValidationRequest> nextStep);

  ptr_lib::shared_ptr<IdentityManager> identityManager_;
  ptr_lib::shared_ptr<PolicyManager> policyManager_;
  Face* face_;
  const int maxSteps_;
};

}

#endif
