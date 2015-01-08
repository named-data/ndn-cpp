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

#ifdef __cplusplus
}
#endif

#endif
