/*
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_LANGUAGE_OBJC_H
#define NDNBOOST_PREDEF_LANGUAGE_OBJC_H

#include <ndnboost/predef/version_number.h>
#include <ndnboost/predef/make.h>

/*`
[heading `NDNBOOST_LANG_OBJC`]

[@http://en.wikipedia.org/wiki/Objective-C Objective-C] language.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__OBJC__`] [__predef_detection__]]
    ]
 */

#define NDNBOOST_LANG_OBJC NDNBOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__OBJC__)
#   undef NDNBOOST_LANG_OBJC
#   define NDNBOOST_LANG_OBJC NDNBOOST_VERSION_NUMBER_AVAILABLE
#endif

#if NDNBOOST_LANG_OBJC
#   define NDNBOOST_LANG_OBJC_AVAILABLE
#endif

#define NDNBOOST_LANG_OBJC_NAME "Objective-C"

#include <ndnboost/predef/detail/test.h>
NDNBOOST_PREDEF_DECLARE_TEST(NDNBOOST_LANG_OBJC,NDNBOOST_LANG_OBJC_NAME)


#endif
