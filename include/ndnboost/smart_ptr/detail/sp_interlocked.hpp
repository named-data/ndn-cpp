#ifndef NDNBOOST_SMART_PTR_DETAIL_SP_INTERLOCKED_HPP_INCLUDED
#define NDNBOOST_SMART_PTR_DETAIL_SP_INTERLOCKED_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  ndnboost/detail/sp_interlocked.hpp
//
//  Copyright 2005, 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <ndnboost/config.hpp>

// NDNBOOST_SP_HAS_INTRIN_H

// VC9 has intrin.h, but it collides with <utility>
#if defined( NDNBOOST_MSVC ) && NDNBOOST_MSVC >= 1600

# define NDNBOOST_SP_HAS_INTRIN_H

// Unlike __MINGW64__, __MINGW64_VERSION_MAJOR is defined by MinGW-w64 for both 32 and 64-bit targets.
#elif defined( __MINGW64_VERSION_MAJOR )

// MinGW-w64 provides intrin.h for both 32 and 64-bit targets.
# define NDNBOOST_SP_HAS_INTRIN_H

// Intel C++ on Windows on VC10+ stdlib
#elif defined( NDNBOOST_INTEL_WIN ) && defined( _CPPLIB_VER ) && _CPPLIB_VER >= 520

# define NDNBOOST_SP_HAS_INTRIN_H

#endif

#if defined( NDNBOOST_USE_WINDOWS_H )

# include <windows.h>

# define NDNBOOST_SP_INTERLOCKED_INCREMENT InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd

#elif defined( NDNBOOST_USE_INTRIN_H ) || defined( NDNBOOST_SP_HAS_INTRIN_H )

#include <intrin.h>

# define NDNBOOST_SP_INTERLOCKED_INCREMENT _InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT _InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE _InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

#elif defined( _WIN32_WCE )

#if _WIN32_WCE >= 0x600

extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );

# define NDNBOOST_SP_INTERLOCKED_INCREMENT _InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT _InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE _InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

#else

// under Windows CE we still have old-style Interlocked* functions

extern "C" long __cdecl InterlockedIncrement( long* );
extern "C" long __cdecl InterlockedDecrement( long* );
extern "C" long __cdecl InterlockedCompareExchange( long*, long, long );
extern "C" long __cdecl InterlockedExchange( long*, long );
extern "C" long __cdecl InterlockedExchangeAdd( long*, long );

# define NDNBOOST_SP_INTERLOCKED_INCREMENT InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd

#endif

#elif defined( NDNBOOST_MSVC ) || defined( NDNBOOST_INTEL_WIN )

#if defined( __CLRCALL_PURE_OR_CDECL )

extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedIncrement( long volatile * );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedDecrement( long volatile * );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedExchange( long volatile *, long );
extern "C" long __CLRCALL_PURE_OR_CDECL _InterlockedExchangeAdd( long volatile *, long );

#else

extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );

#endif

# define NDNBOOST_SP_INTERLOCKED_INCREMENT _InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT _InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE _InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

#elif defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

namespace ndnboost
{

namespace detail
{

extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement( long volatile * );
extern "C" __declspec(dllimport) long __stdcall InterlockedDecrement( long volatile * );
extern "C" __declspec(dllimport) long __stdcall InterlockedCompareExchange( long volatile *, long, long );
extern "C" __declspec(dllimport) long __stdcall InterlockedExchange( long volatile *, long );
extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd( long volatile *, long );

} // namespace detail

} // namespace ndnboost

# define NDNBOOST_SP_INTERLOCKED_INCREMENT ::ndnboost::detail::InterlockedIncrement
# define NDNBOOST_SP_INTERLOCKED_DECREMENT ::ndnboost::detail::InterlockedDecrement
# define NDNBOOST_SP_INTERLOCKED_COMPARE_EXCHANGE ::ndnboost::detail::InterlockedCompareExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE ::ndnboost::detail::InterlockedExchange
# define NDNBOOST_SP_INTERLOCKED_EXCHANGE_ADD ::ndnboost::detail::InterlockedExchangeAdd

#else

# error "Interlocked intrinsics not available"

#endif

#endif // #ifndef NDNBOOST_SMART_PTR_DETAIL_SP_INTERLOCKED_HPP_INCLUDED
