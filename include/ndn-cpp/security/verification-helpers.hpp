/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2017-2020 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: From ndn-cxx security https://github.com/named-data/ndn-cxx/blob/master/ndn-cxx/security/verification-helpers.hpp
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

#ifndef NDN_VERIFICATION_HELPERS_HPP
#define NDN_VERIFICATION_HELPERS_HPP

#include "../interest.hpp"
#include "certificate/public-key.hpp"
#include "v2/certificate-v2.hpp"

namespace ndn {

/**
 * The VerificationHelpers class has static methods to verify signatures and
 * digests.
 */
class VerificationHelpers {
public:
  /**
   * Verify the buffer against the signature using the public key.
   * @param buffer A pointer to the input buffer to verify.
   * @param bufferLength The length of the buffer.
   * @param signature A pointer to the signature bytes.
   * @param signatureLength The length of the signature.
   * @param publicKey The object containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifySignature
    (const uint8_t* buffer, size_t bufferLength, const uint8_t* signature,
     size_t signatureLength, const PublicKey& publicKey,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256);

  /**
   * Verify the buffer against the signature using the public key.
   * @param buffer The input buffer to verify.
   * @param signature The signature bytes.
   * @param publicKey The object containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifySignature
    (const Blob& buffer, const Blob& signature, const PublicKey& publicKey,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256)
  {
    return verifySignature
      (buffer.buf(), buffer.size(), signature.buf(), signature.size(),
       publicKey, digestAlgorithm);
  }

  /**
   * Verify the buffer against the signature using the encoded public key.
   * If the public key can't be decoded, this returns false instead of throwing
   * a decoding exception. If you want to get a decoding exception then use
   * the PublicKey constructor to decode and call verifySignature with the
   * PublicKey object.
   * @param buffer A pointer to the input buffer to verify.
   * @param bufferLength The length of the buffer.
   * @param signature A pointer to the signature bytes.
   * @param signatureLength The length of the signature.
   * @param publicKeyDer The DER-encoded public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @return True if verification succeeds, false if verification fails or for
   * an error decoding the public key.
   * @throws invalid_argument for an invalid keyType or digestAlgorithm.
   */
  static bool
  verifySignature
    (const uint8_t* buffer, size_t bufferLength, const uint8_t* signature,
     size_t signatureLength, const Blob& publicKeyDer,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256);

  /**
   * Verify the buffer against the signature using the encoded public key.
   * If the public key can't be decoded, this returns false instead of throwing
   * a decoding exception. If you want to get a decoding exception then use
   * the PublicKey constructor to decode and call verifySignature with the
   * PublicKey object.
   * @param buffer The input buffer to verify.
   * @param signature The signature bytes.
   * @param publicKeyDer The DER-encoded public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @return True if verification succeeds, false if verification fails or for
   * an error decoding the public key.
   * @throws invalid_argument for an invalid keyType or digestAlgorithm.
   */
  static bool
  verifySignature
    (const Blob& buffer, const Blob& signature, const Blob& publicKeyDer,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256)
  {
    return verifySignature
      (buffer.buf(), buffer.size(), signature.buf(), signature.size(),
       publicKeyDer, digestAlgorithm);
  }

  /**
   * Verify the Data packet using the public key. This does not check the
   * type of public key or digest algorithm against the type of SignatureInfo in
   * the Data packet such as Sha256WithRsaSignature.
   * @param data The Data packet to verify.
   * @param publicKey The object containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to encode the Data
   * packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyDataSignature
    (const Data& data, const PublicKey& publicKey,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Verify the Data packet using the public key. This does not check the
   * type of public key or digest algorithm against the type of SignatureInfo in
   * the Data packet such as Sha256WithRsaSignature.
   * If the public key can't be decoded, this returns false instead of throwing
   * a decoding exception. If you want to get a decoding exception then use
   * the PublicKey constructor to decode and call verifyDataSignature with the
   * PublicKey object.
   * @param data The Data packet to verify.
   * @param publicKeyDer The DER-encoded public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to encode the Data
   * packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or for
   * an error decoding the public key.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyDataSignature
    (const Data& data, const Blob& publicKeyDer,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Verify the Data packet using the public key in the certificate. This does
   * not check the type of public key or digest algorithm against the type of
   * SignatureInfo in the Data packet such as Sha256WithRsaSignature.
   * @param data The Data packet to verify.
   * @param certificate The certificate containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to encode the Data
   * packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or for
   * an error decoding the public key.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyDataSignature
    (const Data& data, const CertificateV2& certificate,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return verifyDataSignature
      (data, certificate.getPublicKey(), digestAlgorithm, wireFormat);
  }

  /**
   * Verify the Interest packet using the public key, where the last two name
   * components are the SignatureInfo and signature bytes. This does not check
   * the type of public key or digest algorithm against the type of
   * SignatureInfo such as Sha256WithRsaSignature.
   * @param interest The Interest packet to verify.
   * @param publicKey The object containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to decode the
   * Interest packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or
   * cannot decode the Interest.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyInterestSignature
    (const Interest& interest, const PublicKey& publicKey,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Verify the Interest packet using the public key, where the last two name
   * components are the SignatureInfo and signature bytes. This does not check
   * the type of public key or digest algorithm against the type of
   * SignatureInfo such as Sha256WithRsaSignature.
   * If the public key can't be decoded, this returns false instead of throwing
   * a decoding exception. If you want to get a decoding exception then use
   * the PublicKey constructor to decode and call verifyInterestSignature with
   * the PublicKey object.
   * @param interest The Interest packet to verify.
   * @param publicKeyDer The DER-encoded public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to decode the
   * Interest packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or
   * cannot decode the Interest or public key.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyInterestSignature
    (const Interest& interest, const Blob& publicKeyDer,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Verify the Interest packet using the public key in the certificate, where
   * the last two name components are the SignatureInfo and signature bytes.
   * This does not check the type of public key or digest algorithm against the
   * type of SignatureInfo such as Sha256WithRsaSignature.
   * @param interest The Interest packet to verify.
   * @param certificate The certificate containing the public key.
   * @param digestAlgorithm (optional) The digest algorithm. If omitted, use SHA256.
   * @param wireFormat (optional) A WireFormat object used to decode the
   * Interest packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or
   * cannot decode the Interest or public key.
   * @throws invalid_argument for an invalid public key type or digestAlgorithm.
   */
  static bool
  verifyInterestSignature
    (const Interest& interest, const CertificateV2& certificate,
     DigestAlgorithm digestAlgorithm = DIGEST_ALGORITHM_SHA256,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat())
  {
    return verifyInterestSignature
      (interest, certificate.getPublicKey(), digestAlgorithm, wireFormat);
  }

  /////////////////////////////////////////////////////////////

  /**
   * Verify the buffer against the digest using the digest algorithm.
   * @param buffer A pointer to the input buffer to verify.
   * @param bufferLength The length of the buffer.
   * @param digest A pointer to the digest bytes.
   * @param digestLength The length of the digest.
   * @param digestAlgorithm The digest algorithm, such as DIGEST_ALGORITHM_SHA256.
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid digestAlgorithm.
   */
  static bool
  verifyDigest
    (const uint8_t* buffer, size_t bufferLength, const uint8_t* digest,
     size_t digestLength, DigestAlgorithm digestAlgorithm);

  /**
   * Verify the buffer against the digest using the digest algorithm.
   * @param buffer The input buffer to verify.
   * @param digest The digest bytes.
   * @param digestAlgorithm The digest algorithm, such as DIGEST_ALGORITHM_SHA256.
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid digestAlgorithm.
   */
  static bool
  verifyDigest
    (const Blob& buffer, const Blob& digest, DigestAlgorithm digestAlgorithm)
  {
    return verifyDigest
      (buffer.buf(), buffer.size(), digest.buf(), digest.size(),
       digestAlgorithm);
  }

  /**
   * Verify the Data packet using the digest algorithm. This does not check the
   * digest algorithm against the type of SignatureInfo in the Data packet such
   * as DigestSha256Signature.
   * @param data The Data packet to verify.
   * @param digestAlgorithm The digest algorithm, such as DIGEST_ALGORITHM_SHA256.
   * @param wireFormat (optional) A WireFormat object used to encode the Data
   * packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails.
   * @throws invalid_argument for an invalid digestAlgorithm.
   */
  static bool
  verifyDataDigest
    (const Data& data, DigestAlgorithm digestAlgorithm,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

  /**
   * Verify the Interest packet using the digest algorithm, where the last two
   * name components are the SignatureInfo and signature bytes. This does not
   * check the digest algorithm against the type of SignatureInfo such as
   * DigestSha256Signature.
   * @param interest The Interest packet to verify.
   * @param digestAlgorithm The digest algorithm, such as DIGEST_ALGORITHM_SHA256.
   * @param wireFormat (optional) A WireFormat object used to decode the
   * Interest packet. If omitted, use WireFormat getDefaultWireFormat().
   * @return True if verification succeeds, false if verification fails or
   * cannot decode the Interest.
   * @throws invalid_argument for an invalid digestAlgorithm.
   */
  static bool
  verifyInterestDigest
    (const Interest& interest, DigestAlgorithm digestAlgorithm,
     WireFormat& wireFormat = *WireFormat::getDefaultWireFormat());

private:
  /**
   * Extract the signature information from the interest name.
   * @param interest The interest whose signature is needed.
   * @param wireFormat The wire format used to decode signature information
   * from the interest name.
   * @return A shared_ptr for the Signature object. This is null if can't decode.
   */
  static ptr_lib::shared_ptr<Signature>
  extractSignature(const Interest& interest, WireFormat& wireFormat);
};

}

#endif
