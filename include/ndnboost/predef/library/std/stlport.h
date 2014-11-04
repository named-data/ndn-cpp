/*
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LIBRARY_STD_STLPORT_H
#define NDNBOOST_PREDEF_LIBRARY_STD_STLPORT_H

#include <ndnboost/predef/library/std/_prefix.h>

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LIB_STD_STLPORT`]

[@http://sourceforge.net/projects/stlport/ STLport Standard C++] library.
Version number available as major, minor, and patch.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__SGI_STL_PORT`] [__predef_detection__]]
    [[`_STLPORT_VERSION`] [__predef_detection__]]

    [[`_STLPORT_MAJOR`, `_STLPORT_MINOR`, `_STLPORT_PATCHLEVEL`] [V.R.P]]
    [[`_STLPORT_VERSION`] [V.R.P]]
    [[`__SGI_STL_PORT`] [V.R.P]]
    ]
 */

#define NDNBOOST_LIB_STD_STLPORT NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)
#   undef NDNBOOST_LIB_STD_STLPORT
#   if !defined(NDNBOOST_LIB_STD_STLPORT) && defined(_STLPORT_MAJOR)
#       define NDNBOOST_LIB_STD_STLPORT \
            NDNBOOST_VERSION_NUMBER(_STLPORT_MAJOR,_STLPORT_MINOR,_STLPORT_PATCHLEVEL)
#   endif
#   if !defined(NDNBOOST_LIB_STD_STLPORT) && defined(_STLPORT_VERSION)
#       define NDNBOOST_LIB_STD_STLPORT NDNBOOST_PREDEF_MAKE_0X_VRP(_STLPORT_VERSION)
#   endif
#   if !defined(NDNBOOST_LIB_STD_STLPORT)
#       define NDNBOOST_LIB_STD_STLPORT NDNBOOST_PREDEF_MAKE_0X_VRP(__SGI_STL_PORT)
#   endif
#endif

#if NDNBOOST_LIB_STD_STLPORT
#   define NDNBOOST_LIB_STD_STLPORT_AVAILABLE
#endif

#define NDNBOOST_LIB_STD_STLPORT_NAME "STLport"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LIB_STD_STLPORT,NDNBOOST_LIB_STD_STLPORT_NAME)


#endif
