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
