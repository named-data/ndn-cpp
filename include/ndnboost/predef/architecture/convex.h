/*
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_ARCHITECTURE_CONVEX_H
#define NDNBOOST_PREDEF_ARCHITECTURE_CONVEX_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_ARCH_CONVEX`]

[@http://en.wikipedia.org/wiki/Convex_Computer Convex Computer] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__convex__`] [__predef_detection__]]

    [[`__convex_c1__`] [1.0.0]]
    [[`__convex_c2__`] [2.0.0]]
    [[`__convex_c32__`] [3.2.0]]
    [[`__convex_c34__`] [3.4.0]]
    [[`__convex_c38__`] [3.8.0]]
    ]
 */

#define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__convex__)
#   undef NDNBOOST_ARCH_CONVEX
#   if !defined(NDNBOOST_ARCH_CONVEX) && defined(__convex_c1__)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(NDNBOOST_ARCH_CONVEX) && defined(__convex_c2__)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(NDNBOOST_ARCH_CONVEX) && defined(__convex_c32__)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(NDNBOOST_ARCH_CONVEX) && defined(__convex_c34__)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER(3,4,0)
#   endif
#   if !defined(NDNBOOST_ARCH_CONVEX) && defined(__convex_c38__)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER(3,8,0)
#   endif
#   if !defined(NDNBOOST_ARCH_CONVEX)
#       define NDNBOOST_ARCH_CONVEX NDNBOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if NDNBOOST_ARCH_CONVEX
#   define NDNBOOST_ARCH_CONVEX_AVAILABLE
#endif

#define NDNBOOST_ARCH_CONVEX_NAME "Convex Computer"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_ARCH_CONVEX,NDNBOOST_ARCH_CONVEX_NAME)



#endif
