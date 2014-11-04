/*
Copyright (c) Microsoft Corporation 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_PLAT_WINDOWS_RUNTIME_H
#define NDNBOOST_PREDEF_PLAT_WINDOWS_RUNTIME_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>
#include <ndnboost/predef/os/windows.h>

/*`
[heading `NDNBOOST_PLAT_WINDOWS_RUNTIME`]

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`WINAPI_FAMILY == WINAPI_FAMILY_APP`] [__predef_detection__]]
    [[`WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_PLAT_WINDOWS_RUNTIME NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if NDNBOOST_OS_WINDOWS && defined(WINAPI_FAMILY) && \
    ( WINAPI_FAMILY == WINAPI_FAMILY_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP )
#   undef NDNBOOST_PLAT_WINDOWS_RUNTIME
#   define NDNBOOST_PLAT_WINDOWS_RUNTIME NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif
 
#if NDNBOOST_PLAT_WINDOWS_RUNTIME
#   define NDNBOOST_PLAT_WINDOWS_RUNTIME_AVALIABLE
#   include <ndnboost/predef/detail/platform_detected.h>
#endif

#define NDNBOOST_PLAT_WINDOWS_RUNTIME_NAME "Windows Runtime"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_PLAT_WINDOWS_RUNTIME,NDNBOOST_PLAT_WINDOWS_RUNTIME_NAME)

#endif
