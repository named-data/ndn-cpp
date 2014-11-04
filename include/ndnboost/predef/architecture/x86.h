/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_X86_H
#define NDNBOOST_PREDEF_ARCHITECTURE_X86_H

#include <ndnboost/predef/architecture/x86/32.h>
#include <ndnboost/predef/architecture/x86/64.h>

/*`
[heading `NDNBOOST_ARCH_X86`]

[@http://en.wikipedia.org/wiki/X86 Intel x86] architecture. This is
a category to indicate that either `NDNBOOST_ARCH_X86_32` or
`NDNBOOST_ARCH_X86_64` is detected.
 */

#define NDNBOOST_ARCH_X86 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if NDNBOOST_ARCH_X86_32 || NDNBOOST_ARCH_X86_64
#   undef NDNBOOST_ARCH_X86
#   define NDNBOOST_ARCH_X86 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_X86
#   define NDNBOOST_ARCH_X86_AVAILABLE
#endif

#define NDNBOOST_ARCH_X86_NAME "Intel x86"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_X86,NDNBOOST_ARCH_X86_NAME)

#endif
