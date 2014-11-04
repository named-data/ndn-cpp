/*
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_COMPILER_COMPAQ_H
#define NDNBOOST_PREDEF_COMPILER_COMPAQ_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_COMP_DEC`]

[@http://www.openvms.compaq.com/openvms/brochures/deccplus/ Compaq C/C++] compiler.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__DECCXX`] [__predef_detection__]]
    [[`__DECC`] [__predef_detection__]]

    [[`__DECCXX_VER`] [V.R.P]]
    [[`__DECC_VER`] [V.R.P]]
    ]
 */

#define NDNBOOST_COMP_DEC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DECC) || defined(__DECCXX)
#   if !defined(NDNBOOST_COMP_DEC_DETECTION) && defined(__DECCXX_VER)
#       define NDNBOOST_COMP_DEC_DETECTION NDNBOOST_PREDEF_MAKE_10_VVRR0PP00(__DECCXX_VER)
#   endif
#   if !defined(NDNBOOST_COMP_DEC_DETECTION) && defined(__DECC_VER)
#       define NDNBOOST_COMP_DEC_DETECTION NDNBOOST_PREDEF_MAKE_10_VVRR0PP00(__DECC_VER)
#   endif
#   if !defined(NDNBOOST_COMP_DEC_DETECTION)
#       define NDNBOOST_COM_DEC_DETECTION NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef NDNBOOST_COMP_DEC_DETECTION
#   if defined(NDNBOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define NDNBOOST_COMP_DEC_EMULATED NDNBOOST_COMP_DEC_DETECTION
#   else
#       undef NDNBOOST_COMP_DEC
#       define NDNBOOST_COMP_DEC NDNBOOST_COMP_DEC_DETECTION
#   endif
#   define NDNBOOST_COMP_DEC_AVAILABLE
#   include <ndnboost/predef/detail/comp_detected.h>
#endif

#define NDNBOOST_COMP_DEC_NAME "Compaq C/C++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DEC,NDNBOOST_COMP_DEC_NAME)

#ifdef NDNBOOST_COMP_DEC_EMULATED
#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_COMP_DEC_EMULATED,NDNBOOST_COMP_DEC_NAME)
#endif


#endif
