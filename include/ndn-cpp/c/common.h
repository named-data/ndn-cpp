/**
 * Copyright (C) 2013-2014 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * See COPYING for copyright and distribution information.
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
