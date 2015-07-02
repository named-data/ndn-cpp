#ifndef NDNBOOST_THREAD_PTHREAD_ONCE_ATOMIC_HPP
#define NDNBOOST_THREAD_PTHREAD_ONCE_ATOMIC_HPP

//  once.hpp
//
//  (C) Copyright 2013 Andrey Semashev
//  (C) Copyright 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/config.hpp>

#include <ndnboost/cstdint.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/invoke.hpp>
#include <ndnboost/core/no_exceptions_support.hpp>
#include <ndnboost/bind.hpp>
#include <ndnboost/atomic.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{

  struct once_flag;

  namespace thread_detail
  {

#if NDNBOOST_ATOMIC_INT_LOCK_FREE == 2
    typedef unsigned int atomic_int_type;
#elif NDNBOOST_ATOMIC_SHORT_LOCK_FREE == 2
    typedef unsigned short atomic_int_type;
#elif NDNBOOST_ATOMIC_CHAR_LOCK_FREE == 2
    typedef unsigned char atomic_int_type;
#elif NDNBOOST_ATOMIC_LONG_LOCK_FREE == 2
    typedef unsigned long atomic_int_type;
#elif defined(NDNBOOST_HAS_LONG_LONG) && NDNBOOST_ATOMIC_LLONG_LOCK_FREE == 2
    typedef ulong_long_type atomic_int_type;
#else
    // All tested integer types are not atomic, the spinlock pool will be used
    typedef unsigned int atomic_int_type;
#endif

    typedef ndnboost::atomic<atomic_int_type> atomic_type;

    NDNBOOST_THREAD_DECL bool enter_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    NDNBOOST_THREAD_DECL void commit_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    NDNBOOST_THREAD_DECL void rollback_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    inline atomic_type& get_atomic_storage(once_flag& flag)  NDNBOOST_NOEXCEPT;
  }

#ifdef NDNBOOST_THREAD_PROVIDES_ONCE_CXX11

  struct once_flag
  {
    NDNBOOST_THREAD_NO_COPYABLE(once_flag)
    NDNBOOST_CONSTEXPR once_flag() NDNBOOST_NOEXCEPT : storage(0)
    {
    }

  private:
    thread_detail::atomic_type storage;

    friend NDNBOOST_THREAD_DECL bool thread_detail::enter_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    friend NDNBOOST_THREAD_DECL void thread_detail::commit_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    friend NDNBOOST_THREAD_DECL void thread_detail::rollback_once_region(once_flag& flag) NDNBOOST_NOEXCEPT;
    friend thread_detail::atomic_type& thread_detail::get_atomic_storage(once_flag& flag) NDNBOOST_NOEXCEPT;
  };

#define NDNBOOST_ONCE_INIT ndnboost::once_flag()

  namespace thread_detail
  {
    inline atomic_type& get_atomic_storage(once_flag& flag) NDNBOOST_NOEXCEPT
    {
      //return reinterpret_cast< atomic_type& >(flag.storage);
      return flag.storage;
    }
  }

#else // NDNBOOST_THREAD_PROVIDES_ONCE_CXX11
  struct once_flag
  {
    // The thread_detail::atomic_int_type storage is marked
    // with this attribute in order to let the compiler know that it will alias this member
    // and silence compilation warnings.
    NDNBOOST_THREAD_ATTRIBUTE_MAY_ALIAS thread_detail::atomic_int_type storage;
  };

  #define NDNBOOST_ONCE_INIT {0}

  namespace thread_detail
  {
    inline atomic_type& get_atomic_storage(once_flag& flag) NDNBOOST_NOEXCEPT
    {
      return reinterpret_cast< atomic_type& >(flag.storage);
    }

  }

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


#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)

  template<typename Function, class ...ArgTypes>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(ArgTypes)... args)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
        ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }
#else
  template<typename Function>
  inline void call_once(once_flag& flag, Function f)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        f();
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }

  template<typename Function, typename T1>
  inline void call_once(once_flag& flag, Function f, T1 p1)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(f, p1) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }

  template<typename Function, typename T1, typename T2>
  inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(f, p1, p2) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }

  template<typename Function, typename T1, typename T2, typename T3>
  inline void call_once(once_flag& flag, Function f, T1 p1, T2 p2, T3 p3)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(f, p1, p2, p3) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }

  template<typename Function>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        f();
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }

  template<typename Function, typename T1>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(
            thread_detail::decay_copy(ndnboost::forward<Function>(f)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p1))
        ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }
  template<typename Function, typename T1, typename T2>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(
            thread_detail::decay_copy(ndnboost::forward<Function>(f)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p2))
        ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }
  template<typename Function, typename T1, typename T2, typename T3>
  inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2, NDNBOOST_THREAD_RV_REF(T3) p3)
  {
    if (thread_detail::enter_once_region(flag))
    {
      NDNBOOST_TRY
      {
        NDNBOOST_THREAD_INVOKE_RET_VOID(
            thread_detail::decay_copy(ndnboost::forward<Function>(f)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p2)),
            thread_detail::decay_copy(ndnboost::forward<T1>(p3))
        ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;

      }
      NDNBOOST_CATCH (...)
      {
        thread_detail::rollback_once_region(flag);
        NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      thread_detail::commit_once_region(flag);
    }
  }



#endif
}

#include <ndnboost/config/abi_suffix.hpp>

#endif

