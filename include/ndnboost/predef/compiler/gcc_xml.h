/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_GCC_XML_H
#define NDNBOOST_PREDEF_COMPILER_GCC_XML_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_GCCXML`]

[@http://www.gccxml.org/ GCC XML] compiler.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__GCCXML__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_COMP_GCCXML NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GCCXML__)
#   define NDNBOOST_COMP_GCCXML_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#ifdef NDNBOOST_COMP_GCCXML_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_GCCXML_EMULATED NDNBOOST_COMP_GCCXML_DETECTION
#   else
#       undef NDNBOOST_COMP_GCCXML
#       define NDNBOOST_COMP_GCCXML NDNBOOST_COMP_GCCXML_DETECTION
#   endif
#   define NDNBOOST_COMP_GCCXML_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_GCCXML_NAME "GCC XML"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GCCXML,NDNBOOST_COMP_GCCXML_NAME)

#ifdef NDNBOOST_COMP_GCCXML_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_GCCXML_EMULATED,NDNBOOST_COMP_GCCXML_NAME)
#endif

#endif
