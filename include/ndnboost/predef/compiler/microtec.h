/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_MICROTEC_H
#define NDNBOOST_PREDEF_COMPILER_MICROTEC_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_MRI`]

[@http://www.mentor.com/microtec/ Microtec C/C++] compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`_MRI`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_COMP_MRI NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_MRI)
#   define NDNBOOST_COMP_MRI_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#ifdef NDNBOOST_COMP_MRI_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_MRI_EMULATED NDNBOOST_COMP_MRI_DETECTION
#   else
#       undef NDNBOOST_COMP_MRI
#       define NDNBOOST_COMP_MRI NDNBOOST_COMP_MRI_DETECTION
#   endif
#   define NDNBOOST_COMP_MRI_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_MRI_NAME "Microtec C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MRI,NDNBOOST_COMP_MRI_NAME)

#ifdef NDNBOOST_COMP_MRI_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_MRI_EMULATED,NDNBOOST_COMP_MRI_NAME)
#endif


#endif
