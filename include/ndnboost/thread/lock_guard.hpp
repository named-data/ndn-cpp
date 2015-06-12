// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba

#ifndef NDNBOOST_THREAD_LOCK_GUARD_HPP
#define NDNBOOST_THREAD_LOCK_GUARD_HPP

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/thread/detail/delete.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/lockable_wrapper.hpp>
#include <ndnboost/thread/lock_options.hpp>
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
#include <ndnboost/thread/is_locked_by_this_thread.hpp>
#include <ndnboost/assert.hpp>
#endif

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{

  template <typename Mutex>
  class lock_guard
  {
  private:
    Mutex& m;

  public:
    typedef Mutex mutex_type;
    NDNBOOST_THREAD_NO_COPYABLE( lock_guard )

    explicit lock_guard(Mutex& m_) :
      m(m_)
    {
      m.lock();
    }

    lock_guard(Mutex& m_, adopt_lock_t) :
      m(m_)
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m));
#endif
    }

#if ! defined NDNBOOST_THREAD_NO_CXX11_HDR_INITIALIZER_LIST
    lock_guard(std::initializer_list<thread_detail::lockable_wrapper<Mutex> > l_) :
      m(*(const_cast<thread_detail::lockable_wrapper<Mutex>*>(l_.begin())->m))
    {
      m.lock();
    }

    lock_guard(std::initializer_list<thread_detail::lockable_adopt_wrapper<Mutex> > l_) :
      m(*(const_cast<thread_detail::lockable_adopt_wrapper<Mutex>*>(l_.begin())->m))
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m));
#endif
    }

#endif
    ~lock_guard()
    {
      m.unlock();
    }
  };


#if ! defined NDNBOOST_THREAD_NO_MAKE_LOCK_GUARD
  template <typename Lockable>
  lock_guard<Lockable> make_lock_guard(Lockable& mtx)
  {
    return { thread_detail::lockable_wrapper<Lockable>(mtx) };
  }
  template <typename Lockable>
  lock_guard<Lockable> make_lock_guard(Lockable& mtx, adopt_lock_t)
  {
    return { thread_detail::lockable_adopt_wrapper<Lockable>(mtx) };
  }
#endif
}

#include <ndnboost/config/abi_suffix.hpp>

#endif
