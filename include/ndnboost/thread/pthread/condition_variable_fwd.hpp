#ifndef NDNBOOST_THREAD_PTHREAD_CONDITION_VARIABLE_FWD_HPP
#define NDNBOOST_THREAD_PTHREAD_CONDITION_VARIABLE_FWD_HPP
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007-8 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba

#include <ndnboost/assert.hpp>
#include <ndnboost/throw_exception.hpp>
#include <pthread.h>
#include <ndnboost/thread/cv_status.hpp>
#include <ndnboost/thread/mutex.hpp>
#include <ndnboost/thread/lock_types.hpp>
#include <ndnboost/thread/thread_time.hpp>
#include <ndnboost/thread/pthread/timespec.hpp>
#if defined NDNBOOST_THREAD_USES_DATETIME
#include <ndnboost/thread/xtime.hpp>
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
#include <ndnboost/chrono/system_clocks.hpp>
#include <ndnboost/chrono/ceil.hpp>
#endif
#include <ndnboost/thread/detail/delete.hpp>
#include <ndnboost/date_time/posix_time/posix_time_duration.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{

    class condition_variable
    {
    private:
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        pthread_mutex_t internal_mutex;
#endif
        pthread_cond_t cond;

    public:
    //private: // used by ndnboost::thread::try_join_until

        inline bool do_wait_until(
            unique_lock<mutex>& lock,
            struct timespec const &timeout);

        bool do_wait_for(
            unique_lock<mutex>& lock,
            struct timespec const &timeout)
        {
          return do_wait_until(lock, ndnboost::detail::timespec_plus(timeout, ndnboost::detail::timespec_now()));
        }

    public:
      NDNBOOST_THREAD_NO_COPYABLE(condition_variable)
        condition_variable()
        {
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            int const res=pthread_mutex_init(&internal_mutex,NULL);
            if(res)
            {
                ndnboost::throw_exception(thread_resource_error(res, "ndnboost::condition_variable::condition_variable() constructor failed in pthread_mutex_init"));
            }
#endif
            int const res2=pthread_cond_init(&cond,NULL);
            if(res2)
            {
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                NDNBOOST_VERIFY(!pthread_mutex_destroy(&internal_mutex));
#endif
                ndnboost::throw_exception(thread_resource_error(res2, "ndnboost::condition_variable::condition_variable() constructor failed in pthread_cond_init"));
            }
        }
        ~condition_variable()
        {
            int ret;
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            do {
              ret = pthread_mutex_destroy(&internal_mutex);
            } while (ret == EINTR);
            NDNBOOST_ASSERT(!ret);
#endif
            do {
              ret = pthread_cond_destroy(&cond);
            } while (ret == EINTR);
            NDNBOOST_ASSERT(!ret);
        }

        void wait(unique_lock<mutex>& m);

        template<typename predicate_type>
        void wait(unique_lock<mutex>& m,predicate_type pred)
        {
            while(!pred()) wait(m);
        }


#if defined NDNBOOST_THREAD_USES_DATETIME
        inline bool timed_wait(
            unique_lock<mutex>& m,
            ndnboost::system_time const& abs_time)
        {
#if defined NDNBOOST_THREAD_WAIT_BUG
            struct timespec const timeout=detail::to_timespec(abs_time + NDNBOOST_THREAD_WAIT_BUG);
            return do_wait_until(m, timeout);
#else
            struct timespec const timeout=detail::to_timespec(abs_time);
            return do_wait_until(m, timeout);
#endif
        }
        bool timed_wait(
            unique_lock<mutex>& m,
            xtime const& abs_time)
        {
            return timed_wait(m,system_time(abs_time));
        }

        template<typename duration_type>
        bool timed_wait(
            unique_lock<mutex>& m,
            duration_type const& wait_duration)
        {
            return timed_wait(m,get_system_time()+wait_duration);
        }

        template<typename predicate_type>
        bool timed_wait(
            unique_lock<mutex>& m,
            ndnboost::system_time const& abs_time,predicate_type pred)
        {
            while (!pred())
            {
                if(!timed_wait(m, abs_time))
                    return pred();
            }
            return true;
        }

        template<typename predicate_type>
        bool timed_wait(
            unique_lock<mutex>& m,
            xtime const& abs_time,predicate_type pred)
        {
            return timed_wait(m,system_time(abs_time),pred);
        }

        template<typename duration_type,typename predicate_type>
        bool timed_wait(
            unique_lock<mutex>& m,
            duration_type const& wait_duration,predicate_type pred)
        {
            return timed_wait(m,get_system_time()+wait_duration,pred);
        }
#endif

#ifdef NDNBOOST_THREAD_USES_CHRONO

        template <class Duration>
        cv_status
        wait_until(
                unique_lock<mutex>& lock,
                const chrono::time_point<chrono::system_clock, Duration>& t)
        {
          using namespace chrono;
          typedef time_point<system_clock, nanoseconds> nano_sys_tmpt;
          wait_until(lock,
                        nano_sys_tmpt(ceil<nanoseconds>(t.time_since_epoch())));
          return system_clock::now() < t ? cv_status::no_timeout :
                                             cv_status::timeout;
        }

        template <class Clock, class Duration>
        cv_status
        wait_until(
                unique_lock<mutex>& lock,
                const chrono::time_point<Clock, Duration>& t)
        {
          using namespace chrono;
          system_clock::time_point     s_now = system_clock::now();
          typename Clock::time_point  c_now = Clock::now();
          wait_until(lock, s_now + ceil<nanoseconds>(t - c_now));
          return Clock::now() < t ? cv_status::no_timeout : cv_status::timeout;
        }

        template <class Clock, class Duration, class Predicate>
        bool
        wait_until(
                unique_lock<mutex>& lock,
                const chrono::time_point<Clock, Duration>& t,
                Predicate pred)
        {
            while (!pred())
            {
                if (wait_until(lock, t) == cv_status::timeout)
                    return pred();
            }
            return true;
        }


        template <class Rep, class Period>
        cv_status
        wait_for(
                unique_lock<mutex>& lock,
                const chrono::duration<Rep, Period>& d)
        {
          using namespace chrono;
          system_clock::time_point s_now = system_clock::now();
          steady_clock::time_point c_now = steady_clock::now();
          wait_until(lock, s_now + ceil<nanoseconds>(d));
          return steady_clock::now() - c_now < d ? cv_status::no_timeout :
                                                   cv_status::timeout;

        }


        template <class Rep, class Period, class Predicate>
        bool
        wait_for(
                unique_lock<mutex>& lock,
                const chrono::duration<Rep, Period>& d,
                Predicate pred)
        {
          return wait_until(lock, chrono::steady_clock::now() + d, ndnboost::move(pred));

//          while (!pred())
//          {
//              if (wait_for(lock, d) == cv_status::timeout)
//                  return pred();
//          }
//          return true;
        }
#endif

#define NDNBOOST_THREAD_DEFINES_CONDITION_VARIABLE_NATIVE_HANDLE
        typedef pthread_cond_t* native_handle_type;
        native_handle_type native_handle()
        {
            return &cond;
        }

        void notify_one() NDNBOOST_NOEXCEPT;
        void notify_all() NDNBOOST_NOEXCEPT;

#ifdef NDNBOOST_THREAD_USES_CHRONO
        inline cv_status wait_until(
            unique_lock<mutex>& lk,
            chrono::time_point<chrono::system_clock, chrono::nanoseconds> tp)
        {
            using namespace chrono;
            nanoseconds d = tp.time_since_epoch();
            timespec ts = ndnboost::detail::to_timespec(d);
            if (do_wait_until(lk, ts)) return cv_status::no_timeout;
            else return cv_status::timeout;
        }
#endif
    };

    NDNBOOST_THREAD_DECL void notify_all_ndnboostat_thread_exit(condition_variable& cond, unique_lock<mutex> lk);

}


#include <ndnboost/config/abi_suffix.hpp>

#endif
