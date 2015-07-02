#ifndef NDNBOOST_THREAD_PTHREAD_ONCE_HPP
#define NDNBOOST_THREAD_PTHREAD_ONCE_HPP

//  once.hpp
//
//  (C) Copyright 2007-8 Anthony Williams
//  (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/invoke.hpp>

#include <ndnboost/thread/pthread/pthread_mutex_scoped_lock.hpp>
#include <ndnboost/thread/detail/delete.hpp>
#include <ndnboost/core/no_exceptions_support.hpp>

#include <ndnboost/bind.hpp>
#include <ndnboost/assert.hpp>
#include <ndnboost/config/abi_prefix.hpp>

#include <ndnboost/cstdint.hpp>
#include <pthread.h>
#include <csignal>

namespace ndnboost
{

  struct once_flag;

  #define NDNBOOST_ONCE_INITIAL_FLAG_VALUE 0

  namespace thread_detail
  {
    typedef ndnboost::uint32_t  uintmax_atomic_t;
    #define NDNBOOST_THREAD_DETAIL_UINTMAX_ATOMIC_C2(value) value##u
    #define NDNBOOST_THREAD_DETAIL_UINTMAX_ATOMIC_MAX_C NDNBOOST_THREAD_DETAIL_UINTMAX_ATOMIC_C2(~0)

  }

#ifdef NDNBOOST_THREAD_PROVIDES_ONCE_CXX11
#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
    template<typename Function, class ...ArgTypes>
    inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(ArgTypes)... args);
#else
    template<typename Function>
    inline void call_once(once_flag& flag, Function f);
    template<typename Function, typename T1>
    inline void call_once(once_flag& flag, Function f, T1 p1);
    template<typename Function, typename T1, typename T2>
    inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2);
    template<typename Function, typename T1, typename T2, typename T3>
    inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2, T3 p3);
#endif

  struct once_flag
  {
      NDNBOOST_THREAD_NO_COPYABLE(once_flag)
      NDNBOOST_CONSTEXPR once_flag() NDNBOOST_NOEXCEPT
        : epoch(NDNBOOST_ONCE_INITIAL_FLAG_VALUE)
      {}
  private:
      volatile thread_detail::uintmax_atomic_t epoch;

#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
      template<typename Function, class ...ArgTypes>
      friend void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(ArgTypes)... args);
#else
      template<typename Function>
      friend void call_once(once_flag& flag, Function f);
      template<typename Function, typename T1>
      friend void call_once(once_flag& flag, Function f, T1 p1);
      template<typename Function, typename T1, typename T2>
      friend void call_once(once_flag& flag, Function f, T1 p1, T2 p2);
      template<typename Function, typename T1, typename T2, typename T3>
      friend void call_once(once_flag& flag, Function f, T1 p1, T2 p2, T3 p3);

#endif

  };

#define NDNBOOST_ONCE_INIT once_flag()

#else // NDNBOOST_THREAD_PROVIDES_ONCE_CXX11

    struct once_flag
    {
      volatile thread_detail::uintmax_atomic_t epoch;
    };

#define NDNBOOST_ONCE_INIT {NDNBOOST_ONCE_INITIAL_FLAG_VALUE}
#endif // NDNBOOST_THREAD_PROVIDES_ONCE_CXX11


#if defined NDNBOOST_THREAD_PROVIDES_INVOKE
#define NDNBOOST_THREAD_INVOKE_RET_VOID detail::invoke
#define NDNBOOST_THREAD_INVOKE_RET_VOID_CALL
#elif defined NDNBOOST_THREAD_PROVIDES_INVOKE_RET
#define NDNBOOST_THREAD_INVOKE_RET_VOID detail::invoke<void>
#define NDNBOOST_THREAD_INVOKE_RET_VOID_CALL
#else
#define NDNBOOST_THREAD_INVOKE_RET_VOID ndnboost::bind
#define NDNBOOST_THREAD_INVOKE_RET_VOID_CALL ()
#endif

    namespace thread_detail
    {
        NDNBOOST_THREAD_DECL uintmax_atomic_t& get_once_per_thread_epoch();
        NDNBOOST_THREAD_DECL extern uintmax_atomic_t once_global_epoch;
        NDNBOOST_THREAD_DECL extern pthread_mutex_t once_epoch_mutex;
        NDNBOOST_THREAD_DECL extern pthread_cond_t once_epoch_cv;
    }

    // Based on Mike Burrows fast_pthread_once algorithm as described in
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2444.html


#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)


  template<typename Function, class ...ArgTypes>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(ArgTypes)... args)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
                    ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;

    }
  }
#else
  template<typename Function>
  inline void call_once(once_flag& flag, Function f)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    f();
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

  template<typename Function, typename T1>
  inline void call_once(once_flag& flag, Function f, T1 p1)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }
  template<typename Function, typename T1, typename T2>
  inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1, p2) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
        }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

  template<typename Function, typename T1, typename T2, typename T3>
  inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2, T3 p3)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1, p2, p3) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
        }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

  template<typename Function>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    f();
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

  template<typename Function, typename T1>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p1))
                    ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }
  template<typename Function, typename T1, typename T2>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p2))
                    ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

  template<typename Function, typename T1, typename T2, typename T3>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2, NDNBOOST_THREAD_RV_REF(T3) p3)
  {
    static thread_detail::uintmax_atomic_t const uninitialized_flag=NDNBOOST_ONCE_INITIAL_FLAG_VALUE;
    static thread_detail::uintmax_atomic_t const being_initialized=uninitialized_flag+1;
    thread_detail::uintmax_atomic_t const epoch=flag.epoch;
    thread_detail::uintmax_atomic_t& this_thread_epoch=thread_detail::get_once_per_thread_epoch();

    if(epoch<this_thread_epoch)
    {
        pthread::pthread_mutex_scoped_lock lk(&thread_detail::once_epoch_mutex);

        while(flag.epoch<=being_initialized)
        {
            if(flag.epoch==uninitialized_flag)
            {
                flag.epoch=being_initialized;
                NDNBOOST_TRY
                {
                    pthread::pthread_mutex_scoped_unlock relocker(&thread_detail::once_epoch_mutex);
                    NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p2)),
                        thread_detail::decay_copy(ndnboost::forward<T1>(p3))
                    ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH (...)
                {
                    flag.epoch=uninitialized_flag;
                    NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                flag.epoch=--thread_detail::once_global_epoch;
                NDNBOOST_VERIFY(!pthread_cond_broadcast(&thread_detail::once_epoch_cv));
            }
            else
            {
                while(flag.epoch==being_initialized)
                {
                    NDNBOOST_VERIFY(!pthread_cond_wait(&thread_detail::once_epoch_cv,&thread_detail::once_epoch_mutex));
                }
            }
        }
        this_thread_epoch=thread_detail::once_global_epoch;
    }
  }

#endif

}

#include <ndnboost/config/abi_suffix.hpp>

#endif
