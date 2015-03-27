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

#ifndef NDN_NAME_H
#define NDN_NAME_H

#include <ndn-cpp/c/name-types.h>
#include <ndn-cpp/c/errors.h>
#include "util/blob.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param self pointer to the ndn_NameComponent struct
 * @param value the pre-allocated buffer for the component value
 * @param valueLength the number of bytes in value
 */
static __inline void ndn_NameComponent_initialize(struct ndn_NameComponent *self, const uint8_t *value, size_t valueLength)
{
  ndn_Blob_initialize(&self->value, value, valueLength);
}

/**
 * Interpret the name component as a network-ordered number and return an integer.
 * @param self A pointer to the ndn_NameComponent struct.
 * @return The integer number.
 */
uint64_t ndn_NameComponent_toNumber(const struct ndn_NameComponent *self);

/**
 * Interpret the name component as a network-ordered number with a marker and return an integer.
 * @param self A pointer to the ndn_NameComponent struct.
 * @param marker The required first byte of the component.
 * @param result Return the integer number.
 * @return 0 for success, or an error code if the first byte of the component does not equal the marker.
 */
ndn_Error ndn_NameComponent_toNumberWithMarker
  (const struct ndn_NameComponent *self, uint8_t marker, uint64_t *result);

/**
 * Interpret the name component as a network-ordered number with a prefix and return an integer.
 * @param self A pointer to the ndn_NameComponent struct.
 * @param prefix The required prefix of the component.
 * @param prefixLength The length of prefix.
 * @param result Return the integer number.
 * @return 0 for success, or an error code if the first bytes of the component do not equal the prefix.
 */
ndn_Error ndn_NameComponent_toNumberWithPrefix
  (const struct ndn_NameComponent *self, const uint8_t *prefix,
   size_t prefixLength, uint64_t *result);

/**
 * Compare this component to the other component using NDN component ordering.
 * A component is less if it is shorter, otherwise if equal length do a byte
 * comparison.
 * @param self A pointer this name component.
 * @param other A pointer to the other name component to compare with.
 * @return -1 if self is less than other, 1 if greater or 0 if equal.
 */
int ndn_NameComponent_compare
  (const struct ndn_NameComponent *self, const struct ndn_NameComponent *other);

/**
 * Set this name component to have the values from the other name component.
 * @param self A pointer to this ndn_NameComponent struct.
 * @param other A pointer to the other ndn_NameComponent struct to get values from.
 */
static __inline void
ndn_NameComponent_setFromNameComponent
  (struct ndn_NameComponent *self, const struct ndn_NameComponent *other)
{
  *self = *other;
}

/**
 * Initialize an ndn_Name struct with the components array.
 * @param self pointer to the ndn_Name struct
 * @param components the pre-allocated array of ndn_NameComponent
 * @param maxComponents the number of elements in the allocated components array
 */
static __inline void ndn_Name_initialize(struct ndn_Name *self, struct ndn_NameComponent *components, size_t maxComponents)
{
  self->components = components;
  self->maxComponents = maxComponents;
  self->nComponents = 0;
}

/**
 * Clear all the components.
 * @param self A pointer to the ndn_Name struct.
 */
static __inline void
ndn_Name_clear(struct ndn_Name *self) { self->nComponents = 0; }

/**
 * Return true if the N components of this name are the same as the first N components of the given name.
 * @param self A pointer to the ndn_Name struct.
 * @param name A pointer to the other name to match.
 * @return 1 if this matches the given name, 0 otherwise.  This always returns 1 if this name is empty.
 */
int ndn_Name_match(const struct ndn_Name *self, const struct ndn_Name *name);

/**
 * Append a component to this name with the bytes in the given array.
 * @param self pointer to the ndn_Name struct.
 * @param value The bytes of the component.  This does not copy the bytes.
 * @param valueLength The number of bytes in value.
 * @return 0 for success, or an error code if there is no more room in the components array (nComponents is already maxComponents).
 */
ndn_Error ndn_Name_appendComponent(struct ndn_Name *self, const uint8_t* value, size_t valueLength);

/**
 * Append a component to this name with the bytes in the given blob.
 * @param self pointer to the ndn_Name struct.
 * @param value An ndn_Blob with the bytes of the component.  This does not copy the bytes.
 * @return 0 for success, or an error code if there is no more room in the components array (nComponents is already maxComponents).
 */
static __inline ndn_Error ndn_Name_appendBlob(struct ndn_Name *self, struct ndn_Blob *value)
{
  return ndn_Name_appendComponent(self, value->value, value->length);
}

/**
 * Append a component to this name with the bytes in raw string value.
 * @param self pointer to the ndn_Name struct.
 * @param value The null-terminated string, treated as a byte array.  This does not copy the bytes.
 * @return 0 for success, or an error code if there is no more room in the components array (nComponents is already maxComponents).
 */
ndn_Error ndn_Name_appendString(struct ndn_Name *self, const char * value);

/**
 * Set this name to have the values from the other name.
 * @param self A pointer to the ndn_Name struct.
 * @param other A pointer to the other ndn_Name struct to get values from.
 * @return 0 for success, or an error code if there is not enough room in this
 * object's components array.
 */
ndn_Error
ndn_Name_setFromName(struct ndn_Name *self, const struct ndn_Name *other);

#ifdef __cplusplus
}
#endif

#endif

