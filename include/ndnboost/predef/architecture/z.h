/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_Z_H
#define NDNBOOST_PREDEF_ARCHITECTURE_Z_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_Z`]

[@http://en.wikipedia.org/wiki/Z/Architecture z/Architecture] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__SYSC_ZARCH__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_Z NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SYSC_ZARCH__)
#   undef NDNBOOST_ARCH_Z
#   define NDNBOOST_ARCH_Z NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_Z
#   define NDNBOOST_ARCH_Z_AVAILABLE
#endif

#define NDNBOOST_ARCH_Z_NAME "z/Architecture"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_Z,NDNBOOST_ARCH_Z_NAME)


#endif
