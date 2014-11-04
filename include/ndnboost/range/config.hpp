// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef NDNBOOST_RANGE_CONFIG_HPP
#define NDNBOOST_RANGE_CONFIG_HPP

#include <ndnboost/detail/workaround.hpp>

#if defined(_MSC_VER)
# pragma once
#endif

#include <ndnboost/config.hpp>

#ifdef NDNBOOST_RANGE_DEDUCED_TYPENAME
#error "macro already defined!"
#endif

#if NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x564))
# define NDNBOOST_RANGE_DEDUCED_TYPENAME typename
#else
#define NDNBOOST_RANGE_DEDUCED_TYPENAME NDNBOOST_DEDUCED_TYPENAME
#endif

#ifdef NDNBOOST_RANGE_NO_ARRAY_SUPPORT
#error "macro already defined!"
#endif

#if NDNBOOST_WORKAROUND( __MWERKS__, <= 0x3003 )
#define NDNBOOST_RANGE_NO_ARRAY_SUPPORT 1
#endif

#ifdef NDNBOOST_RANGE_NO_ARRAY_SUPPORT
#define NDNBOOST_RANGE_ARRAY_REF() (boost_range_array)
#define NDNBOOST_RANGE_NO_STATIC_ASSERT
#else
#define NDNBOOST_RANGE_ARRAY_REF() (&boost_range_array)
#endif

#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
#  define NDNBOOST_RANGE_UNUSED __attribute__((unused))
#else
#  define NDNBOOST_RANGE_UNUSED
#endif



#endif

