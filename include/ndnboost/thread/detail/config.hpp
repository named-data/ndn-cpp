// Copyright (C) 2001-2003
// William E. Kempf
// Copyright (C) 2011-2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_THREAD_CONFIG_WEK01032003_HPP
#define NDNBOOST_THREAD_CONFIG_WEK01032003_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/thread/detail/platform.hpp>

//#define NDNBOOST_THREAD_DONT_PROVIDE_INTERRUPTIONS
// ATTRIBUTE_MAY_ALIAS

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)

  // GCC since 3.3 has may_alias attribute that helps to alleviate optimizer issues with
  // regard to violation of the strict aliasing rules.

  #define NDNBOOST_THREAD_DETAIL_USE_ATTRIBUTE_MAY_ALIAS
  #define NDNBOOST_THREAD_ATTRIBUTE_MAY_ALIAS __attribute__((__may_alias__))
#else
  #define NDNBOOST_THREAD_ATTRIBUTE_MAY_ALIAS
#endif


#if defined NDNBOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED
#define NDNBOOST_THREAD_ASSERT_PRECONDITION(EXPR, EX) \
        if (EXPR) {} else ndnboost::throw_exception(EX)
#define NDNBOOST_THREAD_VERIFY_PRECONDITION(EXPR, EX) \
        if (EXPR) {} else ndnboost::throw_exception(EX)
#define NDNBOOST_THREAD_THROW_ELSE_RETURN(EX, RET) \
        ndnboost::throw_exception(EX)
#else
#define NDNBOOST_THREAD_ASSERT_PRECONDITION(EXPR, EX)
#define NDNBOOST_THREAD_VERIFY_PRECONDITION(EXPR, EX) \
        (void)(EXPR)
#define NDNBOOST_THREAD_THROW_ELSE_RETURN(EX, RET) \
        return (RET)
#endif

// This compiler doesn't support Boost.Chrono
#if defined __IBMCPP__ && (__IBMCPP__ < 1100) \
  && ! defined NDNBOOST_THREAD_DONT_USE_CHRONO
#define NDNBOOST_THREAD_DONT_USE_CHRONO
#if ! defined NDNBOOST_THREAD_USES_DATETIME
#define NDNBOOST_THREAD_USES_DATETIME
#endif
#endif

// This compiler doesn't support Boost.Move
#if NDNBOOST_WORKAROUND(__SUNPRO_CC, < 0x5100) \
  && ! defined NDNBOOST_THREAD_DONT_USE_MOVE
#define NDNBOOST_THREAD_DONT_USE_MOVE
#endif

// This compiler doesn't support Boost.Container Allocators files
#if defined __SUNPRO_CC \
  && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#define NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#endif

#if defined _WIN32_WCE && _WIN32_WCE==0x501 \
  && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#define NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#endif


#if defined NDNBOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX || defined NDNBOOST_NO_CXX11_HDR_INITIALIZER_LIST
#define NDNBOOST_THREAD_NO_CXX11_HDR_INITIALIZER_LIST
#define NDNBOOST_THREAD_NO_MAKE_LOCK_GUARD
#define NDNBOOST_THREAD_NO_MAKE_STRICT_LOCK
#define NDNBOOST_THREAD_NO_MAKE_NESTED_STRICT_LOCK
#endif

#if defined(NDNBOOST_NO_CXX11_HDR_TUPLE) || defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
#define NDNBOOST_THREAD_NO_MAKE_UNIQUE_LOCKS
#define NDNBOOST_THREAD_NO_SYNCHRONIZE
#elif defined _MSC_VER && _MSC_VER <= 1600
// C++ features supported by VC++ 10 (aka 2010)
#define NDNBOOST_THREAD_NO_MAKE_UNIQUE_LOCKS
#define NDNBOOST_THREAD_NO_SYNCHRONIZE
#endif

/// BASIC_THREAD_ID
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_BASIC_THREAD_ID \
 && ! defined NDNBOOST_THREAD_PROVIDES_BASIC_THREAD_ID
#define NDNBOOST_THREAD_PROVIDES_BASIC_THREAD_ID
#endif

/// RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR
//#if defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES || defined NDNBOOST_MSVC
#define NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR
//#endif

// Default version
#if !defined NDNBOOST_THREAD_VERSION
#define NDNBOOST_THREAD_VERSION 2
#else
#if NDNBOOST_THREAD_VERSION!=2  && NDNBOOST_THREAD_VERSION!=3 && NDNBOOST_THREAD_VERSION!=4
#error "NDNBOOST_THREAD_VERSION must be 2, 3 or 4"
#endif
#endif

// CHRONO
// Uses Boost.Chrono by default if not stated the opposite defining NDNBOOST_THREAD_DONT_USE_CHRONO
#if ! defined NDNBOOST_THREAD_DONT_USE_CHRONO \
  && ! defined NDNBOOST_THREAD_USES_CHRONO
#define NDNBOOST_THREAD_USES_CHRONO
#endif

#if ! defined NDNBOOST_THREAD_DONT_USE_ATOMIC \
  && ! defined NDNBOOST_THREAD_USES_ATOMIC
#define NDNBOOST_THREAD_USES_ATOMIC
//#define NDNBOOST_THREAD_DONT_USE_ATOMIC
#endif

#if defined NDNBOOST_THREAD_USES_ATOMIC
// Andrey Semashev
#define NDNBOOST_THREAD_ONCE_ATOMIC
#else
//#elif ! defined NDNBOOST_NO_CXX11_THREAD_LOCAL && ! defined NDNBOOST_NO_THREAD_LOCAL && ! defined NDNBOOST_THREAD_NO_UINT32_PSEUDO_ATOMIC
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2444.html#Appendix
#define NDNBOOST_THREAD_ONCE_FAST_EPOCH
#endif
#if NDNBOOST_THREAD_VERSION==2

// PROVIDE_PROMISE_LAZY
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_PROMISE_LAZY \
  && ! defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
#define NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
#endif

// PROVIDE_THREAD_EQ
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_THREAD_EQ \
  && ! defined NDNBOOST_THREAD_PROVIDES_THREAD_EQ
#define NDNBOOST_THREAD_PROVIDES_THREAD_EQ
#endif

#endif

#if NDNBOOST_THREAD_VERSION>=3

// ONCE_CXX11
// fixme NDNBOOST_THREAD_PROVIDES_ONCE_CXX11 doesn't works when thread.cpp is compiled NDNBOOST_THREAD_VERSION 3
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_ONCE_CXX11 \
 && ! defined NDNBOOST_THREAD_PROVIDES_ONCE_CXX11
#define NDNBOOST_THREAD_DONT_PROVIDE_ONCE_CXX11
#endif

// THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE \
 && ! defined NDNBOOST_THREAD_PROVIDES_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#define NDNBOOST_THREAD_PROVIDES_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#endif

// THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE \
 && ! defined NDNBOOST_THREAD_PROVIDES_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#define NDNBOOST_THREAD_PROVIDES_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#endif

// PROVIDE_FUTURE
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE \
 && ! defined NDNBOOST_THREAD_PROVIDES_FUTURE
#define NDNBOOST_THREAD_PROVIDES_FUTURE
#endif

// FUTURE_CTOR_ALLOCATORS
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS \
 && ! defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#define NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#endif

// SHARED_MUTEX_UPWARDS_CONVERSIONS
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_SHARED_MUTEX_UPWARDS_CONVERSIONS \
 && ! defined NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#define NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#endif

// PROVIDE_EXPLICIT_LOCK_CONVERSION
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_EXPLICIT_LOCK_CONVERSION \
 && ! defined NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#define NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#endif

// GENERIC_SHARED_MUTEX_ON_WIN
#if ! defined NDNBOOST_THREAD_DONT_PROVIDE_GENERIC_SHARED_MUTEX_ON_WIN \
 && ! defined NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#define NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#endif

// USE_MOVE
#if ! defined NDNBOOST_THREAD_DONT_USE_MOVE \
 && ! defined NDNBOOST_THREAD_USES_MOVE
#define NDNBOOST_THREAD_USES_MOVE
#endif

#endif

// deprecated since version 4
#if NDNBOOST_THREAD_VERSION < 4

// NESTED_LOCKS
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#define NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
#endif

// CONDITION
#if ! defined NDNBOOST_THREAD_PROVIDES_CONDITION \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_CONDITION
#define NDNBOOST_THREAD_PROVIDES_CONDITION
#endif

// USE_DATETIME
#if ! defined NDNBOOST_THREAD_DONT_USE_DATETIME \
 && ! defined NDNBOOST_THREAD_USES_DATETIME
#define NDNBOOST_THREAD_USES_DATETIME
#endif
#endif

#if NDNBOOST_THREAD_VERSION>=4

// SIGNATURE_PACKAGED_TASK
#if ! defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_SIGNATURE_PACKAGED_TASK
#define NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#endif

// VARIADIC_THREAD
#if ! defined NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_VARIADIC_THREAD

#if ! defined(NDNBOOST_NO_SFINAE_EXPR) && \
    ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    ! defined(NDNBOOST_NO_CXX11_DECLTYPE) && \
    ! defined(NDNBOOST_NO_CXX11_DECLTYPE_N3276) && \
    ! defined(NDNBOOST_THREAD_NO_CXX11_DECLTYPE_N3276) && \
    ! defined(NDNBOOST_NO_CXX11_TRAILING_RESULT_TYPES) && \
    ! defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && \
    ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)

#define NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD
#endif
#endif

#if ! defined NDNBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_WHEN_ALL_WHEN_ANY

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)

#define NDNBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
#endif
#endif

//    ! defined(NDNBOOST_NO_SFINAE_EXPR) &&
//    ! defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) &&
//    ! defined(NDNBOOST_NO_CXX11_AUTO) &&
//    ! defined(NDNBOOST_NO_CXX11_DECLTYPE) &&
//    ! defined(NDNBOOST_NO_CXX11_DECLTYPE_N3276) &&


// MAKE_READY_AT_THREAD_EXIT
#if ! defined NDNBOOST_THREAD_PROVIDES_MAKE_READY_AT_THREAD_EXIT \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_MAKE_READY_AT_THREAD_EXIT

//#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
#define NDNBOOST_THREAD_PROVIDES_MAKE_READY_AT_THREAD_EXIT
//#endif
#endif

// FUTURE_CONTINUATION
#if ! defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_CONTINUATION
#define NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif

#if ! defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_UNWRAP
#define NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
#endif

// FUTURE_INVALID_AFTER_GET
#if ! defined NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_FUTURE_INVALID_AFTER_GET
#define NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
#endif

// NESTED_LOCKS
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#define NDNBOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#endif

// CONDITION
#if ! defined NDNBOOST_THREAD_PROVIDES_CONDITION \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_CONDITION
#define NDNBOOST_THREAD_DONT_PROVIDE_CONDITION
#endif

#endif // NDNBOOST_THREAD_VERSION>=4

// INTERRUPTIONS
#if ! defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS \
 && ! defined NDNBOOST_THREAD_DONT_PROVIDE_INTERRUPTIONS
#define NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
#endif

// CORRELATIONS

// EXPLICIT_LOCK_CONVERSION.
#if defined NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#define NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION explicit
#else
#define NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION
#endif

// NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN is defined if NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#if defined NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS \
&& ! defined NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#define NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#endif

// For C++11 call_once interface the compiler MUST support constexpr.
// Otherwise once_flag would be initialized during dynamic initialization stage, which is not thread-safe.
#if defined(NDNBOOST_THREAD_PROVIDES_ONCE_CXX11)
#if defined(NDNBOOST_NO_CXX11_CONSTEXPR)
#undef NDNBOOST_THREAD_PROVIDES_ONCE_CXX11
#endif
#endif

#if defined(NDNBOOST_THREAD_PLATFORM_WIN32) && defined NDNBOOST_THREAD_DONT_USE_DATETIME
#undef NDNBOOST_THREAD_DONT_USE_DATETIME
#define NDNBOOST_THREAD_USES_DATETIME
#endif

#if defined(NDNBOOST_THREAD_PLATFORM_WIN32) && defined NDNBOOST_THREAD_DONT_USE_CHRONO
#undef NDNBOOST_THREAD_DONT_USE_CHRONO
#define NDNBOOST_THREAD_USES_CHRONO
#endif

// NDNBOOST_THREAD_PROVIDES_DEPRECATED_FEATURES_SINCE_V3_0_0 defined by default up to Boost 1.55
// NDNBOOST_THREAD_DONT_PROVIDE_DEPRECATED_FEATURES_SINCE_V3_0_0 defined by default up to Boost 1.55
#if defined NDNBOOST_THREAD_PROVIDES_DEPRECATED_FEATURES_SINCE_V3_0_0

#if  ! defined NDNBOOST_THREAD_PROVIDES_THREAD_EQ
#define NDNBOOST_THREAD_PROVIDES_THREAD_EQ
#endif

#endif

#if NDNBOOST_WORKAROUND(__BORLANDC__, < 0x600)
#  pragma warn -8008 // Condition always true/false
#  pragma warn -8080 // Identifier declared but never used
#  pragma warn -8057 // Parameter never used
#  pragma warn -8066 // Unreachable code
#endif

#include <ndnboost/thread/detail/platform.hpp>

#if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#else
  #   if defined(NDNBOOST_HAS_PTHREAD_DELAY_NP) || defined(NDNBOOST_HAS_NANOSLEEP)
  #     define NDNBOOST_THREAD_SLEEP_FOR_IS_STEADY
  #   endif
#endif

// provided for backwards compatibility, since this
// macro was used for several releases by mistake.
#if defined(NDNBOOST_THREAD_DYN_DLL) && ! defined NDNBOOST_THREAD_DYN_LINK
# define NDNBOOST_THREAD_DYN_LINK
#endif

// compatibility with the rest of Boost's auto-linking code:
#if defined(NDNBOOST_THREAD_DYN_LINK) || defined(NDNBOOST_ALL_DYN_LINK)
# undef  NDNBOOST_THREAD_USE_LIB
# if !defined(NDNBOOST_THREAD_USE_DLL)
#  define NDNBOOST_THREAD_USE_DLL
# endif
#endif

#if defined(NDNBOOST_THREAD_BUILD_DLL)   //Build dll
#elif defined(NDNBOOST_THREAD_BUILD_LIB) //Build lib
#elif defined(NDNBOOST_THREAD_USE_DLL)   //Use dll
#elif defined(NDNBOOST_THREAD_USE_LIB)   //Use lib
#else //Use default
#   if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#       if defined(NDNBOOST_MSVC) || defined(NDNBOOST_INTEL_WIN) \
      || defined(__MINGW32__) || defined(MINGW32) || defined(NDNBOOST_MINGW32)
      //For compilers supporting auto-tss cleanup
            //with Boost.Threads lib, use Boost.Threads lib
#           define NDNBOOST_THREAD_USE_LIB
#       else
            //For compilers not yet supporting auto-tss cleanup
            //with Boost.Threads lib, use Boost.Threads dll
#           define NDNBOOST_THREAD_USE_DLL
#       endif
#   else
#       define NDNBOOST_THREAD_USE_LIB
#   endif
#endif

#if defined(NDNBOOST_HAS_DECLSPEC)
#   if defined(NDNBOOST_THREAD_BUILD_DLL) //Build dll
#       define NDNBOOST_THREAD_DECL NDNBOOST_SYMBOL_EXPORT
//#       define NDNBOOST_THREAD_DECL __declspec(dllexport)

#   elif defined(NDNBOOST_THREAD_USE_DLL) //Use dll
#       define NDNBOOST_THREAD_DECL NDNBOOST_SYMBOL_IMPORT
//#       define NDNBOOST_THREAD_DECL __declspec(dllimport)
#   else
#       define NDNBOOST_THREAD_DECL
#   endif
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  define NDNBOOST_THREAD_DECL NDNBOOST_SYMBOL_VISIBLE

#else
#   define NDNBOOST_THREAD_DECL
#endif // NDNBOOST_HAS_DECLSPEC

//
// Automatically link to the correct build variant where possible.
//
#if !defined(NDNBOOST_ALL_NO_LIB) && !defined(NDNBOOST_THREAD_NO_LIB) && !defined(NDNBOOST_THREAD_BUILD_DLL) && !defined(NDNBOOST_THREAD_BUILD_LIB)
//
// Tell the autolink to link dynamically, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#if defined(NDNBOOST_THREAD_USE_DLL)
#   define NDNBOOST_DYN_LINK
#endif
//
// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#if defined(NDNBOOST_THREAD_LIB_NAME)
#    define NDNBOOST_LIB_NAME NDNBOOST_THREAD_LIB_NAME
#else
#    define NDNBOOST_LIB_NAME ndnboost_thread
#endif
//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
// And include the header that does the work:
//
#include <ndnboost/config/auto_link.hpp>
#endif  // auto-linking disabled

#endif // NDNBOOST_THREAD_CONFIG_WEK1032003_HPP

// Change Log:
//   22 Jan 05 Roland Schwarz (speedsnail)
//      Usage of NDNBOOST_HAS_DECLSPEC macro.
//      Default again is static lib usage.
//      NDNBOOST_DYN_LINK only defined when autolink included.
