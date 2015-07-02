//  win/chrono.cpp  --------------------------------------------------------------//

//  Copyright Beman Dawes 2008
//  Copyright 2009-2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//----------------------------------------------------------------------------//
//                                Windows                                     //
//----------------------------------------------------------------------------//
#ifndef NDNBOOST_CHRONO_DETAIL_INLINED_WIN_CHRONO_HPP
#define NDNBOOST_CHRONO_DETAIL_INLINED_WIN_CHRONO_HPP

#include <ndnboost/detail/winapi/time.hpp>
#include <ndnboost/detail/winapi/timers.hpp>
#include <ndnboost/detail/winapi/GetLastError.hpp>

namespace ndnboost
{
namespace chrono
{
namespace chrono_detail
{

  NDNBOOST_CHRONO_INLINE double get_nanosecs_per_tic() NDNBOOST_NOEXCEPT
  {
      ndnboost::detail::winapi::LARGE_INTEGER_ freq;
      if ( !ndnboost::detail::winapi::QueryPerformanceFrequency( &freq ) )
          return 0.0L;
      return double(1000000000.0L / freq.QuadPart);
  }

}

  steady_clock::time_point steady_clock::now() NDNBOOST_NOEXCEPT
  {
    double nanosecs_per_tic = chrono_detail::get_nanosecs_per_tic();

    ndnboost::detail::winapi::LARGE_INTEGER_ pcount;
    if ( nanosecs_per_tic <= 0.0L )
    {
      NDNBOOST_ASSERT(0 && "Boost::Chrono - get_nanosecs_per_tic Internal Error");
      return steady_clock::time_point();
    }
    unsigned times=0;
    while ( ! ndnboost::detail::winapi::QueryPerformanceCounter( &pcount ) )
    {
      if ( ++times > 3 )
      {
        NDNBOOST_ASSERT(0 && "Boost::Chrono - QueryPerformanceCounter Internal Error");
        return steady_clock::time_point();
      }
    }

    return steady_clock::time_point(steady_clock::duration(
      static_cast<steady_clock::rep>((nanosecs_per_tic) * pcount.QuadPart)));
  }


#if !defined NDNBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
  steady_clock::time_point steady_clock::now( system::error_code & ec )
  {
    double nanosecs_per_tic = chrono_detail::get_nanosecs_per_tic();

    ndnboost::detail::winapi::LARGE_INTEGER_ pcount;
    if ( (nanosecs_per_tic <= 0.0L)
            || (!ndnboost::detail::winapi::QueryPerformanceCounter( &pcount )) )
    {
        ndnboost::detail::winapi::DWORD_ cause =
            ((nanosecs_per_tic <= 0.0L)
                    ? ERROR_NOT_SUPPORTED
                    : ndnboost::detail::winapi::GetLastError());
        if (NDNBOOST_CHRONO_IS_THROWS(ec)) {
            ndnboost::throw_exception(
                    system::system_error(
                            cause,
                            NDNBOOST_CHRONO_SYSTEM_CATEGORY,
                            "chrono::steady_clock" ));
        }
        else
        {
            ec.assign( cause, NDNBOOST_CHRONO_SYSTEM_CATEGORY );
            return steady_clock::time_point(duration(0));
        }
    }

    if (!NDNBOOST_CHRONO_IS_THROWS(ec))
    {
        ec.clear();
    }
    return time_point(duration(
      static_cast<steady_clock::rep>(nanosecs_per_tic * pcount.QuadPart)));
  }
#endif

  NDNBOOST_CHRONO_INLINE
  system_clock::time_point system_clock::now() NDNBOOST_NOEXCEPT
  {
    ndnboost::detail::winapi::FILETIME_ ft;
    ndnboost::detail::winapi::GetSystemTimeAsFileTime( &ft );  // never fails
    return system_clock::time_point(
      system_clock::duration(
        ((static_cast<__int64>( ft.dwHighDateTime ) << 32) | ft.dwLowDateTime)
       - 116444736000000000LL
       //- (134775LL*864000000000LL)
      )
    );
  }

#if !defined NDNBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
  NDNBOOST_CHRONO_INLINE
  system_clock::time_point system_clock::now( system::error_code & ec )
  {
    ndnboost::detail::winapi::FILETIME_ ft;
    ndnboost::detail::winapi::GetSystemTimeAsFileTime( &ft );  // never fails
    if (!NDNBOOST_CHRONO_IS_THROWS(ec))
    {
        ec.clear();
    }
    return system_clock::time_point(
      system_clock::duration(
       ((static_cast<__int64>( ft.dwHighDateTime ) << 32) | ft.dwLowDateTime)
       - 116444736000000000LL
       //- (134775LL*864000000000LL)
       ));
  }
#endif

  NDNBOOST_CHRONO_INLINE
  std::time_t system_clock::to_time_t(const system_clock::time_point& t) NDNBOOST_NOEXCEPT
  {
      __int64 temp = t.time_since_epoch().count();
      temp /= 10000000;
      return static_cast<std::time_t>( temp );
  }

  NDNBOOST_CHRONO_INLINE
  system_clock::time_point system_clock::from_time_t(std::time_t t) NDNBOOST_NOEXCEPT
  {
      __int64 temp = t;
      temp *= 10000000;
      return time_point(duration(temp));
  }

}  // namespace chrono
}  // namespace ndnboost

#endif
