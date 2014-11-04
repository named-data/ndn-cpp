//  (C) Copyright Christopher Jefferson 2011.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  config for libc++
//  Might need more in here later.

#if !defined(_LIBCPP_VERSION)
#  include <ciso646>
#  if !defined(_LIBCPP_VERSION)
#      error "This is not libc++!"
#  endif
#endif

#define NDNBOOST_STDLIB "libc++ version " NDNBOOST_STRINGIZE(_LIBCPP_VERSION)

#define NDNBOOST_HAS_THREADS

#ifdef _LIBCPP_HAS_NO_VARIADICS
#    define NDNBOOST_NO_CXX11_HDR_TUPLE
#endif

// NDNBOOST_NO_CXX11_ALLOCATOR should imply no support for the C++11
// allocator model. The C++11 allocator model requires a conforming
// std::allocator_traits which is only possible with C++11 template
// aliases since members rebind_alloc and rebind_traits require it.
#if defined(_LIBCPP_HAS_NO_TEMPLATE_ALIASES)
#    define NDNBOOST_NO_CXX11_ALLOCATOR
#endif

//
// These appear to be unusable/incomplete so far:
//
#  define NDNBOOST_NO_CXX11_HDR_CHRONO
#  define NDNBOOST_NO_CXX11_HDR_FUTURE
#  define NDNBOOST_NO_CXX11_HDR_TYPE_TRAITS
#  define NDNBOOST_NO_CXX11_ATOMIC_SMART_PTR
#  define NDNBOOST_NO_CXX11_HDR_ATOMIC

// libc++ uses a non-standard messages_base
#define NDNBOOST_NO_STD_MESSAGES

//  --- end ---
