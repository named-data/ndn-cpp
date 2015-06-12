//  ndnboost/chrono/config.hpp  -------------------------------------------------//

//  Copyright Beman Dawes 2003, 2006, 2008
//  Copyright 2009-2011 Vicente J. Botet Escriba
//  Copyright (c) Microsoft Corporation 2014

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/chrono for documentation.

#ifndef NDNBOOST_CHRONO_CONFIG_HPP
#define NDNBOOST_CHRONO_CONFIG_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/predef.h>

#if !defined NDNBOOST_CHRONO_VERSION
#define NDNBOOST_CHRONO_VERSION 1
#else
#if NDNBOOST_CHRONO_VERSION!=1  && NDNBOOST_CHRONO_VERSION!=2
#error "NDNBOOST_CHRONO_VERSION must be 1 or 2"
#endif
#endif

#if defined(NDNBOOST_CHRONO_SOURCE) && !defined(NDNBOOST_USE_WINDOWS_H)
#define NDNBOOST_USE_WINDOWS_H
#endif

#if ! defined NDNBOOST_CHRONO_PROVIDES_DATE_IO_FOR_SYSTEM_CLOCK_TIME_POINT \
    && ! defined NDNBOOST_CHRONO_DONT_PROVIDE_DATE_IO_FOR_SYSTEM_CLOCK_TIME_POINT

# define NDNBOOST_CHRONO_PROVIDES_DATE_IO_FOR_SYSTEM_CLOCK_TIME_POINT

#endif

//  NDNBOOST_CHRONO_POSIX_API, NDNBOOST_CHRONO_MAC_API, or NDNBOOST_CHRONO_WINDOWS_API
//  can be defined by the user to specify which API should be used

#if defined(NDNBOOST_CHRONO_WINDOWS_API)
# warning Boost.Chrono will use the Windows API
#elif defined(NDNBOOST_CHRONO_MAC_API)
# warning Boost.Chrono will use the Mac API
#elif defined(NDNBOOST_CHRONO_POSIX_API)
# warning Boost.Chrono will use the POSIX API
#endif

# if defined( NDNBOOST_CHRONO_WINDOWS_API ) && defined( NDNBOOST_CHRONO_POSIX_API )
#   error both NDNBOOST_CHRONO_WINDOWS_API and NDNBOOST_CHRONO_POSIX_API are defined
# elif defined( NDNBOOST_CHRONO_WINDOWS_API ) && defined( NDNBOOST_CHRONO_MAC_API )
#   error both NDNBOOST_CHRONO_WINDOWS_API and NDNBOOST_CHRONO_MAC_API are defined
# elif defined( NDNBOOST_CHRONO_MAC_API ) && defined( NDNBOOST_CHRONO_POSIX_API )
#   error both NDNBOOST_CHRONO_MAC_API and NDNBOOST_CHRONO_POSIX_API are defined
# elif !defined( NDNBOOST_CHRONO_WINDOWS_API ) && !defined( NDNBOOST_CHRONO_MAC_API ) && !defined( NDNBOOST_CHRONO_POSIX_API )
#   if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32))
#     define NDNBOOST_CHRONO_WINDOWS_API
#   elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#     define NDNBOOST_CHRONO_MAC_API
#   else
#     define NDNBOOST_CHRONO_POSIX_API
#   endif
# endif

# if defined( NDNBOOST_CHRONO_WINDOWS_API )
#   ifndef UNDER_CE
#     define NDNBOOST_CHRONO_HAS_PROCESS_CLOCKS
#   endif
#   define NDNBOOST_CHRONO_HAS_CLOCK_STEADY
#   if NDNBOOST_PLAT_WINDOWS_DESKTOP
#     define NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#   endif
#   define NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY true
# endif

# if defined( NDNBOOST_CHRONO_MAC_API )
#   define NDNBOOST_CHRONO_HAS_PROCESS_CLOCKS
#   define NDNBOOST_CHRONO_HAS_CLOCK_STEADY
#   define NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#   define NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY true
# endif

# if defined( NDNBOOST_CHRONO_POSIX_API )
#   define NDNBOOST_CHRONO_HAS_PROCESS_CLOCKS
#   include <time.h>  //to check for CLOCK_REALTIME and CLOCK_MONOTONIC and _POSIX_THREAD_CPUTIME
#   if defined(CLOCK_MONOTONIC)
#      define NDNBOOST_CHRONO_HAS_CLOCK_STEADY
#   endif
#   if defined(_POSIX_THREAD_CPUTIME) && !defined(NDNBOOST_DISABLE_THREADS)
#     define NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#     define NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY true
#   endif
#   if defined(CLOCK_THREAD_CPUTIME_ID) && !defined(NDNBOOST_DISABLE_THREADS)
#     define NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#     define NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY true
#   endif
#   if defined(sun) || defined(__sun)
#     undef NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#     undef NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY
#   endif
#   if (defined(__HP_aCC) || defined(__GNUC__)) && defined(__hpux)
#     undef NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#     undef NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY
#   endif
#   if defined(__VXWORKS__)
#     undef NDNBOOST_CHRONO_HAS_PROCESS_CLOCKS
#   endif
# endif

#if defined(NDNBOOST_CHRONO_THREAD_DISABLED) && defined(NDNBOOST_CHRONO_HAS_THREAD_CLOCK)
#undef NDNBOOST_CHRONO_HAS_THREAD_CLOCK
#undef NDNBOOST_CHRONO_THREAD_CLOCK_IS_STEADY
#endif

// unicode support  ------------------------------//

#if defined(NDNBOOST_NO_CXX11_UNICODE_LITERALS) || defined(NDNBOOST_NO_CXX11_CHAR16_T) || defined(NDNBOOST_NO_CXX11_CHAR32_T)
//~ #define NDNBOOST_CHRONO_HAS_UNICODE_SUPPORT
#else
#define NDNBOOST_CHRONO_HAS_UNICODE_SUPPORT 1
#endif

#ifndef NDNBOOST_CHRONO_LIB_CONSTEXPR
#if defined( NDNBOOST_NO_CXX11_NUMERIC_LIMITS )
#define NDNBOOST_CHRONO_LIB_CONSTEXPR
#elif defined(_LIBCPP_VERSION) &&  !defined(_LIBCPP_CONSTEXPR)
  #define NDNBOOST_CHRONO_LIB_CONSTEXPR
#else
  #define NDNBOOST_CHRONO_LIB_CONSTEXPR NDNBOOST_CONSTEXPR
#endif
#endif

#if defined( NDNBOOST_NO_CXX11_NUMERIC_LIMITS )
#  define NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW throw()
#else
#ifdef NDNBOOST_NO_CXX11_NOEXCEPT
#  define NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW throw()
#else
#  define NDNBOOST_CHRONO_LIB_NOEXCEPT_OR_THROW noexcept
#endif
#endif

#if defined NDNBOOST_CHRONO_PROVIDE_HYBRID_ERROR_HANDLING \
 && defined NDNBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
#error "NDNBOOST_CHRONO_PROVIDE_HYBRID_ERROR_HANDLING && NDNBOOST_CHRONO_PROVIDE_HYBRID_ERROR_HANDLING defined"
#endif

#if defined NDNBOOST_CHRONO_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0 \
 && defined NDNBOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0
#error "NDNBOOST_CHRONO_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0 && NDNBOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0 defined"
#endif

#if ! defined NDNBOOST_CHRONO_PROVIDE_HYBRID_ERROR_HANDLING \
 && ! defined NDNBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
#define NDNBOOST_CHRONO_PROVIDE_HYBRID_ERROR_HANDLING
#endif

#if (NDNBOOST_CHRONO_VERSION == 2)
#if ! defined NDNBOOST_CHRONO_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0 \
 && ! defined NDNBOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0
#define NDNBOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0
#endif
#endif

#ifdef NDNBOOST_CHRONO_HEADER_ONLY
#define NDNBOOST_CHRONO_INLINE inline
#define NDNBOOST_CHRONO_STATIC inline
#define NDNBOOST_CHRONO_DECL

#else
#define NDNBOOST_CHRONO_INLINE
#define NDNBOOST_CHRONO_STATIC static

//  enable dynamic linking on Windows  ---------------------------------------//

// we need to import/export our code only if the user has specifically
// asked for it by defining either NDNBOOST_ALL_DYN_LINK if they want all boost
// libraries to be dynamically linked, or NDNBOOST_CHRONO_DYN_LINK
// if they want just this one to be dynamically liked:
#if defined(NDNBOOST_ALL_DYN_LINK) || defined(NDNBOOST_CHRONO_DYN_LINK)
// export if this is our own source, otherwise import:
#ifdef NDNBOOST_CHRONO_SOURCE
# define NDNBOOST_CHRONO_DECL NDNBOOST_SYMBOL_EXPORT
#else
# define NDNBOOST_CHRONO_DECL NDNBOOST_SYMBOL_IMPORT
#endif  // NDNBOOST_CHRONO_SOURCE
#endif  // DYN_LINK
//
// if NDNBOOST_CHRONO_DECL isn't defined yet define it now:
#ifndef NDNBOOST_CHRONO_DECL
#define NDNBOOST_CHRONO_DECL
#endif



//  enable automatic library variant selection  ------------------------------//

#if !defined(NDNBOOST_CHRONO_SOURCE) && !defined(NDNBOOST_ALL_NO_LIB) && !defined(NDNBOOST_CHRONO_NO_LIB)
//
// Set the name of our library; this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#define NDNBOOST_LIB_NAME ndnboost_chrono
//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
#if defined(NDNBOOST_ALL_DYN_LINK) || defined(NDNBOOST_CHRONO_DYN_LINK)
#  define NDNBOOST_DYN_LINK
#endif
//
// And include the header that does the work:
//
#include <ndnboost/config/auto_link.hpp>
#endif  // auto-linking disabled
#endif // NDNBOOST_CHRONO_HEADER_ONLY
#endif // NDNBOOST_CHRONO_CONFIG_HPP

