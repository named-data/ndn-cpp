// Copyright (C) 2002-2003
// David Moore, William E. Kempf
// Copyright (C) 2007-8 Anthony Williams
// (C) Copyright 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_BARRIER_JDM030602_HPP
#define NDNBOOST_BARRIER_JDM030602_HPP

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/thread/detail/delete.hpp>

#include <ndnboost/throw_exception.hpp>
#include <ndnboost/thread/mutex.hpp>
#include <ndnboost/thread/lock_types.hpp>
#include <ndnboost/thread/condition_variable.hpp>
#include <string>
#include <stdexcept>
#include <ndnboost/thread/detail/nullary_function.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/is_void.hpp>
#include <ndnboost/core/enable_if.hpp>
#include <ndnboost/utility/result_of.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{
  namespace thread_detail
  {
    typedef detail::nullary_function<void()> void_completion_function;
    typedef detail::nullary_function<size_t()> size_completion_function;

    struct default_barrier_reseter
    {
      unsigned int size_;
      default_barrier_reseter(unsigned int size) :
        size_(size)
      {
      }
      NDNBOOST_THREAD_MOVABLE(default_barrier_reseter)

      default_barrier_reseter(default_barrier_reseter const& other) NDNBOOST_NOEXCEPT :
      size_(other.size_)
      {
      }
      default_barrier_reseter(NDNBOOST_THREAD_RV_REF(default_barrier_reseter) other) NDNBOOST_NOEXCEPT :
      size_(NDNBOOST_THREAD_RV(other).size_)
      {
      }

      unsigned int operator()()
      {
        return size_;
      }
    };

    struct void_functor_barrier_reseter
    {
      unsigned int size_;
      void_completion_function fct_;
      template <typename F>
      void_functor_barrier_reseter(unsigned int size, NDNBOOST_THREAD_RV_REF(F) funct)
      : size_(size), fct_(ndnboost::move(funct))
      {}
      template <typename F>
      void_functor_barrier_reseter(unsigned int size, F& funct)
      : size_(size), fct_(funct)
      {}

      NDNBOOST_THREAD_MOVABLE(void_functor_barrier_reseter)

      void_functor_barrier_reseter(void_functor_barrier_reseter const& other) NDNBOOST_NOEXCEPT :
      size_(other.size_), fct_(other.fct_)
      {
      }
      void_functor_barrier_reseter(NDNBOOST_THREAD_RV_REF(void_functor_barrier_reseter) other) NDNBOOST_NOEXCEPT :
      size_(NDNBOOST_THREAD_RV(other).size_), fct_(NDNBOOST_THREAD_RV(other).fct_)
      {
      }

      unsigned int operator()()
      {
        fct_();
        return size_;
      }
    };
    struct void_fct_ptr_barrier_reseter
    {
      unsigned int size_;
      void(*fct_)();
      void_fct_ptr_barrier_reseter(unsigned int size, void(*funct)()) :
        size_(size), fct_(funct)
      {
      }
      NDNBOOST_THREAD_MOVABLE(void_fct_ptr_barrier_reseter)

      void_fct_ptr_barrier_reseter(void_fct_ptr_barrier_reseter const& other) NDNBOOST_NOEXCEPT :
      size_(other.size_), fct_(other.fct_)
      {
      }
      void_fct_ptr_barrier_reseter(NDNBOOST_THREAD_RV_REF(void_fct_ptr_barrier_reseter) other) NDNBOOST_NOEXCEPT :
      size_(NDNBOOST_THREAD_RV(other).size_), fct_(NDNBOOST_THREAD_RV(other).fct_)
      {
      }
      unsigned int operator()()
      {
        fct_();
        return size_;
      }
    };
  }
  class barrier
  {
    static inline unsigned int check_counter(unsigned int count)
    {
      if (count == 0) ndnboost::throw_exception(
          thread_exception(system::errc::invalid_argument, "barrier constructor: count cannot be zero."));
      return count;
    }
    struct dummy
    {
    };

  public:
    NDNBOOST_THREAD_NO_COPYABLE( barrier)

    explicit barrier(unsigned int count) :
      m_count(check_counter(count)), m_generation(0), fct_(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::default_barrier_reseter(count)))
    {
    }

    template <typename F>
    barrier(
        unsigned int count,
        NDNBOOST_THREAD_RV_REF(F) funct,
        typename enable_if<
        typename is_void<typename result_of<F>::type>::type, dummy*
        >::type=0
    )
    : m_count(check_counter(count)),
      m_generation(0),
      fct_(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::void_functor_barrier_reseter(count,
        ndnboost::move(funct)))
    )
    {
    }
    template <typename F>
    barrier(
        unsigned int count,
        F &funct,
        typename enable_if<
        typename is_void<typename result_of<F>::type>::type, dummy*
        >::type=0
    )
    : m_count(check_counter(count)),
      m_generation(0),
      fct_(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::void_functor_barrier_reseter(count,
        funct))
    )
    {
    }

    template <typename F>
    barrier(
        unsigned int count,
        NDNBOOST_THREAD_RV_REF(F) funct,
        typename enable_if<
        typename is_same<typename result_of<F>::type, unsigned int>::type, dummy*
        >::type=0
    )
    : m_count(check_counter(count)),
      m_generation(0),
      fct_(ndnboost::move(funct))
    {
    }
    template <typename F>
    barrier(
        unsigned int count,
        F& funct,
        typename enable_if<
        typename is_same<typename result_of<F>::type, unsigned int>::type, dummy*
        >::type=0
    )
    : m_count(check_counter(count)),
      m_generation(0),
      fct_(funct)
    {
    }

    barrier(unsigned int count, void(*funct)()) :
      m_count(check_counter(count)), m_generation(0),
      fct_(funct
          ? NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::size_completion_function(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::void_fct_ptr_barrier_reseter(count, funct))))
          : NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::size_completion_function(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::default_barrier_reseter(count))))
      )
    {
    }
    barrier(unsigned int count, unsigned int(*funct)()) :
      m_count(check_counter(count)), m_generation(0),
      fct_(funct
          ? NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::size_completion_function(funct))
          : NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::size_completion_function(NDNBOOST_THREAD_MAKE_RV_REF(thread_detail::default_barrier_reseter(count))))
      )
    {
    }

    bool wait()
    {
      ndnboost::unique_lock < ndnboost::mutex > lock(m_mutex);
      unsigned int gen = m_generation;

      if (--m_count == 0)
      {
        m_generation++;
        m_count = static_cast<unsigned int>(fct_());
        NDNBOOST_ASSERT(m_count != 0);
        m_cond.notify_all();
        return true;
      }

      while (gen == m_generation)
        m_cond.wait(lock);
      return false;
    }

    void count_down_and_wait()
    {
      wait();
    }

  private:
    mutex m_mutex;
    condition_variable m_cond;
    unsigned int m_count;
    unsigned int m_generation;
    thread_detail::size_completion_function fct_;
  };

} // namespace ndnboost

#include <ndnboost/config/abi_suffix.hpp>

#endif
