#ifndef NDNBOOST_THREAD_DETAIL_INTERLOCKED_READ_WIN32_HPP
#define NDNBOOST_THREAD_DETAIL_INTERLOCKED_READ_WIN32_HPP

//  interlocked_read_win32.hpp
//
//  (C) Copyright 2005-8 Anthony Williams
//  (C) Copyright 2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/detail/interlocked.hpp>
#include <ndnboost/thread/detail/config.hpp>

#include <ndnboost/config/abi_prefix.hpp>

#ifdef NDNBOOST_MSVC

extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

namespace ndnboost
{
    namespace detail
    {
        inline long interlocked_read_acquire(long volatile* x) NDNBOOST_NOEXCEPT
        {
            long const res=*x;
            _ReadWriteBarrier();
            return res;
        }
        inline void* interlocked_read_acquire(void* volatile* x) NDNBOOST_NOEXCEPT
        {
            void* const res=*x;
            _ReadWriteBarrier();
            return res;
        }

        inline void interlocked_write_release(long volatile* x,long value) NDNBOOST_NOEXCEPT
        {
            _ReadWriteBarrier();
            *x=value;
        }
        inline void interlocked_write_release(void* volatile* x,void* value) NDNBOOST_NOEXCEPT
        {
            _ReadWriteBarrier();
            *x=value;
        }
    }
}

#else

namespace ndnboost
{
    namespace detail
    {
        inline long interlocked_read_acquire(long volatile* x) NDNBOOST_NOEXCEPT
        {
            return NDNBOOST_INTERLOCKED_COMPARE_EXCHANGE(x,0,0);
        }
        inline void* interlocked_read_acquire(void* volatile* x) NDNBOOST_NOEXCEPT
        {
            return NDNBOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(x,0,0);
        }
        inline void interlocked_write_release(long volatile* x,long value) NDNBOOST_NOEXCEPT
        {
            NDNBOOST_INTERLOCKED_EXCHANGE(x,value);
        }
        inline void interlocked_write_release(void* volatile* x,void* value) NDNBOOST_NOEXCEPT
        {
            NDNBOOST_INTERLOCKED_EXCHANGE_POINTER(x,value);
        }
    }
}

#endif

#include <ndnboost/config/abi_suffix.hpp>

#endif
