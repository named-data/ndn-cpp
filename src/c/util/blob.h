/**
 * Copyright (C) 2013-2014 Regents of the University of California.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU General Public License is in the file COPYING.
 */

#ifndef NDN_BLOB_H
#define NDN_BLOB_H

#include <ndn-cpp/c/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An ndn_Blob holds a pointer to a read-only pre-allocated buffer and its length.
 */
struct ndn_Blob {
  const uint8_t *value; /**< pointer to the pre-allocated buffer for the value. Must be treated as read only. */
  size_t length;        /**< the number of bytes in value. */
};

/**
 * Initialize the ndn_Blob struct with the given value.
 * @param self pointer to the ndn_Blob struct.
 * @param value The pre-allocated buffer for the value, or 0 for none.
 * @param length The number of bytes in value.
 */
static __inline void ndn_Blob_initialize(struct ndn_Blob *self, const uint8_t *value, size_t length) 
{
  self->value = value;
  self->length = length;
}

#ifdef __cplusplus
}
#endif

#endif
