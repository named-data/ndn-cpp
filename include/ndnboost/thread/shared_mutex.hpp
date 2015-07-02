#ifndef NDNBOOST_THREAD_SHARED_MUTEX_HPP
#define NDNBOOST_THREAD_SHARED_MUTEX_HPP

//  shared_mutex.hpp
//
//  (C) Copyright 2007 Anthony Williams
//  (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/config.hpp>
#if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#if defined(NDNBOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN)
#include <ndnboost/thread/pthread/shared_mutex.hpp>
#else
#include <ndnboost/thread/win32/shared_mutex.hpp>
#endif
#elif defined(NDNBOOST_THREAD_PLATFORM_PTHREAD)
//#include <ndnboost/thread/v2/shared_mutex.hpp>
#include <ndnboost/thread/pthread/shared_mutex.hpp>
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
    struct is_basic_lockable<shared_mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_lockable<shared_mutex>
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
#endif

  }
}

#endif
