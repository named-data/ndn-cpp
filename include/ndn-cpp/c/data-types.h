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

#ifndef NDN_DATA_TYPES_H
#define NDN_DATA_TYPES_H

#include "key-types.h"
#include "publisher-public-key-digest-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ndn_ContentType defines constants for the MetaInfo "type" field.  Note that the constants for BLOB, LINK and KEY
 * are the same as defined in the NDN-TLV spec.
 */
typedef enum {
  ndn_ContentType_BLOB = 0,
  // ContentType DATA is deprecated.  Use ndn_ContentType_BLOB.
  ndn_ContentType_DATA = 0,
  ndn_ContentType_LINK = 1,
  ndn_ContentType_KEY =  2,
  // ContentType ENCR, GONE and NACK are not supported in NDN-TLV encoding and are deprecated.
  ndn_ContentType_ENCR = 3,
  ndn_ContentType_GONE = 4,
  ndn_ContentType_NACK = 5
} ndn_ContentType;

/** ndn_SignatureType defines constants for the Signature "type" field.
 * Note that the constants are the same as defined in the NDN-TLV spec, but we
 * keep a separate enum so that we aren't directly tied to the TLV code.
 */
typedef enum {
  ndn_SignatureType_DigestSha256Signature = 0,
  ndn_SignatureType_Sha256WithRsaSignature = 1,
  ndn_SignatureType_Sha256WithEcdsaSignature = 3,
  ndn_SignatureType_HmacWithSha256Signature = 4
} ndn_SignatureType;

/**
 * An ndn_Signature struct holds the signature bits and other info representing
 * the signature in a data packet or signed interest. We use one structure which
 * is a union of all the fields from the different known signature types. This
 * lets us avoid the infrastructure to simulate an abstract base class with
 * subclasses and virtual methods.
 */
struct ndn_Signature {
  ndn_SignatureType type;          /**< -1 for none */
  struct ndn_Blob digestAlgorithm; /**< A Blob whose value is a pointer to a pre-allocated buffer.  0 for none.
                                    *   If none, default is 2.16.840.1.101.3.4.2.1 (sha-256). */
  struct ndn_Blob witness;         /**< A Blob whose value is a pointer to pre-allocated buffer.  0 for none. */
  struct ndn_Blob signature;
  /** @deprecated.  The Signature publisherPublicKeyDigest is deprecated.  If you need a publisher public key digest,
   * set the keyLocator keyLocatorType to KEY_LOCATOR_DIGEST and set its key data to the digest. */
  struct ndn_PublisherPublicKeyDigest publisherPublicKeyDigest;
  struct ndn_KeyLocator keyLocator;
};

/**
 * An ndn_MetaInfo struct holds the meta info which is signed inside the data packet.
 */
struct ndn_MetaInfo {
  ndn_MillisecondsSince1970 timestampMilliseconds; /**< milliseconds since 1/1/1970. -1 for none */
  ndn_ContentType type;                  /**< default is ndn_ContentType_DATA. -1 for none */
  ndn_Milliseconds freshnessPeriod;      /**< -1 for none */
  struct ndn_NameComponent finalBlockId; /**< has a pointer to a pre-allocated buffer.  0 for none */
};

struct ndn_Data {
  struct ndn_Signature signature;
  struct ndn_Name name;
  struct ndn_MetaInfo metaInfo;
  struct ndn_Blob content;     /**< A Blob with a pointer to the content. */
};

#ifdef __cplusplus
}
#endif

#endif
