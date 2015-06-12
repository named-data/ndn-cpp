#ifndef NDNBOOST_THREAD_PTHREAD_MUTEX_HPP
#define NDNBOOST_THREAD_PTHREAD_MUTEX_HPP
// (C) Copyright 2007-8 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/config.hpp>
#include <pthread.h>
#include <ndnboost/throw_exception.hpp>
#include <ndnboost/core/ignore_unused.hpp>
#include <ndnboost/thread/exceptions.hpp>
#if defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
#include <ndnboost/thread/lock_types.hpp>
#endif
#include <ndnboost/thread/thread_time.hpp>
#include <ndnboost/thread/xtime.hpp>
#include <ndnboost/assert.hpp>
#include <errno.h>
#include <ndnboost/thread/pthread/timespec.hpp>
#include <ndnboost/thread/pthread/pthread_mutex_scoped_lock.hpp>
#ifdef NDNBOOST_THREAD_USES_CHRONO
#include <ndnboost/chrono/system_clocks.hpp>
#include <ndnboost/chrono/ceil.hpp>
#endif
#include <ndnboost/thread/detail/delete.hpp>

#ifdef _POSIX_TIMEOUTS
#if _POSIX_TIMEOUTS >= 0 && _POSIX_TIMEOUTS>=200112L
#ifndef NDNBOOST_PTHREAD_HAS_TIMEDLOCK
#define NDNBOOST_PTHREAD_HAS_TIMEDLOCK
#endif
#endif
#endif


#include <ndnboost/config/abi_prefix.hpp>

#ifndef NDNBOOST_THREAD_HAS_NO_EINTR_BUG
#define NDNBOOST_THREAD_HAS_EINTR_BUG
#endif

namespace ndnboost
{
  namespace posix {
#ifdef NDNBOOST_THREAD_HAS_EINTR_BUG
    NDNBOOST_FORCEINLINE int pthread_mutex_destroy(pthread_mutex_t* m)
    {
      int ret;
      do
      {
          ret = ::pthread_mutex_destroy(m);
      } while (ret == EINTR);
      return ret;
    }
    NDNBOOST_FORCEINLINE int pthread_mutex_lock(pthread_mutex_t* m)
    {
      int ret;
      do
      {
          ret = ::pthread_mutex_lock(m);
      } while (ret == EINTR);
      return ret;
    }
    NDNBOOST_FORCEINLINE int pthread_mutex_unlock(pthread_mutex_t* m)
    {
      int ret;
      do
      {
          ret = ::pthread_mutex_unlock(m);
      } while (ret == EINTR);
      return ret;
    }
#else
    NDNBOOST_FORCEINLINE int pthread_mutex_destroy(pthread_mutex_t* m)
    {
      return ::pthread_mutex_destroy(m);
    }
    NDNBOOST_FORCEINLINE int pthread_mutex_lock(pthread_mutex_t* m)
    {
      return ::pthread_mutex_lock(m);
    }
    NDNBOOST_FORCEINLINE int pthread_mutex_unlock(pthread_mutex_t* m)
    {
      return ::pthread_mutex_unlock(m);
    }

#endif

  }
    class mutex
    {
    private:
        pthread_mutex_t m;
    public:
        NDNBOOST_THREAD_NO_COPYABLE(mutex)

        mutex()
        {
            int const res=pthread_mutex_init(&m,NULL);
            if(res)
            {
                ndnboost::throw_exception(thread_resource_error(res, "ndnboost:: mutex constructor failed in pthread_mutex_init"));
            }
        }
        ~mutex()
        {
          int const res = posix::pthread_mutex_destroy(&m);
          ndnboost::ignore_unused(res);
          NDNBOOST_ASSERT(!res);
        }

        void lock()
        {
            int res = posix::pthread_mutex_lock(&m);
            if (res)
            {
                ndnboost::throw_exception(lock_error(res,"boost: mutex lock failed in pthread_mutex_lock"));
            }
        }

        void unlock()
        {
            int res = posix::pthread_mutex_unlock(&m);
            if (res)
            {
                ndnboost::throw_exception(lock_error(res,"boost: mutex unlock failed in pthread_mutex_unlock"));
            }
        }

        bool try_lock()
        {
            int res;
            do
            {
                res = pthread_mutex_trylock(&m);
            } while (res == EINTR);
            if (res==EBUSY)
            {
                return false;
            }

            return !res;
        }

#define NDNBOOST_THREAD_DEFINES_MUTEX_NATIVE_HANDLE
        typedef pthread_mutex_t* native_handle_type;
        native_handle_type native_handle()
        {
            return &m;
        }

#if defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
        typedef unique_lock<mutex> scoped_lock;
        typedef detail::try_lock_wrapper<mutex> scoped_try_lock;
#endif
    };

    typedef mutex try_mutex;

    class timed_mutex
    {
    private:
        pthread_mutex_t m;
#ifndef NDNBOOST_PTHREAD_HAS_TIMEDLOCK
        pthread_cond_t cond;
        bool is_locked;
#endif
    public:
        NDNBOOST_THREAD_NO_COPYABLE(timed_mutex)
        timed_mutex()
        {
            int const res=pthread_mutex_init(&m,NULL);
            if(res)
            {
                ndnboost::throw_exception(thread_resource_error(res, "ndnboost:: timed_mutex constructor failed in pthread_mutex_init"));
            }
#ifndef NDNBOOST_PTHREAD_HAS_TIMEDLOCK
            int const res2=pthread_cond_init(&cond,NULL);
            if(res2)
            {
                NDNBOOST_VERIFY(!posix::pthread_mutex_destroy(&m));
                //NDNBOOST_VERIFY(!pthread_mutex_destroy(&m));
                ndnboost::throw_exception(thread_resource_error(res2, "ndnboost:: timed_mutex constructor failed in pthread_cond_init"));
            }
            is_locked=false;
#endif
        }
        ~timed_mutex()
        {
            NDNBOOST_VERIFY(!posix::pthread_mutex_destroy(&m));
#ifndef NDNBOOST_PTHREAD_HAS_TIMEDLOCK
            NDNBOOST_VERIFY(!pthread_cond_destroy(&cond));
#endif
        }

#if defined NDNBOOST_THREAD_USES_DATETIME
        template<typename TimeDuration>
        bool timed_lock(TimeDuration const & relative_time)
        {
            return timed_lock(get_system_time()+relative_time);
        }
        bool timed_lock(ndnboost::xtime const & absolute_time)
        {
            return timed_lock(system_time(absolute_time));
        }
#endif
#ifdef NDNBOOST_PTHREAD_HAS_TIMEDLOCK
        void lock()
        {
            int res = posix::pthread_mutex_lock(&m);
            if (res)
            {
                ndnboost::throw_exception(lock_error(res,"boost: mutex lock failed in pthread_mutex_lock"));
            }
        }

        void unlock()
        {
            int res = posix::pthread_mutex_unlock(&m);
            if (res)
            {
                ndnboost::throw_exception(lock_error(res,"boost: mutex unlock failed in pthread_mutex_unlock"));
            }
        }

        bool try_lock()
        {
          int res;
          do
          {
              res = pthread_mutex_trylock(&m);
          } while (res == EINTR);
          if (res==EBUSY)
          {
              return false;
          }

          return !res;
        }


    private:
        bool do_try_lock_until(struct timespec const &timeout)
        {
          int const res=pthread_mutex_timedlock(&m,&timeout);
          NDNBOOST_ASSERT(!res || res==ETIMEDOUT);
          return !res;
        }
    public:

#else
        void lock()
        {
            ndnboost::pthread::pthread_mutex_scoped_lock const local_lock(&m);
            while(is_locked)
            {
                NDNBOOST_VERIFY(!pthread_cond_wait(&cond,&m));
            }
            is_locked=true;
        }

        void unlock()
        {
            ndnboost::pthread::pthread_mutex_scoped_lock const local_lock(&m);
            is_locked=false;
            NDNBOOST_VERIFY(!pthread_cond_signal(&cond));
        }

        bool try_lock()
        {
            ndnboost::pthread::pthread_mutex_scoped_lock const local_lock(&m);
            if(is_locked)
            {
                return false;
            }
            is_locked=true;
            return true;
        }

    private:
        bool do_try_lock_until(struct timespec const &timeout)
        {
            ndnboost::pthread::pthread_mutex_scoped_lock const local_lock(&m);
            while(is_locked)
            {
                int const cond_res=pthread_cond_timedwait(&cond,&m,&timeout);
                if(cond_res==ETIMEDOUT)
                {
                    return false;
                }
                NDNBOOST_ASSERT(!cond_res);
            }
            is_locked=true;
            return true;
        }
    public:
#endif

#if defined NDNBOOST_THREAD_USES_DATETIME
        bool timed_lock(system_time const & abs_time)
        {
            struct timespec const ts=ndnboost::detail::to_timespec(abs_time);
            return do_try_lock_until(ts);
        }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
        template <class Rep, class Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& rel_time)
        {
          return try_lock_until(chrono::steady_clock::now() + rel_time);
        }
        template <class Clock, class Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& t)
        {
          using namespace chrono;
          system_clock::time_point     s_now = system_clock::now();
          typename Clock::time_point  c_now = Clock::now();
          return try_lock_until(s_now + ceil<nanoseconds>(t - c_now));
        }
        template <class Duration>
        bool try_lock_until(const chrono::time_point<chrono::system_clock, Duration>& t)
        {
          using namespace chrono;
          typedef time_point<system_clock, nanoseconds> nano_sys_tmpt;
          return try_lock_until(nano_sys_tmpt(ceil<nanoseconds>(t.time_since_epoch())));
        }
        bool try_lock_until(const chrono::time_point<chrono::system_clock, chrono::nanoseconds>& tp)
        {
          //using namespace chrono;
          chrono::nanoseconds d = tp.time_since_epoch();
          timespec ts = ndnboost::detail::to_timespec(d);
          return do_try_lock_until(ts);
        }
#endif

#define NDNBOOST_THREAD_DEFINES_TIMED_MUTEX_NATIVE_HANDLE
        typedef pthread_mutex_t* native_handle_type;
        native_handle_type native_handle()
        {
            return &m;
        }

#if defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
        typedef unique_lock<timed_mutex> scoped_timed_lock;
        typedef detail::try_lock_wrapper<timed_mutex> scoped_try_lock;
        typedef scoped_timed_lock scoped_lock;
#endif
    };

}

#include <ndnboost/config/abi_suffix.hpp>


#endif
