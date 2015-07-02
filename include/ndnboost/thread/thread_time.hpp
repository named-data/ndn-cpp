#ifndef NDNBOOST_THREAD_TIME_HPP
#define NDNBOOST_THREAD_TIME_HPP
//  (C) Copyright 2007 Anthony Williams 
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/date_time/time_clock.hpp>
#include <ndnboost/date_time/microsec_time_clock.hpp>
#include <ndnboost/date_time/posix_time/posix_time_types.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{
    typedef ndnboost::posix_time::ptime system_time;
    
    inline system_time get_system_time()
    {
#if defined(NDNBOOST_DATE_TIME_HAS_HIGH_PRECISION_CLOCK)
        return ndnboost::date_time::microsec_clock<system_time>::universal_time();
#else // defined(NDNBOOST_DATE_TIME_HAS_HIGH_PRECISION_CLOCK)
        return ndnboost::date_time::second_clock<system_time>::universal_time();
#endif // defined(NDNBOOST_DATE_TIME_HAS_HIGH_PRECISION_CLOCK)
    }

    namespace detail
    {
        inline system_time get_system_time_sentinel()
        {
            return system_time(ndnboost::posix_time::pos_infin);
        }

        inline unsigned long get_milliseconds_until(system_time const& target_time)
        {
            if(target_time.is_pos_infinity())
            {
                return ~(unsigned long)0;
            }
            system_time const now=get_system_time();
            if(target_time<=now)
            {
                return 0;
            }
            return static_cast<unsigned long>((target_time-now).total_milliseconds()+1);
        }

    }
    
}

#include <ndnboost/config/abi_suffix.hpp>

#endif
