/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_LIBCOMO_H
#define NDNBOOST_PREDEF_LIBRARY_STD_LIBCOMO_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_COMO`]

[@http://www.comeaucomputing.com/libcomo/ Comeau Computing] Standard C++ Library.
Version number available as major.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__LIBCOMO__`] [__predef_detection__]]

    [[`__LIBCOMO_VERSION__`] [V.0.0]]
    ]
 */

#define NDNBOOST_LIB_STD_COMO NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__LIBCOMO__)
#   undef NDNBOOST_LIB_STD_COMO
#   define NDNBOOST_LIB_STD_COMO NDNBOOST_VERSION_NUMBER(__LIBCOMO_VERSION__,0,0)
#endif

#if NDNBOOST_LIB_STD_COMO
#   define NDNBOOST_LIB_STD_COMO_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_COMO_NAME "Comeau Computing"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_COMO,NDNBOOST_LIB_STD_COMO_NAME)


#endif
