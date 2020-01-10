/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN config_policy_manager.py by Adeola Bannis.
 * Originally from Yingdi Yu <http://irl.cs.ucla.edu/~yingdi/>.
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

#ifndef NDN_CONFIG_POLICY_MANAGER_HPP
#define NDN_CONFIG_POLICY_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include "certificate-cache.hpp"
#include "../v2/certificate-cache-v2.hpp"
#include "policy-manager.hpp"

// Give friend access to the tests.
class TestVerificationRules_NameRelation_Test;
class TestVerificationRules_SimpleRegex_Test;
class TestVerificationRules_Hierarchical_Test;
class TestVerificationRules_HyperRelation_Test;

namespace ndn {

class BoostInfoTree;
class BoostInfoParser;
class IdentityCertificate;

/**
 * A ConfigPolicyManager manages trust according to a configuration file in the
 * Validator Configuration File Format. See:
 * http://named-data.net/doc/ndn-cxx/current/tutorials/security-validator-config.html
 *
 * Once a rule is matched, the ConfigPolicyManager looks in the
 * certificate cache for the certificate matching the name in the KeyLocator
 * and uses its public key to verify the data packet or signed interest. If the
 * certificate can't be found, it is downloaded, verified and installed. A chain
 * of certificates will be followed to a maximum depth.
 * If the new certificate is accepted, it is used to complete the verification.
 *
 * The KeyLocators of data packets and signed interests MUST contain a name for
 * verification to succeed.
 */
class ConfigPolicyManager : public PolicyManager {
public:
  /**
   * Create a new ConfigPolicyManager which will act on the rules specified in
   * the configuration and download unknown certificates when necessary.
   * This creates a security v1 PolicyManager to verify certificates in format
   * v1. To verify certificate format v2, use the ConfigPolicyManager with a
   * CertificateCacheV2.
   * @param configFileName (optional) If not empty, the path to the
   * configuration file containing verification rules. Otherwise, you should
   * separately call load().
   * @param certificateCache (optional) A CertificateCache to hold known
   * certificates. If this is null or omitted, then create an internal
   * CertificateCache.
   * @param searchDepth (optional) The maximum number of links to follow when
   * verifying a certificate chain.
   * @param graceInterval (optional) The window of time difference (in milliseconds)
   * allowed between the timestamp of the first interest signed with a new
   * public key and the validation time. If omitted, use a default value.
   * @param keyTimestampTtl (optional) How long a public key's last-used
   * timestamp is kept in the store (milliseconds). If omitted, use a default
   * value.
   * @param maxTrackedKeys (optional) The maximum number of public key use
   * timestamps to track. If omitted, use a default.
   */
  ConfigPolicyManager
    (const std::string& configFileName = "",
     const ptr_lib::shared_ptr<CertificateCache>& certificateCache =
     ptr_lib::shared_ptr<CertificateCache>(), int searchDepth = 5,
     Milliseconds graceInterval = 3000, Milliseconds keyTimestampTtl = 3600000,
     int maxTrackedKeys = 1000);

  /**
   * Create a new ConfigPolicyManager which will act on the rules specified in
   * the configuration and download unknown certificates when necessary. This
   * uses certificate format v2.
   * @param configFileName If not empty, the path to the configuration file
   * containing verification rules. Otherwise, you can give an empty string and
   * separately call load().
   * @param certificateCache A CertificateCacheV2 to hold known certificates.
   * @param searchDepth (optional) The maximum number of links to follow when
   * verifying a certificate chain.
   * @param graceInterval (optional) The window of time difference (in milliseconds)
   * allowed between the timestamp of the first interest signed with a new
   * public key and the validation time. If omitted, use a default value.
   * @param keyTimestampTtl (optional) How long a public key's last-used
   * timestamp is kept in the store (milliseconds). If omitted, use a default
   * value.
   * @param maxTrackedKeys (optional) The maximum number of public key use
   * timestamps to track. If omitted, use a default.
   */
  ConfigPolicyManager
    (const std::string& configFileName,
     const ptr_lib::shared_ptr<CertificateCacheV2>& certificateCache,
     int searchDepth = 5, Milliseconds graceInterval = 3000,
     Milliseconds keyTimestampTtl = 3600000, int maxTrackedKeys = 1000);

  /**
   * The virtual destructor.
   */
  virtual
  ~ConfigPolicyManager();

  /**
   * Reset the certificate cache and other fields to the constructor state.
   */
  void
  reset();

  /**
   * Call reset() and load the configuration rules from the file.
   * @param configFileName The path to the configuration file containing the
   * verification rules.
   */
  void
  load(const std::string& configFileName);

  /**
   * Call reset() and load the configuration rules from the input.
   * @param input The contents of the configuration rules, with lines separated
   * by "\n" or "\r\n".
   * @param inputName Used for log messages, etc.
   */
  void
  load(const std::string& input, const std::string& inputName);

  /**
   * Check if the received data packet can escape from verification and be
   * trusted as valid. If the configuration file contains the trust anchor
   * 'any', nothing is verified.
   * @param data The received data packet.
   * @return true if the data does not need to be verified to be trusted as
   * valid, otherwise false.
   */
  virtual bool
  skipVerifyAndTrust(const Data& data);

  /**
   * Check if the received signed interest can escape from verification and be
   * trusted as valid. If the configuration file contains the trust anchor
   * 'any', nothing is verified.
   * @param interest The received interest.
   * @return true if the interest does not need to be verified to be trusted as
   * valid, otherwise false.
   */
  virtual bool
  skipVerifyAndTrust(const Interest& interest);

  /**
   * Check if this PolicyManager has a verification rule for the received data.
   * If the configuration file contains the trust anchor 'any', nothing is
   * verified.
   * @param data The received data packet.
   * @return true if the data must be verified, otherwise false.
   */
  virtual bool
  requireVerify(const Data& data);

  /**
   * Check if this PolicyManager has a verification rule for the received signed
   * interest.
   * If the configuration file contains the trust anchor 'any', nothing is
   * verified.
   * @param interest The received interest.
   * @return true if the interest must be verified, otherwise false.
   */
  virtual bool
  requireVerify(const Interest& interest);

  /**
   * Check whether the received data packet complies with the verification policy,
   * and get the indication of the next verification step.
   * @param data The Data object with the signature to check.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param onVerified If the signature is verified, this calls onVerified(data).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onValidationFailed If the signature check fails, this calls
   * onValidationFailed(data, reason).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return the indication of next verification step, null if there is no
   * further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Data>& data, int stepCount,
     const OnVerified& onVerified,
     const OnDataValidationFailed& onValidationFailed);

  /**
   * Check whether the received signed interest complies with the verification
   * policy, and get the indication of the next verification step.
   * @param interest The interest with the signature to check.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param onVerified If the signature is verified, this calls onVerified(interest).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @param onValidationFailed If the signature check fails, this calls
   * onValidationFailed(interest, reason).
   * NOTE: The library will log any exceptions thrown by this callback, but for
   * better error handling the callback should catch and properly handle any
   * exceptions.
   * @return the indication of next verification step, null if there is no
   * further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnInterestValidationFailed& onValidationFailed,
     WireFormat& wireFormat);

  /**
   * Override to always indicate that the signing certificate name and data name
   * satisfy the signing policy.
   * @param dataName The name of data to be signed.
   * @param certificateName The name of signing certificate.
   * @return true to indicate that the signing certificate can be used to sign
   * the data.
   */
  virtual bool
  checkSigningPolicy(const Name& dataName, const Name& certificateName);

  /**
   * Infer the signing identity name according to the policy. If the signing
   * identity cannot be inferred, return an empty name.
   * @param dataName The name of data to be signed.
   * @return The signing identity or an empty name if cannot infer.
   */
  virtual Name
  inferSigningIdentity(const Name& dataName);

private:
  // Give friend access to the tests.
  friend class ::TestVerificationRules_NameRelation_Test;
  friend class ::TestVerificationRules_SimpleRegex_Test;
  friend class ::TestVerificationRules_Hierarchical_Test;
  friend class ::TestVerificationRules_HyperRelation_Test;

  /**
   * TrustAnchorRefreshManager manages the trust-anchor certificates, including
   *   refresh.
   */
  class TrustAnchorRefreshManager {
  public:
    TrustAnchorRefreshManager(bool isSecurityV1)
    : isSecurityV1_(isSecurityV1)
    {
    }

    static ptr_lib::shared_ptr<IdentityCertificate>
    loadIdentityCertificateFromFile(const std::string& filename);

    static ptr_lib::shared_ptr<CertificateV2>
    loadCertificateV2FromFile(const std::string& filename);

    ptr_lib::shared_ptr<IdentityCertificate>
    getCertificate(Name certificateName) const;

    ptr_lib::shared_ptr<CertificateV2>
    getCertificateV2(Name certificateName) const;

    void
    addDirectory(const std::string& directoryName, Milliseconds refreshPeriod);

    void
    refreshAnchors();

  private:
    class DirectoryInfo {
    public:
      DirectoryInfo
        (const std::vector<std::string>& certificateNames,
         MillisecondsSince1970 nextRefresh, Milliseconds refreshPeriod)
      : certificateNames_(certificateNames), nextRefresh_(nextRefresh),
        refreshPeriod_(refreshPeriod)
      {
      }

      std::vector<std::string> certificateNames_;
      MillisecondsSince1970 nextRefresh_;
      Milliseconds refreshPeriod_;
    };

    bool isSecurityV1_;
    CertificateCache certificateCache_;
    CertificateCacheV2 certificateCacheV2_;
    // refreshDirectories_ maps the directory name to certificate names so they
    //   can be deleted when necessary, and the next refresh time.
    std::map<std::string, ptr_lib::shared_ptr<DirectoryInfo> > refreshDirectories_;
  };

  /**
   * The configuration file allows 'trust anchor' certificates to be preloaded.
   * The certificates may also be loaded from a directory, and if the 'refresh'
   * option is set to an interval, the certificates are reloaded at the
   * specified interval
   */
  void
  loadTrustAnchorCertificates();

  /**
   * Once a rule is found to match data or a signed interest, the name in the
   * KeyLocator must satisfy the condition in the 'checker' section of the rule,
   * else the data or interest is rejected.
   * @param signatureName The certificate name from the KeyLocator.
   * @param objectName The name of the data packet or interest. In the case of
   * signed interests, this excludes the timestamp, nonce and signature
   * components.
   * @param rule The rule from the configuration file that matches the data or
   * interest.
   * @param failureReason If verification fails, set failureReason to the
   * failure reason.
   * @return True if matches.
   */
  bool
  checkSignatureMatch
    (const Name& signatureName, const Name& objectName,
     const BoostInfoTree& rule, std::string& failureReason);

  /**
   * This looks up certificates specified as base64-encoded data or file names.
   * These are cached by filename or encoding to avoid repeated reading of files
   * or decoding.
   * @param certID
   * @param isPath
   * @return The IdentityCertificate or null if not found.
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  lookupCertificate(const std::string& certID, bool isPath);

  /**
   * This looks up certificates specified as base64-encoded data or file names.
   * These are cached by filename or encoding to avoid repeated reading of files
   * or decoding.
   * @param certID
   * @param isPath
   * @return The CertificateV2 or null if not found.
   */
  ptr_lib::shared_ptr<CertificateV2>
  lookupCertificateV2(const std::string& certID, bool isPath);

  /**
   * Search the configuration file for the first rule that matches the data or
   * signed interest name. In the case of interests, the name to match should
   * exclude the timestamp, nonce, and signature components.
   * @param objName The name to be matched.
   * @param matchType The rule type to match, "data" or "interest".
   * @return A pointer to the BoostInfoTree for the matching rule, or 0
   *   if not found.
   */
  const BoostInfoTree*
  findMatchingRule(const Name& objName, const std::string& matchType) const;

  /**
   * Determines if a name satisfies the relation to another name, based on
   * matchRelation.
   * @param name
   * @param matchName
   * @param matchRelation  Can be one of:
      "is-prefix-of" - passes if the name is equal to or has the other
         name as a prefix
      "is-strict-prefix-of" - passes if the name has the other name as a
         prefix, and is not equal
      "equal" - passes if the two names are equal
   * @return True if matches.
   */
  static bool
  matchesRelation
    (const Name& name, const Name& matchName, const std::string& matchRelation);

  /**
   * Extract the signature information from the interest name.
   * @param interest The interest whose signature is needed.
   * @param wireFormat The wire format used to decode signature information
   * from the interest name.
   * @param failureReason If can't decode, set failureReason to the failure
   * reason.
   * @return A shared_ptr for the Signature object. This is null if can't decode.
   */
  static ptr_lib::shared_ptr<Signature>
  extractSignature
    (const Interest& interest, WireFormat& wireFormat,
     std::string& failureReason);

  /**
   * Determine whether the timestamp from the interest is newer than the last
   * use of this key, or within the grace interval on first use.
   * @param keyName The name of the public key used to sign the interest.
   * @param timestamp The timestamp extracted from the interest name.
   * @param failureReason If timestamp is not fresh, set failureReason to the
   * failure reason.
   * @return True if timestamp is fresh as described above.
   */
  bool
  interestTimestampIsFresh
    (const Name& keyName, MillisecondsSince1970 timestamp,
     std::string& failureReason) const;

  /**
   * Trim the table size down if necessary, and insert/update the latest
   * interest signing timestamp for the key. Any key which has not been used
   * within the TTL period is purged. If the table is still too large, the
   * oldest key is purged.
   * @param keyName The name of the public key used to sign the interest.
   * @param timestamp The timestamp extracted from the interest name.
   */
  void
  updateTimestampForKey(const Name& keyName, MillisecondsSince1970 timestamp);

  /**
   * Check the type of signatureInfo to get the KeyLocator. Look in the
   * IdentityStorage for the public key with the name in the KeyLocator and use
   * it to verify the signedBlob. If the public key can't be found, return
   * false. (This is a generalized method which can verify both a Data packet
   * and an interest.)
   * @param signatureInfo An object of a subclass of Signature, e.g.
   * Sha256WithRsaSignature.
   * @param signedBlob the SignedBlob with the signed portion to verify.
   * @param failureReason If verification fails, set failureReason to the
   * failure reason.
   * @return True if the signature verifies, False if not.
   */
  bool
  verify
    (const Signature* signatureInfo, const SignedBlob& signedBlob,
     std::string& failureReason) const;

  /**
   * This is a helper for checkVerificationPolicy to verify the rule and return
   * a certificate interest to fetch the next certificate in the hierarchy if
   * needed.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param matchType Either "data" or "interest".
   * @param objectName The name of the data or interest packet.
   * @param signature The Signature object for the data or interest packet.
   * @param failureReason If can't determine the interest, set failureReason
   * to the failure reason.
   * @return A null object if if can't determine the interest, otherwise the
   * interest for the ValidationRequest to fetch the next certificate. However,
   * if the interest has an empty name, the validation succeeded and no need to
   * fetch a certificate.
   */
  ptr_lib::shared_ptr<Interest>
  getCertificateInterest
    (int stepCount, const std::string& matchType, const Name& objectName,
     const Signature* signature, std::string& failureReason);

  /**
   * This is called by KeyChain::verifyData because checkVerificationPolicy
   * returned a ValidationRequest to fetch a certificate and verify
   * a certificate, through a separate call to KeyChain::verifyData. When
   * it verifies the data, it calls onVerified which is this method.
   * @param data The fetched data packet containing the certificate which has
   * already been verified.
   * @param originalData The original data from checkVerificationPolicy.
   * @param stepCount The value from checkVerificationPolicy.
   * @param onVerified The value from checkVerificationPolicy.
   * @param onValidationFailed The value from checkVerificationPolicy.
   */
  void
  onCertificateDownloadComplete
    (const ptr_lib::shared_ptr<Data> &data,
     const ptr_lib::shared_ptr<Data> &originalData, int stepCount,
     const OnVerified& onVerified,
     const OnDataValidationFailed& onValidationFailed);

  /**
   * This is called by KeyChain::verifyData because checkVerificationPolicy
   * returned a ValidationRequest to fetch a certificate and verify
   * a certificate, through a separate call to KeyChain::verifyData. When
   * it verifies the data, it calls onVerified which is this method.
   * @param data The fetched data packet containing the certificate which has
   * already been verified.
   * @param originalInterest The original interest from checkVerificationPolicy.
   * @param stepCount The value from checkVerificationPolicy.
   * @param onVerified The value from checkVerificationPolicy.
   * @param onValidationFailed The value from checkVerificationPolicy.
   */
  void
  onCertificateDownloadCompleteForInterest
    (const ptr_lib::shared_ptr<Data> &data,
     const ptr_lib::shared_ptr<Interest> &originalInterest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnInterestValidationFailed& onValidationFailed,
     WireFormat& wireFormat);

  bool isSecurityV1_;
  ptr_lib::shared_ptr<CertificateCache> certificateCache_;
  ptr_lib::shared_ptr<CertificateCacheV2> certificateCacheV2_;
  int maxDepth_;
  Milliseconds keyGraceInterval_;
  Milliseconds keyTimestampTtl_;
  int maxTrackedKeys_;
  // fixedCertificateCache_ stores the fixed-signer certificate name associated with
  //    validation rules so we don't keep loading from files.
  std::map<std::string, std::string> fixedCertificateCache_;
  // keyTimestamps_ stores the timestamps for each public key used in command
  //   interests to avoid replay attacks.
  // key is the public key name, value is the last timestamp.
  std::map<std::string, MillisecondsSince1970> keyTimestamps_;
  ptr_lib::shared_ptr<BoostInfoParser> config_;
  bool requiresVerification_;
  ptr_lib::shared_ptr<TrustAnchorRefreshManager> refreshManager_;
};

}

#endif
