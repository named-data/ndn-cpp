// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba

#ifndef NDNBOOST_THREAD_LOCK_TYPES_HPP
#define NDNBOOST_THREAD_LOCK_TYPES_HPP

#include <ndnboost/thread/detail/config.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/exceptions.hpp>
#include <ndnboost/thread/lock_options.hpp>
#include <ndnboost/thread/lockable_traits.hpp>
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
#include <ndnboost/thread/is_locked_by_this_thread.hpp>
#endif
#include <ndnboost/thread/thread_time.hpp>

#include <ndnboost/assert.hpp>
#ifdef NDNBOOST_THREAD_USES_CHRONO
#include <ndnboost/chrono/time_point.hpp>
#include <ndnboost/chrono/duration.hpp>
#endif
#include <ndnboost/detail/workaround.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{
  struct xtime;

  template <typename Mutex>
  class shared_lock;

  template <typename Mutex>
  class upgrade_lock;

  template <typename Mutex>
  class unique_lock;

  namespace detail
  {
    template <typename Mutex>
    class try_lock_wrapper;
  }

#ifdef NDNBOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
  namespace sync
  {
    template<typename T>
    struct is_basic_lockable<unique_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<typename T>
    struct is_lockable<unique_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };

    template<typename T>
    struct is_basic_lockable<shared_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<typename T>
    struct is_lockable<shared_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };

    template<typename T>
    struct is_basic_lockable<upgrade_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<typename T>
    struct is_lockable<upgrade_lock<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };

    template<typename T>
    struct is_basic_lockable<detail::try_lock_wrapper<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<typename T>
    struct is_lockable<detail::try_lock_wrapper<T> >
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = true);
    };
  }
#endif


  template <typename Mutex>
  class unique_lock
  {
  private:
    Mutex* m;
    bool is_locked;

  private:
    explicit unique_lock(upgrade_lock<Mutex>&);
    unique_lock& operator=(upgrade_lock<Mutex>& other);
  public:
    typedef Mutex mutex_type;
    NDNBOOST_THREAD_MOVABLE_ONLY( unique_lock)

#if 0 // This should not be needed anymore. Use instead NDNBOOST_THREAD_MAKE_RV_REF.
#if NDNBOOST_WORKAROUND(__SUNPRO_CC, < 0x5100)
    unique_lock(const volatile unique_lock&);
#endif
#endif
    unique_lock()NDNBOOST_NOEXCEPT :
    m(0),is_locked(false)
    {}

    explicit unique_lock(Mutex& m_) :
      m(&m_), is_locked(false)
    {
      lock();
    }
    unique_lock(Mutex& m_, adopt_lock_t) :
      m(&m_), is_locked(true)
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m));
#endif
    }
    unique_lock(Mutex& m_, defer_lock_t)NDNBOOST_NOEXCEPT:
    m(&m_),is_locked(false)
    {}
    unique_lock(Mutex& m_, try_to_lock_t) :
      m(&m_), is_locked(false)
    {
      try_lock();
    }
#if defined NDNBOOST_THREAD_USES_DATETIME
    template<typename TimeDuration>
    unique_lock(Mutex& m_,TimeDuration const& target_time):
    m(&m_),is_locked(false)
    {
      timed_lock(target_time);
    }
    unique_lock(Mutex& m_,system_time const& target_time):
    m(&m_),is_locked(false)
    {
      timed_lock(target_time);
    }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    unique_lock(Mutex& mtx, const chrono::time_point<Clock, Duration>& t)
    : m(&mtx), is_locked(mtx.try_lock_until(t))
    {
    }
    template <class Rep, class Period>
    unique_lock(Mutex& mtx, const chrono::duration<Rep, Period>& d)
    : m(&mtx), is_locked(mtx.try_lock_for(d))
    {
    }
#endif

    unique_lock(NDNBOOST_THREAD_RV_REF(unique_lock) other) NDNBOOST_NOEXCEPT:
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other);

#ifndef NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
    //std-2104 unique_lock move-assignment should not be noexcept
    unique_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) //NDNBOOST_NOEXCEPT
    {
      unique_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }
#endif

    //std-2104 unique_lock move-assignment should not be noexcept
    unique_lock& operator=(NDNBOOST_THREAD_RV_REF(unique_lock) other) //NDNBOOST_NOEXCEPT
    {
      unique_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }
#if 0 // This should not be needed anymore. Use instead NDNBOOST_THREAD_MAKE_RV_REF.
#if NDNBOOST_WORKAROUND(__SUNPRO_CC, < 0x5100)
    unique_lock& operator=(unique_lock<Mutex> other)
    {
      swap(other);
      return *this;
    }
#endif // NDNBOOST_WORKAROUND
#endif

    // Conversion from upgrade locking
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END ul, try_to_lock_t)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(ul).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(ul).mutex()->try_unlock_upgrade_and_lock())
        {
          m = NDNBOOST_THREAD_RV(ul).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(ul).release();
      }
    }

#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END ul,
        const chrono::time_point<Clock, Duration>& abs_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(ul).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(ul).mutex()->try_unlock_upgrade_and_lock_until(abs_time))
        {
          m = NDNBOOST_THREAD_RV(ul).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(ul).release();
      }
    }

    template <class Rep, class Period>
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END ul,
        const chrono::duration<Rep, Period>& rel_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(ul).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(ul).mutex()->try_unlock_upgrade_and_lock_for(rel_time))
        {
          m = NDNBOOST_THREAD_RV(ul).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(ul).release();
      }
    }
#endif

#ifdef NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
    // Conversion from shared locking
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl, try_to_lock_t)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock())
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }

#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl,
        const chrono::time_point<Clock, Duration>& abs_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock_until(abs_time))
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }

    template <class Rep, class Period>
    unique_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl,
        const chrono::duration<Rep, Period>& rel_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock_for(rel_time))
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }
#endif // NDNBOOST_THREAD_USES_CHRONO
#endif // NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS

    void swap(unique_lock& other)NDNBOOST_NOEXCEPT
    {
      std::swap(m,other.m);
      std::swap(is_locked,other.is_locked);
    }

    ~unique_lock()
    {
      if (owns_lock())
      {
        m->unlock();
      }
    }
    void lock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if (owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      m->lock();
      is_locked = true;
    }
    bool try_lock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if (owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked = m->try_lock();
      return is_locked;
    }
#if defined NDNBOOST_THREAD_USES_DATETIME
    template<typename TimeDuration>
    bool timed_lock(TimeDuration const& relative_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked=m->timed_lock(relative_time);
      return is_locked;
    }

    bool timed_lock(::ndnboost::system_time const& absolute_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked=m->timed_lock(absolute_time);
      return is_locked;
    }
    bool timed_lock(::ndnboost::xtime const& absolute_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked=m->timed_lock(absolute_time);
      return is_locked;
    }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO

    template <class Rep, class Period>
    bool try_lock_for(const chrono::duration<Rep, Period>& rel_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked=m->try_lock_for(rel_time);
      return is_locked;
    }
    template <class Clock, class Duration>
    bool try_lock_until(const chrono::time_point<Clock, Duration>& abs_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost unique_lock owns already the mutex"));
      }
      is_locked=m->try_lock_until(abs_time);
      return is_locked;
    }
#endif

    void unlock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock has no mutex"));
      }
      if (!owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost unique_lock doesn't own the mutex"));
      }
      m->unlock();
      is_locked = false;
    }

#if defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    typedef void (unique_lock::*bool_type)();
    operator bool_type() const NDNBOOST_NOEXCEPT
    {
      return is_locked?&unique_lock::lock:0;
    }
    bool operator!() const NDNBOOST_NOEXCEPT
    {
      return !owns_lock();
    }
#else
    explicit operator bool() const NDNBOOST_NOEXCEPT
    {
      return owns_lock();
    }
#endif
    bool owns_lock() const NDNBOOST_NOEXCEPT
    {
      return is_locked;
    }

    Mutex* mutex() const NDNBOOST_NOEXCEPT
    {
      return m;
    }

    Mutex* release()NDNBOOST_NOEXCEPT
    {
      Mutex* const res=m;
      m=0;
      is_locked=false;
      return res;
    }

    friend class shared_lock<Mutex> ;
    friend class upgrade_lock<Mutex> ;
  };

  template<typename Mutex>
  void swap(unique_lock<Mutex>& lhs, unique_lock<Mutex>& rhs)
  NDNBOOST_NOEXCEPT
  {
    lhs.swap(rhs);
  }

  NDNBOOST_THREAD_DCL_MOVABLE_BEG(Mutex) unique_lock<Mutex> NDNBOOST_THREAD_DCL_MOVABLE_END

  template<typename Mutex>
  class shared_lock
  {
  protected:
    Mutex* m;
    bool is_locked;

  public:
    typedef Mutex mutex_type;
    NDNBOOST_THREAD_MOVABLE_ONLY(shared_lock)

    shared_lock() NDNBOOST_NOEXCEPT:
    m(0),is_locked(false)
    {}

    explicit shared_lock(Mutex& m_):
    m(&m_),is_locked(false)
    {
      lock();
    }
    shared_lock(Mutex& m_,adopt_lock_t):
    m(&m_),is_locked(true)
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m));
#endif
    }
    shared_lock(Mutex& m_,defer_lock_t) NDNBOOST_NOEXCEPT:
    m(&m_),is_locked(false)
    {}
    shared_lock(Mutex& m_,try_to_lock_t):
    m(&m_),is_locked(false)
    {
      try_lock();
    }
#if defined NDNBOOST_THREAD_USES_DATETIME
    shared_lock(Mutex& m_,system_time const& target_time):
    m(&m_),is_locked(false)
    {
      timed_lock(target_time);
    }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    shared_lock(Mutex& mtx, const chrono::time_point<Clock, Duration>& t)
    : m(&mtx), is_locked(mtx.try_lock_shared_until(t))
    {
    }
    template <class Rep, class Period>
    shared_lock(Mutex& mtx, const chrono::duration<Rep, Period>& d)
    : m(&mtx), is_locked(mtx.try_lock_shared_for(d))
    {
    }
#endif

    shared_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) NDNBOOST_NOEXCEPT:
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION shared_lock(NDNBOOST_THREAD_RV_REF_BEG unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other):
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      if(is_locked)
      {
        m->unlock_and_lock_shared();
      }
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION shared_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other):
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      if(is_locked)
      {
        m->unlock_upgrade_and_lock_shared();
      }
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    //std-2104 unique_lock move-assignment should not be noexcept
    shared_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG shared_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) //NDNBOOST_NOEXCEPT
    {
      shared_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }
#ifndef NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
    shared_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other)
    {
      shared_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }

    shared_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other)
    {
      shared_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }
#endif

    void swap(shared_lock& other) NDNBOOST_NOEXCEPT
    {
      std::swap(m,other.m);
      std::swap(is_locked,other.is_locked);
    }

    Mutex* mutex() const NDNBOOST_NOEXCEPT
    {
      return m;
    }

    Mutex* release() NDNBOOST_NOEXCEPT
    {
      Mutex* const res=m;
      m=0;
      is_locked=false;
      return res;
    }

    ~shared_lock()
    {
      if(owns_lock())
      {
        m->unlock_shared();
      }
    }
    void lock()
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      m->lock_shared();
      is_locked=true;
    }
    bool try_lock()
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->try_lock_shared();
      return is_locked;
    }
#if defined NDNBOOST_THREAD_USES_DATETIME
    bool timed_lock(ndnboost::system_time const& target_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->timed_lock_shared(target_time);
      return is_locked;
    }
    template<typename Duration>
    bool timed_lock(Duration const& target_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->timed_lock_shared(target_time);
      return is_locked;
    }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Rep, class Period>
    bool try_lock_for(const chrono::duration<Rep, Period>& rel_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->try_lock_shared_for(rel_time);
      return is_locked;
    }
    template <class Clock, class Duration>
    bool try_lock_until(const chrono::time_point<Clock, Duration>& abs_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->try_lock_shared_until(abs_time);
      return is_locked;
    }
#endif
    void unlock()
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(!owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock doesn't own the mutex"));
      }
      m->unlock_shared();
      is_locked=false;
    }

#if defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    typedef void (shared_lock<Mutex>::*bool_type)();
    operator bool_type() const NDNBOOST_NOEXCEPT
    {
      return is_locked?&shared_lock::lock:0;
    }
    bool operator!() const NDNBOOST_NOEXCEPT
    {
      return !owns_lock();
    }
#else
    explicit operator bool() const NDNBOOST_NOEXCEPT
    {
      return owns_lock();
    }
#endif
    bool owns_lock() const NDNBOOST_NOEXCEPT
    {
      return is_locked;
    }

  };

  NDNBOOST_THREAD_DCL_MOVABLE_BEG(Mutex) shared_lock<Mutex> NDNBOOST_THREAD_DCL_MOVABLE_END

  template<typename Mutex>
  void swap(shared_lock<Mutex>& lhs,shared_lock<Mutex>& rhs) NDNBOOST_NOEXCEPT
  {
    lhs.swap(rhs);
  }

  template <typename Mutex>
  class upgrade_lock
  {
  protected:
    Mutex* m;
    bool is_locked;

  public:
    typedef Mutex mutex_type;
    NDNBOOST_THREAD_MOVABLE_ONLY( upgrade_lock)

    upgrade_lock()NDNBOOST_NOEXCEPT:
    m(0),is_locked(false)
    {}

    explicit upgrade_lock(Mutex& m_) :
      m(&m_), is_locked(false)
    {
      lock();
    }
    upgrade_lock(Mutex& m_, adopt_lock_t) :
      m(&m_), is_locked(true)
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m));
#endif
    }
    upgrade_lock(Mutex& m_, defer_lock_t)NDNBOOST_NOEXCEPT:
    m(&m_),is_locked(false)
    {}
    upgrade_lock(Mutex& m_, try_to_lock_t) :
      m(&m_), is_locked(false)
    {
      try_lock();
    }

#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    upgrade_lock(Mutex& mtx, const chrono::time_point<Clock, Duration>& t)
    : m(&mtx), is_locked(mtx.try_lock_upgrade_until(t))
    {
    }
    template <class Rep, class Period>
    upgrade_lock(Mutex& mtx, const chrono::duration<Rep, Period>& d)
    : m(&mtx), is_locked(mtx.try_lock_upgrade_for(d))
    {
    }
#endif

    upgrade_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) NDNBOOST_NOEXCEPT:
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    NDNBOOST_THREAD_EXPLICIT_LOCK_CONVERSION upgrade_lock(NDNBOOST_THREAD_RV_REF_BEG unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other):
    m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
    {
      if(is_locked)
      {
        m->unlock_and_lock_upgrade();
      }
      NDNBOOST_THREAD_RV(other).is_locked=false;
      NDNBOOST_THREAD_RV(other).m=0;
    }

    //std-2104 unique_lock move-assignment should not be noexcept
    upgrade_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) //NDNBOOST_NOEXCEPT
    {
      upgrade_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }

#ifndef NDNBOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
    upgrade_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other)
    {
      upgrade_lock temp(::ndnboost::move(other));
      swap(temp);
      return *this;
    }
#endif

#ifdef NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
    // Conversion from shared locking
    upgrade_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl, try_to_lock_t)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock_upgrade())
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }

#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Clock, class Duration>
    upgrade_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl,
        const chrono::time_point<Clock, Duration>& abs_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock_upgrade_until(abs_time))
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }

    template <class Rep, class Period>
    upgrade_lock(NDNBOOST_THREAD_RV_REF_BEG shared_lock<mutex_type> NDNBOOST_THREAD_RV_REF_END sl,
        const chrono::duration<Rep, Period>& rel_time)
    : m(0),is_locked(false)
    {
      if (NDNBOOST_THREAD_RV(sl).owns_lock())
      {
        if (NDNBOOST_THREAD_RV(sl).mutex()->try_unlock_shared_and_lock_upgrade_for(rel_time))
        {
          m = NDNBOOST_THREAD_RV(sl).release();
          is_locked = true;
        }
      }
      else
      {
        m = NDNBOOST_THREAD_RV(sl).release();
      }
    }
#endif // NDNBOOST_THREAD_USES_CHRONO
#endif // NDNBOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
    void swap(upgrade_lock& other)NDNBOOST_NOEXCEPT
    {
      std::swap(m,other.m);
      std::swap(is_locked,other.is_locked);
    }
    Mutex* mutex() const NDNBOOST_NOEXCEPT
    {
      return m;
    }

    Mutex* release()NDNBOOST_NOEXCEPT
    {
      Mutex* const res=m;
      m=0;
      is_locked=false;
      return res;
    }
    ~upgrade_lock()
    {
      if (owns_lock())
      {
        m->unlock_upgrade();
      }
    }
    void lock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if (owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost upgrade_lock owns already the mutex"));
      }
      m->lock_upgrade();
      is_locked = true;
    }
    bool try_lock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if (owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost upgrade_lock owns already the mutex"));
      }
      is_locked = m->try_lock_upgrade();
      return is_locked;
    }
    void unlock()
    {
      if (m == 0)
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if (!owns_lock())
      {
        ndnboost::throw_exception(
            ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost upgrade_lock doesn't own the mutex"));
      }
      m->unlock_upgrade();
      is_locked = false;
    }
#ifdef NDNBOOST_THREAD_USES_CHRONO
    template <class Rep, class Period>
    bool try_lock_for(const chrono::duration<Rep, Period>& rel_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->try_lock_upgrade_for(rel_time);
      return is_locked;
    }
    template <class Clock, class Duration>
    bool try_lock_until(const chrono::time_point<Clock, Duration>& abs_time)
    {
      if(m==0)
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::operation_not_permitted), "boost shared_lock has no mutex"));
      }
      if(owns_lock())
      {
        ndnboost::throw_exception(ndnboost::lock_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost shared_lock owns already the mutex"));
      }
      is_locked=m->try_lock_upgrade_until(abs_time);
      return is_locked;
    }
#endif
#if defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    typedef void (upgrade_lock::*bool_type)();
    operator bool_type() const NDNBOOST_NOEXCEPT
    {
      return is_locked?&upgrade_lock::lock:0;
    }
    bool operator!() const NDNBOOST_NOEXCEPT
    {
      return !owns_lock();
    }
#else
    explicit operator bool() const NDNBOOST_NOEXCEPT
    {
      return owns_lock();
    }
#endif
    bool owns_lock() const NDNBOOST_NOEXCEPT
    {
      return is_locked;
    }
    friend class shared_lock<Mutex> ;
    friend class unique_lock<Mutex> ;
  };

  template<typename Mutex>
  void swap(upgrade_lock<Mutex>& lhs, upgrade_lock<Mutex>& rhs)
  NDNBOOST_NOEXCEPT
  {
    lhs.swap(rhs);
  }

  NDNBOOST_THREAD_DCL_MOVABLE_BEG(Mutex) upgrade_lock<Mutex> NDNBOOST_THREAD_DCL_MOVABLE_END

  template<typename Mutex>
  unique_lock<Mutex>::unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other):
  m(NDNBOOST_THREAD_RV(other).m),is_locked(NDNBOOST_THREAD_RV(other).is_locked)
  {
    if(is_locked)
    {
      m->unlock_upgrade_and_lock();
    }
    NDNBOOST_THREAD_RV(other).release();
  }

  template <class Mutex>
  class upgrade_to_unique_lock
  {
  private:
    upgrade_lock<Mutex>* source;
    unique_lock<Mutex> exclusive;

  public:
    typedef Mutex mutex_type;
    NDNBOOST_THREAD_MOVABLE_ONLY( upgrade_to_unique_lock)

    explicit upgrade_to_unique_lock(upgrade_lock<Mutex>& m_) :
      source(&m_), exclusive(::ndnboost::move(*source))
    {
    }
    ~upgrade_to_unique_lock()
    {
      if (source)
      {
        *source = NDNBOOST_THREAD_MAKE_RV_REF(upgrade_lock<Mutex> (::ndnboost::move(exclusive)));
      }
    }

    upgrade_to_unique_lock(NDNBOOST_THREAD_RV_REF_BEG upgrade_to_unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) NDNBOOST_NOEXCEPT:
    source(NDNBOOST_THREAD_RV(other).source),exclusive(::ndnboost::move(NDNBOOST_THREAD_RV(other).exclusive))
    {
      NDNBOOST_THREAD_RV(other).source=0;
    }

    //std-2104 unique_lock move-assignment should not be noexcept
    upgrade_to_unique_lock& operator=(NDNBOOST_THREAD_RV_REF_BEG upgrade_to_unique_lock<Mutex> NDNBOOST_THREAD_RV_REF_END other) //NDNBOOST_NOEXCEPT
    {
      upgrade_to_unique_lock temp(other);
      swap(temp);
      return *this;
    }

    void swap(upgrade_to_unique_lock& other)NDNBOOST_NOEXCEPT
    {
      std::swap(source,other.source);
      exclusive.swap(other.exclusive);
    }

#if defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    typedef void (upgrade_to_unique_lock::*bool_type)(upgrade_to_unique_lock&);
    operator bool_type() const NDNBOOST_NOEXCEPT
    {
      return exclusive.owns_lock()?&upgrade_to_unique_lock::swap:0;
    }
    bool operator!() const NDNBOOST_NOEXCEPT
    {
      return !owns_lock();
    }
#else
    explicit operator bool() const NDNBOOST_NOEXCEPT
    {
      return owns_lock();
    }
#endif

    bool owns_lock() const NDNBOOST_NOEXCEPT
    {
      return exclusive.owns_lock();
    }
    Mutex* mutex() const NDNBOOST_NOEXCEPT
    {
      return exclusive.mutex();
    }
  };

NDNBOOST_THREAD_DCL_MOVABLE_BEG(Mutex) upgrade_to_unique_lock<Mutex> NDNBOOST_THREAD_DCL_MOVABLE_END

namespace detail
{
  template<typename Mutex>
  class try_lock_wrapper:
private unique_lock<Mutex>
  {
    typedef unique_lock<Mutex> base;
  public:
    NDNBOOST_THREAD_MOVABLE_ONLY(try_lock_wrapper)

    try_lock_wrapper()
    {}

    explicit try_lock_wrapper(Mutex& m):
    base(m,try_to_lock)
    {}

    try_lock_wrapper(Mutex& m_,adopt_lock_t):
    base(m_,adopt_lock)
    {
#if ! defined NDNBOOST_THREAD_PROVIDES_NESTED_LOCKS
      NDNBOOST_ASSERT(is_locked_by_this_thread(m_));
#endif
    }
    try_lock_wrapper(Mutex& m_,defer_lock_t):
    base(m_,defer_lock)
    {}
    try_lock_wrapper(Mutex& m_,try_to_lock_t):
    base(m_,try_to_lock)
    {}
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    try_lock_wrapper(NDNBOOST_THREAD_RV_REF(try_lock_wrapper) other):
    base(::ndnboost::move(other))
    {}

#elif defined NDNBOOST_THREAD_USES_MOVE
    try_lock_wrapper(NDNBOOST_THREAD_RV_REF(try_lock_wrapper) other):
    base(::ndnboost::move(static_cast<base&>(other)))
    {}

#else
    try_lock_wrapper(NDNBOOST_THREAD_RV_REF(try_lock_wrapper) other):
    base(NDNBOOST_THREAD_RV_REF(base)(*other))
    {}
#endif
    try_lock_wrapper& operator=(NDNBOOST_THREAD_RV_REF_BEG try_lock_wrapper<Mutex> NDNBOOST_THREAD_RV_REF_END other)
    {
      try_lock_wrapper temp(other);
      swap(temp);
      return *this;
    }
    void swap(try_lock_wrapper& other)
    {
      base::swap(other);
    }
    void lock()
    {
      base::lock();
    }
    bool try_lock()
    {
      return base::try_lock();
    }
    void unlock()
    {
      base::unlock();
    }
    bool owns_lock() const
    {
      return base::owns_lock();
    }
    Mutex* mutex() const NDNBOOST_NOEXCEPT
    {
      return base::mutex();
    }
    Mutex* release()
    {
      return base::release();
    }

#if defined(NDNBOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    typedef typename base::bool_type bool_type;
    operator bool_type() const
    {
      return base::operator bool_type();
    }
    bool operator!() const
    {
      return !this->owns_lock();
    }
#else
    explicit operator bool() const
    {
      return owns_lock();
    }
#endif
  };

  template<typename Mutex>
  void swap(try_lock_wrapper<Mutex>& lhs,try_lock_wrapper<Mutex>& rhs)
  {
    lhs.swap(rhs);
  }
}
}
#include <ndnboost/config/abi_suffix.hpp>

#endif
