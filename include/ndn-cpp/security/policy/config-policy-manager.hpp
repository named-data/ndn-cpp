/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
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
#include <ndn-cpp/security/policy/certificate-cache.hpp>
#include "policy-manager.hpp"

class TestVerificationRulesFriend;

namespace ndn {

class BoostInfoTree;
class BoostInfoParser;
class IdentityCertificate;

/**
 * A ConfigPolicyManager manages trust according to a configuration file in the
 * Validator Configuration File Format
 * (http://redmine.named-data.net/projects/ndn-cxx/wiki/CommandValidatorConf)
 *
 * Once a rule is matched, the ConfigPolicyManager looks in the
 * CertificateCache for the IdentityCertificate matching the name in the KeyLocator
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
   * Create a new ConfigPolicyManager which acts on the rules specified in the
   * configuration file and downloads unknown certificates when necessary.
   * @param configFileName The path to the configuration file containing
   * verification rules.
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
   * @param maxTrackedKeys The maximum number of public key use timestamps to
   * track.
   */
  ConfigPolicyManager
    (const std::string& configFileName,
     const ptr_lib::shared_ptr<CertificateCache>& certificateCache =
     ptr_lib::shared_ptr<CertificateCache>(), int searchDepth = 5,
     Milliseconds graceInterval = 3000, Milliseconds keyTimestampTtl = 3600000,
     int maxTrackedKeys = 1000);

  /**
   * The virtual destructor.
   */
  virtual
  ~ConfigPolicyManager();

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
   * @param onVerifyFailed If the signature check fails, this calls
   * onVerifyFailed(data).
   * @return the indication of next verification step, null if there is no
   * further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Data>& data, int stepCount,
     const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed);

  /**
   * Check whether the received signed interest complies with the verification
   * policy, and get the indication of the next verification step.
   * @param interest The interest with the signature to check.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param onVerified If the signature is verified, this calls onVerified(interest).
   * @param onVerifyFailed If the signature check fails, this calls
   * onVerifyFailed(interest).
   * @return the indication of next verification step, null if there is no
   * further step.
   */
  virtual ptr_lib::shared_ptr<ValidationRequest>
  checkVerificationPolicy
    (const ptr_lib::shared_ptr<Interest>& interest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat);

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
  // Allow the unit tests to call private members.
  friend class ::TestVerificationRulesFriend;

  /**
   * TrustAnchorRefreshManager manages the trust-anchor certificates, including
   *   refresh.
   */
  class TrustAnchorRefreshManager {
  public:
    TrustAnchorRefreshManager()
    {
    }

    static ptr_lib::shared_ptr<IdentityCertificate>
    loadIdentityCertificateFromFile(const std::string& filename);

    ptr_lib::shared_ptr<IdentityCertificate>
    getCertificate(Name certificateName) const
    {
      // Assume the timestamp is already removed.
      return certificateCache_.getCertificate(certificateName);
    }

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

    CertificateCache certificateCache_;
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
   * @return True if matches.
   */
  bool
  checkSignatureMatch
    (const Name& signatureName, const Name& objectName, const BoostInfoTree& rule);

  /**
   * This looks up certificates specified as base64-encoded data or file names.
   * These are cached by filename or encoding to avoid repeated reading of files
   * or decoding.
   * @param certID
   * @param isPath
   * @return
   */
  ptr_lib::shared_ptr<IdentityCertificate>
  lookupCertificate(const std::string& certID, bool isPath);

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
   * @return A shared_ptr for the Signature object. This is null if can't decode.
   */
  static ptr_lib::shared_ptr<Signature>
  extractSignature(const Interest& interest, WireFormat& wireFormat);

  /**
   * Determine whether the timestamp from the interest is newer than the last
   * use of this key, or within the grace interval on first use.
   * @param keyName The name of the public key used to sign the interest.
   * @param timestamp The timestamp extracted from the interest name.
   * @return True if timestamp is fresh as described above.
   */
  bool
  interestTimestampIsFresh(const Name& keyName, MillisecondsSince1970 timestamp) const;

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
   * @return True if the signature verifies, False if not.
   */
  bool
  verify(const Signature* signatureInfo, const SignedBlob& signedBlob) const;

  /**
   * This is a helper for checkVerificationPolicy to verify the rule and return
   * a certificate interest to fetch the next certificate in the hierarchy if
   * needed.
   * @param stepCount The number of verification steps that have been done, used
   * to track the verification progress.
   * @param matchType Either "data" or "interest".
   * @param objectName The name of the data or interest packet.
   * @param signature The Signature object for the data or interest packet.
   * @return A null object if validation failed, otherwise the interest for the
   * ValidationRequest to fetch the next certificate. However, if the interest
   * has an empty name, the validation succeeded and no need to fetch a
   * certificate.
   */
  ptr_lib::shared_ptr<Interest>
  getCertificateInterest
    (int stepCount, const std::string& matchType, const Name& objectName,
     const Signature* signature);

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
   * @param onVerifyFailed The value from checkVerificationPolicy.
   */
  void
  onCertificateDownloadComplete
    (const ptr_lib::shared_ptr<Data> &data,
     const ptr_lib::shared_ptr<Data> &originalData, int stepCount,
     const OnVerified& onVerified, const OnVerifyFailed& onVerifyFailed);

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
   * @param onVerifyFailed The value from checkVerificationPolicy.
   */
  void
  onCertificateDownloadCompleteForInterest
    (const ptr_lib::shared_ptr<Data> &data,
     const ptr_lib::shared_ptr<Interest> &originalInterest, int stepCount,
     const OnVerifiedInterest& onVerified,
     const OnVerifyInterestFailed& onVerifyFailed, WireFormat& wireFormat);

  ptr_lib::shared_ptr<CertificateCache> certificateCache_;
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
  TrustAnchorRefreshManager refreshManager_;
};

}

#endif
