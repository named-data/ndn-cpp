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

#ifndef NDN_KEY_LOCATOR_H
#define NDN_KEY_LOCATOR_H

#include <ndn-cpp/c/key-types.h>
#include "name.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * Initialize an ndn_KeyLocator struct with the pre-allocated nameComponents, and defaults for all the values.
 * @param self A pointer to the ndn_KeyLocator struct.
 * @param keyNameComponents The pre-allocated array of ndn_NameComponent.
 * @param maxKeyNameComponents The number of elements in the allocated keyNameComponents array.
 */
static __inline void ndn_KeyLocator_initialize
  (struct ndn_KeyLocator *self, struct ndn_NameComponent *keyNameComponents, size_t maxKeyNameComponents) {
  self->type = (ndn_KeyLocatorType)-1;
  ndn_Blob_initialize(&self->keyData, 0, 0);
  ndn_Name_initialize(&self->keyName, keyNameComponents, maxKeyNameComponents);
  self->keyNameType = (ndn_KeyNameType)-1;
}

#ifdef __cplusplus
}
#endif

#endif
