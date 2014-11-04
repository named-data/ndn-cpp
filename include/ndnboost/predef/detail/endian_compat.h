/*
Copyright Rene Rivera 2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef NDNBOOST_PREDEF_DETAIL_ENDIAN_COMPAT_H
#define NDNBOOST_PREDEF_DETAIL_ENDIAN_COMPAT_H

#include <ndnboost/predef/other/endian.h>

#if NDNBOOST_ENDIAN_BIG_BYTE
#   define NDNBOOST_BIG_ENDIAN
#   define NDNBOOST_BYTE_ORDER 4321
#endif
#if NDNBOOST_ENDIAN_LITTLE_BYTE
#   define NDNBOOST_LITTLE_ENDIAN
#   define NDNBOOST_BYTE_ORDER 1234
#endif
#if NDNBOOST_ENDIAN_LITTLE_WORD
#   define NDNBOOST_PDP_ENDIAN
#   define NDNBOOST_BYTE_ORDER 2134
#endif

#endif
