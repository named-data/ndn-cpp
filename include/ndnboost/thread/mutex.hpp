#ifndef NDNBOOST_THREAD_MUTEX_HPP
#define NDNBOOST_THREAD_MUTEX_HPP

//  mutex.hpp
//
//  (C) Copyright 2007 Anthony Williams
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/platform.hpp>
#if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#include <ndnboost/thread/win32/mutex.hpp>
#elif defined(NDNBOOST_THREAD_PLATFORM_PTHREAD)
#include <ndnboost/thread/pthread/mutex.hpp>
#else
#error "Boost threads unavailable on this platform"
#endif

#include <ndnboost/thread/lockable_traits.hpp>


namespace ndnboost
{
  namespace sync
  {
#ifdef NDNBOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
    template<>
    struct is_basic_lockable<mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_lockable<mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_basic_lockable<timed_mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_lockable<timed_mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
#endif
  }
}

#endif
