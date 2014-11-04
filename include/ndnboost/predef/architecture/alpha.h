/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_ALPHA_H
#define NDNBOOST_PREDEF_ARCHITECTURE_ALPHA_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_ALPHA`]

[@http://en.wikipedia.org/wiki/DEC_Alpha DEC Alpha] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]
    [[`__alpha__`] [__predef_detection__]]
    [[`__alpha`] [__predef_detection__]]
    [[`_M_ALPHA`] [__predef_detection__]]

    [[`__alpha_ev4__`] [4.0.0]]
    [[`__alpha_ev5__`] [5.0.0]]
    [[`__alpha_ev6__`] [6.0.0]]
    ]
 */

#define NDNBOOST_ARCH_ALPHA NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__alpha__) || defined(__alpha) || \
    defined(_M_ALPHA)
#   undef NDNBOOST_ARCH_ALPHA
#   if !defined(NDNBOOST_ARCH_ALPHA) && defined(__alpha_ev4__)
#       define NDNBOOST_ARCH_ALPHA NDNBOOST_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(NDNBOOST_ARCH_ALPHA) && defined(__alpha_ev5__)
#       define NDNBOOST_ARCH_ALPHA NDNBOOST_VERSION_NUMBER(5,0,0)
#   endif
#   if !defined(NDNBOOST_ARCH_ALPHA) && defined(__alpha_ev6__)
#       define NDNBOOST_ARCH_ALPHA NDNBOOST_VERSION_NUMBER(6,0,0)
#   endif
#   if !defined(NDNBOOST_ARCH_ALPHA)
#       define NDNBOOST_ARCH_ALPHA NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_ARCH_ALPHA
#   define NDNBOOST_ARCH_ALPHA_AVAILABLE
#endif

#define NDNBOOST_ARCH_ALPHA_NAME "DEC Alpha"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_ALPHA,NDNBOOST_ARCH_ALPHA_NAME)


#endif
