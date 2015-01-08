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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_KeyLocatorType_KEYNAME = 1,
  ndn_KeyLocatorType_KEY_LOCATOR_DIGEST = 2,
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

#ifdef __cplusplus
}
#endif

#endif
