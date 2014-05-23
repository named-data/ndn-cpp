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

#ifndef NDN_COMMON_H
#define NDN_COMMON_H

#include "../ndn-cpp-config.h"
#include <stdint.h>
// TODO: Is stddef.h portable?
#include <stddef.h>

#if NDN_CPP_HAVE_ATTRIBUTE_DEPRECATED
  #define DEPRECATED_IN_NDN_CPP __attribute__((deprecated))
#else
  #define DEPRECATED_IN_NDN_CPP
#endif

#if !NDN_CPP_HAVE_ROUND
#define round(x) floor((x) + 0.5)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A time interval represented as the number of milliseconds.
 */
typedef double ndn_Milliseconds;
   
/**
 * The calendar time represented as the number of milliseconds since 1/1/1970.
 */
typedef double ndn_MillisecondsSince1970;

#ifdef __cplusplus
}
#endif

#endif
