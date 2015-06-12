//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_CONTAINER_DETAIL_WORKAROUND_HPP
#define NDNBOOST_CONTAINER_DETAIL_WORKAROUND_HPP

#include <ndnboost/container/detail/config_begin.hpp>

#if    !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)\
    && !defined(NDNBOOST_INTERPROCESS_DISABLE_VARIADIC_TMPL)
   #define NDNBOOST_CONTAINER_PERFECT_FORWARDING
#endif

#if defined(NDNBOOST_NO_CXX11_NOEXCEPT)
   #if defined(NDNBOOST_MSVC)
      #define NDNBOOST_CONTAINER_NOEXCEPT throw()
   #else
      #define NDNBOOST_CONTAINER_NOEXCEPT
   #endif
   #define NDNBOOST_CONTAINER_NOEXCEPT_IF(x)
#else
   #define NDNBOOST_CONTAINER_NOEXCEPT    noexcept
   #define NDNBOOST_CONTAINER_NOEXCEPT_IF(x) noexcept(x)
#endif

#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && defined(__GXX_EXPERIMENTAL_CXX0X__)\
    && (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__ < 40700)
   #define NDNBOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST
#endif

#if !defined(NDNBOOST_FALLTHOUGH)
   #define NDNBOOST_CONTAINER_FALLTHOUGH
#else
   #define NDNBOOST_CONTAINER_FALLTHOUGH NDNBOOST_FALLTHOUGH;
#endif

//Macros for documentation purposes. For code, expands to the argument
#define NDNBOOST_CONTAINER_IMPDEF(TYPE) TYPE
#define NDNBOOST_CONTAINER_SEEDOC(TYPE) TYPE

//Macros for memset optimization. In most platforms
//memsetting pointers and floatings is safe and faster.
//
//If your platform does not offer these guarantees
//define these to value zero.
#ifndef NDNBOOST_CONTAINER_MEMZEROED_FLOATING_POINT_IS_NOT_ZERO
#define NDNBOOST_CONTAINER_MEMZEROED_FLOATING_POINT_IS_ZERO 1
#endif

#ifndef NDNBOOST_CONTAINER_MEMZEROED_POINTER_IS_NOT_NULL
#define NDNBOOST_CONTAINER_MEMZEROED_POINTER_IS_NULL
#endif

#include <ndnboost/container/detail/config_end.hpp>

#endif   //#ifndef NDNBOOST_CONTAINER_DETAIL_WORKAROUND_HPP
