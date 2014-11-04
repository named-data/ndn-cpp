/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_X86_64_H
#define NDNBOOST_PREDEF_ARCHITECTURE_X86_64_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_X86_64`]

[@http://en.wikipedia.org/wiki/Ia64 Intel IA-64] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__x86_64`] [__predef_detection__]]
    [[`__x86_64__`] [__predef_detection__]]
    [[`__amd64__`] [__predef_detection__]]
    [[`__amd64`] [__predef_detection__]]
    [[`_M_X64`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_X86_64 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__x86_64) || defined(__x86_64__) || \
    defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64)
#   undef NDNBOOST_ARCH_X86_64
#   define NDNBOOST_ARCH_X86_64 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_X86_64
#   define NDNBOOST_ARCH_X86_64_AVAILABLE
#endif

#define NDNBOOST_ARCH_X86_64_NAME "Intel x86-64"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_X86_64,NDNBOOST_ARCH_X86_64_NAME)

#include <ndnboost/predef/architecture/x86.h>

#endif
