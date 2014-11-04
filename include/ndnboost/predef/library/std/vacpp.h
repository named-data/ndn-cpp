/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_VACPP_H
#define NDNBOOST_PREDEF_LIBRARY_STD_VACPP_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_IBM`]

[@http://www.ibm.com/software/awdtools/xlcpp/ IBM VACPP Standard C++] library.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__IBMCPP__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_LIB_STD_IBM NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IBMCPP__)
#   undef NDNBOOST_LIB_STD_IBM
#   define NDNBOOST_LIB_STD_IBM NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_LIB_STD_IBM
#   define NDNBOOST_LIB_STD_IBM_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_IBM_NAME "IBM VACPP"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_IBM,NDNBOOST_LIB_STD_IBM_NAME)


#endif
