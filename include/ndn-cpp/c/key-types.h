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

#ifndef NDN_KEY_TYPES_H
#define NDN_KEY_TYPES_H

#include "name-types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_KeyLocatorType_KEYNAME = 1,
  ndn_KeyLocatorType_KEY_LOCATOR_DIGEST = 2,
  // KeyLocatorType KEY and CERTIFICATE are not supported in NDN-TLV encoding and are deprecated.
  ndn_KeyLocatorType_KEY = 3,
  ndn_KeyLocatorType_CERTIFICATE = 4
} ndn_KeyLocatorType;

/**
 * @deprecated The use of a digest attached to the KeyName is deprecated.
 */
typedef enum {
  ndn_KeyNameType_PUBLISHER_PUBLIC_KEY_DIGEST = 1,
  ndn_KeyNameType_PUBLISHER_CERTIFICATE_DIGEST = 2,
  ndn_KeyNameType_PUBLISHER_ISSUER_KEY_DIGEST = 3,
  ndn_KeyNameType_PUBLISHER_ISSUER_CERTIFICATE_DIGEST = 4
} ndn_KeyNameType;

/**
 * An ndn_KeyLocator holds the type of key locator and related data.
 */
struct ndn_KeyLocator {
  ndn_KeyLocatorType type;     /**< -1 for none */
  struct ndn_Blob keyData;            /**< A Blob whose value is a pointer to a pre-allocated buffer for the key data as follows:
    *   If type is ndn_KeyLocatorType_KEY, the key data.
    *   If type is ndn_KeyLocatorType_CERTIFICATE, the certificate data.
    *   If type is ndn_KeyLocatorType_KEY_LOCATOR_DIGEST, the digest data.
    *   If type is ndn_KeyLocatorType_KEYNAME and keyNameType is ndn_KeyNameType_PUBLISHER_PUBLIC_KEY_DIGEST, the publisher public key digest.
    *   If type is ndn_KeyLocatorType_KEYNAME and keyNameType is ndn_KeyNameType_PUBLISHER_CERTIFICATE_DIGEST, the publisher certificate digest.
    *   If type is ndn_KeyLocatorType_KEYNAME and keyNameType is ndn_KeyNameType_PUBLISHER_ISSUER_KEY_DIGEST, the publisher issuer key digest.
    *   If type is ndn_KeyLocatorType_KEYNAME and keyNameType is ndn_KeyNameType_PUBLISHER_ISSUER_CERTIFICATE_DIGEST, the publisher issuer certificate digest.
    */
  struct ndn_Name keyName;     /**< The key name (only used if type is ndn_KeyLocatorType_KEYNAME.) */
  /** @deprecated The use of a digest attached to the KeyName is deprecated. */
  ndn_KeyNameType keyNameType; /**< The type of data for keyName, -1 for none. (only used if type is ndn_KeyLocatorType_KEYNAME.) */
};

#ifdef __cplusplus
}
#endif

#endif
