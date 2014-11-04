/*
Copyright Rene Rivera 2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_BLACKFIN_H
#define NDNBOOST_PREDEF_ARCHITECTURE_BLACKFIN_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_BLACKFIN`]

Blackfin Processors from Analog Devices.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__bfin__`] [__predef_detection__]]
    [[`__BFIN__`] [__predef_detection__]]
    [[`bfin`] [__predef_detection__]]
    [[`BFIN`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_ARCH_BLACKFIN NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__bfin__) || defined(__BFIN__) || \
    defined(bfin) || defined(BFIN)
#   undef NDNBOOST_ARCH_BLACKFIN
#   define NDNBOOST_ARCH_BLACKFIN NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_ARCH_BLACKFIN
#   define NDNBOOST_ARCH_BLACKFIN_AVAILABLE
#endif

#define NDNBOOST_ARCH_BLACKFIN_NAME "Blackfin"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_BLACKFIN,NDNBOOST_ARCH_BLACKFIN_NAME)


#endif
