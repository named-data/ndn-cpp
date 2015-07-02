//  config.hpp  ---------------------------------------------------------------//

//  Copyright 2012 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef NDNBOOST_RATIO_CONFIG_HPP
#define NDNBOOST_RATIO_CONFIG_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/cstdint.hpp>


#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 5) || !defined(__GXX_EXPERIMENTAL_CXX0X__)
#  if ! defined NDNBOOST_NO_CXX11_U16STRING
#    define NDNBOOST_NO_CXX11_U16STRING
#  endif
#  if ! defined NDNBOOST_NO_CXX11_U32STRING
#    define NDNBOOST_NO_CXX11_U32STRING
#  endif
#endif


#if !defined NDNBOOST_RATIO_VERSION
#define NDNBOOST_RATIO_VERSION 1
#else
#if NDNBOOST_RATIO_VERSION!=1  && NDNBOOST_RATIO_VERSION!=2
#error "NDNBOOST_RATIO_VERSION must be 1 or 2"
#endif
#endif

#if NDNBOOST_RATIO_VERSION==1
#if ! defined NDNBOOST_RATIO_DONT_PROVIDE_DEPRECATED_FEATURES_SINCE_V2_0_0
#define NDNBOOST_RATIO_PROVIDES_DEPRECATED_FEATURES_SINCE_V2_0_0
#endif
#endif

#if NDNBOOST_RATIO_VERSION==2
#if ! defined NDNBOOST_RATIO_PROVIDES_DEPRECATED_FEATURES_SINCE_V2_0_0
#define NDNBOOST_RATIO_DONT_PROVIDE_DEPRECATED_FEATURES_SINCE_V2_0_0
#endif
#endif

#ifdef INTMAX_C
#define NDNBOOST_RATIO_INTMAX_C(a) INTMAX_C(a)
#else
#define NDNBOOST_RATIO_INTMAX_C(a) a##LL
#endif

#ifdef UINTMAX_C
#define NDNBOOST_RATIO_UINTMAX_C(a) UINTMAX_C(a)
#else
#define NDNBOOST_RATIO_UINTMAX_C(a) a##ULL
#endif

#define NDNBOOST_RATIO_INTMAX_T_MAX (0x7FFFFFFFFFFFFFFELL)


#ifndef NDNBOOST_NO_CXX11_STATIC_ASSERT
#define NDNBOOST_RATIO_STATIC_ASSERT(CND, MSG, TYPES) static_assert(CND,MSG)
#elif defined(NDNBOOST_RATIO_USES_STATIC_ASSERT)
#include <ndnboost/static_assert.hpp>
#define NDNBOOST_RATIO_STATIC_ASSERT(CND, MSG, TYPES) NDNBOOST_STATIC_ASSERT(CND)
#elif defined(NDNBOOST_RATIO_USES_MPL_ASSERT)
#include <ndnboost/mpl/assert.hpp>
#include <ndnboost/mpl/bool.hpp>
#define NDNBOOST_RATIO_STATIC_ASSERT(CND, MSG, TYPES)                                 \
    NDNBOOST_MPL_ASSERT_MSG(ndnboost::mpl::bool_< (CND) >::type::value, MSG, TYPES)
#else
//~ #elif defined(NDNBOOST_RATIO_USES_ARRAY_ASSERT)
#define NDNBOOST_RATIO_CONCAT(A,B) A##B
#define NDNBOOST_RATIO_NAME(A,B) NDNBOOST_RATIO_CONCAT(A,B)
#define NDNBOOST_RATIO_STATIC_ASSERT(CND, MSG, TYPES) static char NDNBOOST_RATIO_NAME(__boost_ratio_test_,__LINE__)[(CND)?1:-1]
//~ #define NDNBOOST_RATIO_STATIC_ASSERT(CND, MSG, TYPES)
#endif

#if !defined(NDNBOOST_NO_CXX11_STATIC_ASSERT) || !defined(NDNBOOST_RATIO_USES_MPL_ASSERT)
#define NDNBOOST_RATIO_OVERFLOW_IN_ADD "overflow in ratio add"
#define NDNBOOST_RATIO_OVERFLOW_IN_SUB "overflow in ratio sub"
#define NDNBOOST_RATIO_OVERFLOW_IN_MUL "overflow in ratio mul"
#define NDNBOOST_RATIO_OVERFLOW_IN_DIV "overflow in ratio div"
#define NDNBOOST_RATIO_NUMERATOR_IS_OUT_OF_RANGE "ratio numerator is out of range"
#define NDNBOOST_RATIO_DIVIDE_BY_0 "ratio divide by 0"
#define NDNBOOST_RATIO_DENOMINATOR_IS_OUT_OF_RANGE "ratio denominator is out of range"
#endif


//#define NDNBOOST_RATIO_EXTENSIONS

#endif  // header
