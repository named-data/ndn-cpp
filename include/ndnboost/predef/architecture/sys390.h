/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_SYS390_H
#define NDNBOOST_PREDEF_ARCHITECTURE_SYS390_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_SYS390`]

[@http://en.wikipedia.org/wiki/System/390 System/390] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__s390__`] [__predef_detection__]]
    [[`__s390x__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_SYS390 NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__s390__) || defined(__s390x__)
#   undef NDNBOOST_ARCH_SYS390
#   define NDNBOOST_ARCH_SYS390 NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_SYS390
#   define NDNBOOST_ARCH_SYS390_AVAILABLE
#endif

#define NDNBOOST_ARCH_SYS390_NAME "System/390"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_SYS390,NDNBOOST_ARCH_SYS390_NAME)


#endif
