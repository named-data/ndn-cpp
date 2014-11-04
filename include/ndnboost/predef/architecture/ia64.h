/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_IA64_H
#define NDNBOOST_PREDEF_ARCHITECTURE_IA64_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_IA64`]

[@http://en.wikipedia.org/wiki/Ia64 Intel Itanium 64] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__ia64__`] [__predef_detection__]]
    [[`_IA64`] [__predef_detection__]]
    [[`__IA64__`] [__predef_detection__]]
    [[`__ia64`] [__predef_detection__]]
    [[`_M_IA64`] [__predef_detection__]]
    [[`__itanium__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_IA64 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ia64__) || defined(_IA64) || \
    defined(__IA64__) || defined(__ia64) || \
    defined(_M_IA64) || defined(__itanium__)
#   undef NDNBOOST_ARCH_IA64
#   define NDNBOOST_ARCH_IA64 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_IA64
#   define NDNBOOST_ARCH_IA64_AVAILABLE
#endif

#define NDNBOOST_ARCH_IA64_NAME "Intel Itanium 64"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_IA64,NDNBOOST_ARCH_IA64_NAME)

#endif
