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

#ifndef NDN_PUBLISHERPUBLICKEYDIGEST_H
#define NDN_PUBLISHERPUBLICKEYDIGEST_H

#include <ndn-cpp/c/publisher-public-key-digest-types.h>
#include "util/blob.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize an ndn_PublisherPublicKeyDigest struct with 0 for none.
 */
static __inline void ndn_PublisherPublicKeyDigest_initialize(struct ndn_PublisherPublicKeyDigest *self)
{
  ndn_Blob_initialize(&self->publisherPublicKeyDigest, 0, 0);
}

#ifdef __cplusplus
}
#endif

#endif
