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

#ifndef NDN_INTEREST_H
#define NDN_INTEREST_H

#include <ndn-cpp/c/interest-types.h>
#include "name.h"
#include "publisher-public-key-digest.h"
#include "key-locator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param self pointer to the ndn_NameComponent struct
 * @param type one of the ndn_ExcludeType enum
 * @param component the pre-allocated buffer for the component value, only used if type is ndn_Exclude_COMPONENT
 * @param componentLength the number of bytes in value, only used if type is ndn_Exclude_COMPONENT
 */
static __inline void ndn_ExcludeEntry_initialize(struct ndn_ExcludeEntry *self, ndn_ExcludeType type, const uint8_t *component, size_t componentLength)
{
  self->type = type;
  ndn_NameComponent_initialize(&self->component, component, componentLength);
}

/**
 * Initialize an ndn_Exclude struct with the entries array.
 * @param self A pointer to the ndn_Exclude struct.
 * @param entries the pre-allocated array of ndn_ExcludeEntry
 * @param maxEntries the number of elements in the allocated entries array
 */
static __inline void ndn_Exclude_initialize(struct ndn_Exclude *self, struct ndn_ExcludeEntry *entries, size_t maxEntries)
{
  self->entries = entries;
  self->maxEntries = maxEntries;
  self->nEntries = 0;
}

/**
 * Append a new entry of type ndn_Exclude_ANY.
 * @param self A pointer to the ndn_Exclude struct.
 * @return 0 for success, or an error code if there is no more room in the
 * entries array (nEntries is already maxEntries).
 */
ndn_Error
ndn_Exclude_appendAny(struct ndn_Exclude *self);

/**
 * Append a new entry of type ndn_Exclude_COMPONENT with the given component
 * value.
 * @param self A pointer to the ndn_Exclude struct.
 * @param component The bytes of the component.  This does not copy the bytes.
 * @param componentLength The number of bytes in component.
 * @return 0 for success, or an error code if there is no more room in the
 * entries array (nEntries is already maxEntries).
 */
ndn_Error
ndnExclude_appendComponent
  (struct ndn_Exclude *self, const uint8_t* component, size_t componentLength);

/**
 * Compare the components using NDN component ordering.
 * A component is less if it is shorter, otherwise if equal length do a byte comparison.
 * @param component1 A pointer to the first name component.
 * @param component2 A pointer to the second name component.
 * @return -1 if component1 is less than component2, 1 if greater or 0 if equal.
 */
int ndn_Exclude_compareComponents(struct ndn_NameComponent *component1, struct ndn_NameComponent *component2);

/**
 * Initialize an ndn_Interest struct with the pre-allocated nameComponents and excludeEntries,
 * and defaults for all the values.
 * @param self pointer to the ndn_Interest struct
 * @param nameComponents the pre-allocated array of ndn_NameComponent
 * @param maxNameComponents the number of elements in the allocated nameComponents array
 * @param excludeEntries the pre-allocated array of ndn_ExcludeEntry
 * @param maxExcludeEntries the number of elements in the allocated excludeEntries array
 * @param keyNameComponents The pre-allocated array of ndn_NameComponent for the keyLocator.
 * @param maxKeyNameComponents The number of elements in the allocated keyNameComponents array.
 */
static __inline void ndn_Interest_initialize
  (struct ndn_Interest *self, struct ndn_NameComponent *nameComponents, size_t maxNameComponents,
   struct ndn_ExcludeEntry *excludeEntries, size_t maxExcludeEntries, struct ndn_NameComponent *keyNameComponents,
   size_t maxKeyNameComponents)
{
  ndn_Name_initialize(&self->name, nameComponents, maxNameComponents);
  self->minSuffixComponents = -1;
  self->maxSuffixComponents = -1;
  ndn_PublisherPublicKeyDigest_initialize(&self->publisherPublicKeyDigest);
  ndn_Exclude_initialize(&self->exclude, excludeEntries, maxExcludeEntries);
  self->childSelector = -1;
  self->answerOriginKind = -1;
  self->scope = -1;
  self->interestLifetimeMilliseconds = -1.0;
  ndn_Blob_initialize(&self->nonce, 0, 0);
  ndn_KeyLocator_initialize(&self->keyLocator, keyNameComponents, maxKeyNameComponents);
}

/**
 * Return true if answerOriginKind indicates that the content must be fresh. If
 * answerOriginKind is not specified, the default is true.
 * @param self A pointer to the ndn_Interest struct.
 * @return 1 if must be fresh, otherwise 0.
 */
static __inline int ndn_Interest_getMustBeFresh(struct ndn_Interest *self)
{
  if (self->answerOriginKind < 0)
    return 1;
  else
    return (self->answerOriginKind & ndn_Interest_ANSWER_STALE) == 0 ? 1 : 0;
}

#ifdef __cplusplus
}
#endif

#endif

