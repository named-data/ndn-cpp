/*
Copyright (c) Microsoft Corporation 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_PLAT_WINDOWS_STORE_H
#define NDNBOOST_PREDEF_PLAT_WINDOWS_STORE_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>
#include <ndnboost/predef/os/windows.h>

/*`
[heading `NDNBOOST_PLAT_WINDOWS_STORE`]

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`WINAPI_FAMILY == WINAPI_FAMILY_APP`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_PLAT_WINDOWS_STORE NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if NDNBOOST_OS_WINDOWS && defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_APP
#   undef NDNBOOST_PLAT_WINDOWS_STORE
#   define NDNBOOST_PLAT_WINDOWS_STORE NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif
 
#if NDNBOOST_PLAT_WINDOWS_STORE
#   define NDNBOOST_PLAT_WINDOWS_STORE_AVALIABLE
#   include <ndnboost/predef/detail/platform_detected.h>
#endif

#define NDNBOOST_PLAT_WINDOWS_STORE_NAME "Windows Store"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_PLAT_WINDOWS_STORE,NDNBOOST_PLAT_WINDOWS_STORE_NAME)

#endif
