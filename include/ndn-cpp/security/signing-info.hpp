/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/signing-info.hpp
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

#ifndef NDN_SIGNING_INFO_HPP
#define NDN_SIGNING_INFO_HPP

#include "../name.hpp"
#include "../signature.hpp"
#include "pib/pib-identity.hpp"
#include "pib/pib-key.hpp"
#include "security-common.hpp"

namespace ndn {

/**
 * A SigningInfo holds the signing parameters passed to the KeyChain. A
 * SigningInfo is invalid if the specified identity/key/certificate does not
 * exist, or the PibIdentity or PibKey instance is not valid.
 */
class SigningInfo {
public:
  enum SignerType {
    /** No signer is specified. Use default settings or follow the trust schema. */
    SIGNER_TYPE_NULL = 0,
    /** The signer is an identity. Use its default key and default certificate. */
    SIGNER_TYPE_ID = 1,
    /** The signer is a key. Use its default certificate. */
    SIGNER_TYPE_KEY = 2,
    /** The signer is a certificate. Use it directly. */
    SIGNER_TYPE_CERT = 3,
    /** Use a SHA-256 digest. No signer needs to be specified. */
    SIGNER_TYPE_SHA256 = 4,
  };

  /**
   * Create a SigningInfo with the optional signerType and signerName and other
   * default values. The digest algorithm is set to DIGEST_ALGORITHM_SHA256.
   * @param signerType (optional) The type of signer. If omitted, use
   * SIGNER_TYPE_NULL (which will cause KeyChain::sign to use the default
   * identity).
   * @param signerName (optional) The name of signer. The interpretation of
   * the signerName differs based on the signerType. This copies the Name. If
   * omitted, use an empty Name.
   */
  SigningInfo
    (SignerType signerType = SIGNER_TYPE_NULL, const Name& signerName = Name())
  {
    reset(signerType);
    name_ = signerName;
    digestAlgorithm_ = DIGEST_ALGORITHM_SHA256;
  }

  /**
   * Create a SigningInfo of type SIGNER_TYPE_ID according to the given
   * PibIdentity. The digest algorithm is set to DIGEST_ALGORITHM_SHA256.
   * @param identity An existing PibIdentity which is not copied, or a null
   * PibIdentity. If this is a null ptr_lib::shared_ptr<PibIdentity>() then
   * use the default identity, otherwise use identity->getName().
   */
  SigningInfo(const ptr_lib::shared_ptr<PibIdentity>& identity)
  {
    digestAlgorithm_ = DIGEST_ALGORITHM_SHA256;
    setPibIdentity(identity);
  }

  /**
   * Create a SigningInfo of type SIGNER_TYPE_KEY according to the given PibKey.
   * The digest algorithm is set to DIGEST_ALGORITHM_SHA256.
   * @param key An existing PibKey which is not copied, or a null PibKey. If
   * this is a null ptr_lib::shared_ptr<PibKey>() then use the default
   * key for the identity, otherwise use key->getName().
   */
  SigningInfo(const ptr_lib::shared_ptr<PibKey>& key)
  {
    digestAlgorithm_ = DIGEST_ALGORITHM_SHA256;
    setPibKey(key);
  }

  /**
   * Create a SigningInfo from its string representation.
   * The digest algorithm is set to DIGEST_ALGORITHM_SHA256.
   * @param signingString The representative signing string for the signing
   * method, as follows:
   * Default signing: "" (the empty string).
   * Signing with the default certificate of the default key for the identity
   * with the specified name:
   * `id:/my-identity`.
   * Signing with the default certificate of the key with the specified name:
   * `key:/my-identity/ksk-1`.
   * Signing with the certificate with the specified name:
   * `cert:/my-identity/KEY/ksk-1/ID-CERT/%FD%01`.
   * Signing with sha256 digest: `id:/localhost/identity/digest-sha256` (the
   * value returned by getDigestSha256Identity()).
   * @throws std::invalid_argument if the signingString format is invalid.
   */
  SigningInfo(const std::string& signingString);

  /**
   * Set this to type SIGNER_TYPE_ID and an identity with name identityName.
   * This does not change the digest algorithm.
   * @param identityName The name of the identity. This copies the Name.
   * @return This SigningInfo.
   */
  SigningInfo&
  setSigningIdentity(const Name& identityName)
  {
    reset(SIGNER_TYPE_ID);
    name_ = identityName;
    return *this;
  }

  /**
   * Set this to type SIGNER_TYPE_KEY and a key with name keyName.
   * This does not change the digest algorithm.
   * @param keyName The name of the key. This copies the Name.
   * @return This SigningInfo.
   */
  SigningInfo&
  setSigningKeyName(const Name& keyName)
  {
    reset(SIGNER_TYPE_KEY);
    name_ = keyName;
    return *this;
  }

  /**
   * Set this to type SIGNER_TYPE_CERT and a certificate with name
   * certificateName. This does not change the digest algorithm.
   * @param certificateName The name of the certificate. This copies the Name.
   * @return This SigningInfo.
   */
  SigningInfo&
  setSigningCertificateName(const Name& certificateName)
  {
    reset(SIGNER_TYPE_CERT);
    name_ = certificateName;
    return *this;
  }

  /**
   * Set this to type SIGNER_TYPE_SHA256, and set the digest algorithm to
   * DIGEST_ALGORITHM_SHA256.
   * @return This SigningInfo.
   */
  SigningInfo&
  setSha256Signing()
  {
    reset(SIGNER_TYPE_SHA256);
    digestAlgorithm_ = DIGEST_ALGORITHM_SHA256;
    return *this;
  }

  /**
   * Set this to type SIGNER_TYPE_ID according to the given PibIdentity.
   * This does not change the digest algorithm.
   * @param identity An existing PibIdentity which is not copied, or a null
   * PibIdentity. If this is a null ptr_lib::shared_ptr<PibIdentity>() then
   * use the default identity, otherwise use identity->getName().
   * @return This SigningInfo.
   */
  SigningInfo&
  setPibIdentity(const ptr_lib::shared_ptr<PibIdentity>& identity)
  {
    reset(SIGNER_TYPE_ID);
    name_ = identity ? identity->getName() : Name();
    identity_ = identity;
    return *this;
  }

  /**
   * Set this to type SIGNER_TYPE_KEY according to the given PibKey.
   * This does not change the digest algorithm.
   * @param key An existing PibKey which is not copied, or a null PibKey. If
   * this is a null ptr_lib::shared_ptr<PibKey>() then use the default
   * key for the identity, otherwise use key->getName().
   * @return This SigningInfo.
   */
  SigningInfo&
  setPibKey(const ptr_lib::shared_ptr<PibKey>& key)
  {
    reset(SIGNER_TYPE_KEY);
    name_ = key ? key->getName() : Name();
    key_ = key;
    return *this;
  }

  /**
   * Get the type of the signer.
   * @return The type of the signer
   */
  SignerType
  getSignerType() const { return type_; }

  /**
   * Get the name of signer.
   * @return The name of signer. The interpretation differs based on the
   * signerType.
   */
  const Name&
  getSignerName() const { return name_; }

  /**
   * Get the PibIdentity of the signer.
   * @return The PibIdentity handler of the signer, or a null
   * ptr_lib::shared_ptr<PibIdentity>() if getSignerName() should be used to
   * find the identity.
   * @throws std::runtime_error if the signer type is not SIGNER_TYPE_ID.
   */
  const ptr_lib::shared_ptr<PibIdentity>&
  getPibIdentity() const;

  /**
   * Get the PibKey of the signer.
   * @return The PibKey handler of the signer, or a null
   * ptr_lib::shared_ptr<PibKey>() if getSignerName() should be used to find the
   * key.
   * @throws std::runtime_error if the signer type is not SIGNER_TYPE_KEY.
   */
  const ptr_lib::shared_ptr<PibKey>&
  getPibKey() const;

  /**
   * Set the digest algorithm for public key operations.
   * @param digestAlgorithm The digest algorithm.
   * @return This SigningInfo.
   */
  SigningInfo&
  setDigestAlgorithm(DigestAlgorithm digestAlgorithm)
  {
    digestAlgorithm_ = digestAlgorithm;
    return *this;
  }

  /**
   * Get the digest algorithm for public key operations.
   * @return The digest algorithm.
   */
  DigestAlgorithm
  getDigestAlgorithm() const { return digestAlgorithm_; }

  /**
   * Set the validity period for the signature info.
   * Note that the equivalent ndn-cxx method uses a semi-prepared SignatureInfo,
   * but this method only uses the ValidityPeriod from the SignatureInfo.
   * @param validityPeriod The validity period, which is copied.
   * @return This SigningInfo.
   */
  SigningInfo&
  setValidityPeriod(const ValidityPeriod& validityPeriod)
  {
    validityPeriod_ = validityPeriod;
    return *this;
  }

  /**
   * Get the validity period for the signature info.
   * Note that the equivalent ndn-cxx method uses a semi-prepared SignatureInfo,
   * but this method only uses the ValidityPeriod from the SignatureInfo.
   * @return The validity period.
   */
  const ValidityPeriod&
  getValidityPeriod() const { return validityPeriod_; }

  /**
   * Get the string representation of this SigningInfo.
   * @return The string representation.
   */
  std::string
  toString() const;

  /**
   * Get the localhost identity which indicates that the signature is generated
   * using SHA-256.
   * @return A new Name of the SHA-256 identity.
   */
  static Name
  getDigestSha256Identity() { return Name("/localhost/identity/digest-sha256"); }

private:
  /**
   * Check and set the signerType, and set others to default values. This does
   * NOT reset the digest algorithm.
   * @param signerType The The type of signer.
   */
  void
  reset(SignerType signerType);

  SignerType type_;
  Name name_;
  ptr_lib::shared_ptr<PibIdentity> identity_;
  ptr_lib::shared_ptr<PibKey> key_;
  DigestAlgorithm digestAlgorithm_;
  ValidityPeriod validityPeriod_;
};

inline std::ostream&
operator << (std::ostream& os, const SigningInfo& info)
{
  os << info.toString();
  return os;
}


}

#endif
