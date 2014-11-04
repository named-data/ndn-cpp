/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_MODENA_H
#define NDNBOOST_PREDEF_LIBRARY_STD_MODENA_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_MSIPL`]

[@http://modena.us/ Modena Software Lib++] Standard C++ Library.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`MSIPL_COMPILE_H`] [__predef_detection__]]
    [[`__MSIPL_COMPILE_H`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_LIB_STD_MSIPL NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(MSIPL_COMPILE_H) || defined(__MSIPL_COMPILE_H)
#   undef NDNBOOST_LIB_STD_MSIPL
#   define NDNBOOST_LIB_STD_MSIPL NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_LIB_STD_MSIPL
#   define NDNBOOST_LIB_STD_MSIPL_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_MSIPL_NAME "Modena Software Lib++"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_MSIPL,NDNBOOST_LIB_STD_MSIPL_NAME)


#endif
