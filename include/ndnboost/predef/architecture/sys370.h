/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_SYS370_H
#define NDNBOOST_PREDEF_ARCHITECTURE_SYS370_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_SYS370`]

[@http://en.wikipedia.org/wiki/System/370 System/370] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__370__`] [__predef_detection__]]
    [[`__THW_370__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_SYS370 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__370__) || defined(__THW_370__)
#   undef NDNBOOST_ARCH_SYS370
#   define NDNBOOST_ARCH_SYS370 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_SYS370
#   define NDNBOOST_ARCH_SYS370_AVAILABLE
#endif

#define NDNBOOST_ARCH_SYS370_NAME "System/370"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_SYS370,NDNBOOST_ARCH_SYS370_NAME)


#endif
