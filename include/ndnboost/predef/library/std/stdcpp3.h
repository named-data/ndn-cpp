/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_STDCPP3_H
#define NDNBOOST_PREDEF_LIBRARY_STD_STDCPP3_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_GNU`]

[@http://gcc.gnu.org/libstdc++/ GNU libstdc++] Standard C++ library.
Version number available as year (from 1970), month, and day.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__GLIBCXX__`] [__predef_detection__]]
    [[`__GLIBCPP__`] [__predef_detection__]]

    [[`__GLIBCXX__`] [V.R.P]]
    [[`__GLIBCPP__`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_STD_GNU NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GLIBCPP__) || defined(__GLIBCXX__)
#   undef NDNBOOST_LIB_STD_GNU
#   if defined(__GLIBCXX__)
#       define NDNBOOST_LIB_STD_GNU NDNBOOST_PREDEF_MAKE_YYYYMMDD(__GLIBCXX__)
#   else
#       define NDNBOOST_LIB_STD_GNU NDNBOOST_PREDEF_MAKE_YYYYMMDD(__GLIBCPP__)
#   endif
#endif

#if NDNBOOST_LIB_STD_GNU
#   define NDNBOOST_LIB_STD_GNU_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_GNU_NAME "GNU"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_GNU,NDNBOOST_LIB_STD_GNU_NAME)


#endif
