/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_EKOPATH_H
#define NDNBOOST_PREDEF_COMPILER_EKOPATH_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_PATH`]

[@http://en.wikipedia.org/wiki/PathScale EKOpath] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__PATHCC__`] [__predef_detection__]]

    [[`__PATHCC__`, `__PATHCC_MINOR__`, `__PATHCC_PATCHLEVEL__`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_PATH NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__PATHCC__)
#   define NDNBOOST_COMP_PATH_DETECTION \
        NDNBOOST_VERSION_NUMBER(__PATHCC__,__PATHCC_MINOR__,__PATHCC_PATCHLEVEL__)
#endif

#ifdef NDNBOOST_COMP_PATH_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_PATH_EMULATED NDNBOOST_COMP_PATH_DETECTION
#   else
#       undef NDNBOOST_COMP_PATH
#       define NDNBOOST_COMP_PATH NDNBOOST_COMP_PATH_DETECTION
#   endif
#   define NDNBOOST_COMP_PATH_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_PATH_NAME "EKOpath"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PATH,NDNBOOST_COMP_PATH_NAME)

#ifdef NDNBOOST_COMP_PATH_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_PATH_EMULATED,NDNBOOST_COMP_PATH_NAME)
#endif


#endif
