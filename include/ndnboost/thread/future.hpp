//  (C) Copyright 2008-10 Anthony Williams
//  (C) Copyright 2011-2014 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_THREAD_FUTURE_HPP
#define NDNBOOST_THREAD_FUTURE_HPP

#include <ndnboost/thread/detail/config.hpp>

// ndnboost::thread::future requires exception handling
// due to ndnboost::exception::exception_ptr dependency

#ifndef NDNBOOST_NO_EXCEPTIONS

#include <ndnboost/core/scoped_enum.hpp>
#include <stdexcept>
#include <ndnboost/thread/exceptional_ptr.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/invoker.hpp>
#include <ndnboost/thread/thread_time.hpp>
#include <ndnboost/thread/mutex.hpp>
#include <ndnboost/thread/condition_variable.hpp>
#include <ndnboost/thread/lock_algorithms.hpp>
#include <ndnboost/thread/lock_types.hpp>
#include <ndnboost/exception_ptr.hpp>
#include <ndnboost/shared_ptr.hpp>
#include <ndnboost/scoped_ptr.hpp>
#include <ndnboost/type_traits/is_fundamental.hpp>
#include <ndnboost/thread/detail/is_convertible.hpp>
#include <ndnboost/type_traits/decay.hpp>
#include <ndnboost/type_traits/is_void.hpp>
#include <ndnboost/type_traits/conditional.hpp>
#include <ndnboost/config.hpp>
#include <ndnboost/throw_exception.hpp>
#include <algorithm>
#include <ndnboost/function.hpp>
#include <ndnboost/bind.hpp>
#include <ndnboost/core/ref.hpp>
#include <ndnboost/scoped_array.hpp>
#include <ndnboost/enable_shared_from_this.hpp>
#include <ndnboost/core/enable_if.hpp>

#include <list>
#include <ndnboost/next_prior.hpp>
#include <vector>

#include <ndnboost/thread/future_error_code.hpp>
#ifdef NDNBOOST_THREAD_USES_CHRONO
#include <ndnboost/chrono/system_clocks.hpp>
#endif

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#include <ndnboost/thread/detail/memory.hpp>
#include <ndnboost/container/scoped_allocator.hpp>
#if ! defined  NDNBOOST_NO_CXX11_ALLOCATOR
#include <memory>
#endif
#endif

#include <ndnboost/utility/result_of.hpp>
#include <ndnboost/thread/thread_only.hpp>

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
#include <ndnboost/thread/csbl/tuple.hpp>
#include <ndnboost/thread/csbl/vector.hpp>
#endif

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE
#define NDNBOOST_THREAD_FUTURE future
#else
#define NDNBOOST_THREAD_FUTURE unique_future
#endif

namespace ndnboost
{

  //enum class launch
  NDNBOOST_SCOPED_ENUM_DECLARE_BEGIN(launch)
  {
      none = 0,
      async = 1,
      deferred = 2,
#ifdef NDNBOOST_THREAD_PROVIDES_EXECUTORS
      executor = 4,
#endif
      any = async | deferred
  }
  NDNBOOST_SCOPED_ENUM_DECLARE_END(launch)

  //enum class future_status
  NDNBOOST_SCOPED_ENUM_DECLARE_BEGIN(future_status)
  {
      ready,
      timeout,
      deferred
  }
  NDNBOOST_SCOPED_ENUM_DECLARE_END(future_status)

  class NDNBOOST_SYMBOL_VISIBLE future_error
      : public std::logic_error
  {
      system::error_code ec_;
  public:
      future_error(system::error_code ec)
      : logic_error(ec.message()),
        ec_(ec)
      {
      }

      const system::error_code& code() const NDNBOOST_NOEXCEPT
      {
        return ec_;
      }
  };

    class NDNBOOST_SYMBOL_VISIBLE future_uninitialized:
        public future_error
    {
    public:
        future_uninitialized() :
          future_error(system::make_error_code(future_errc::no_state))
        {}
    };
    class NDNBOOST_SYMBOL_VISIBLE broken_promise:
        public future_error
    {
    public:
        broken_promise():
          future_error(system::make_error_code(future_errc::broken_promise))
        {}
    };
    class NDNBOOST_SYMBOL_VISIBLE future_already_retrieved:
        public future_error
    {
    public:
        future_already_retrieved():
          future_error(system::make_error_code(future_errc::future_already_retrieved))
        {}
    };
    class NDNBOOST_SYMBOL_VISIBLE promise_already_satisfied:
        public future_error
    {
    public:
        promise_already_satisfied():
          future_error(system::make_error_code(future_errc::promise_already_satisfied))
        {}
    };

    class NDNBOOST_SYMBOL_VISIBLE task_already_started:
        public future_error
    {
    public:
        task_already_started():
        future_error(system::make_error_code(future_errc::promise_already_satisfied))
        {}
    };

    class NDNBOOST_SYMBOL_VISIBLE task_moved:
        public future_error
    {
    public:
        task_moved():
          future_error(system::make_error_code(future_errc::no_state))
        {}
    };

    class promise_moved:
        public future_error
    {
    public:
          promise_moved():
          future_error(system::make_error_code(future_errc::no_state))
        {}
    };

    namespace future_state
    {
        enum state { uninitialized, waiting, ready, moved, deferred };
    }

    namespace detail
    {
        struct relocker
        {
            ndnboost::unique_lock<ndnboost::mutex>& lock_;
            bool  unlocked_;

            relocker(ndnboost::unique_lock<ndnboost::mutex>& lk):
                lock_(lk)
            {
                lock_.unlock();
                unlocked_=true;
            }
            ~relocker()
            {
              if (unlocked_) {
                lock_.lock();
              }
            }
            void lock() {
              if (unlocked_) {
                lock_.lock();
                unlocked_=false;
              }
            }
        private:
            relocker& operator=(relocker const&);
        };

        struct shared_state_base : enable_shared_from_this<shared_state_base>
        {
            typedef std::list<ndnboost::condition_variable_any*> waiter_list;
            // This type should be only included conditionally if interruptions are allowed, but is included to maintain the same layout.
            typedef shared_ptr<shared_state_base> continuation_ptr_type;

            ndnboost::exception_ptr exception;
            bool done;
            bool is_deferred_;
            launch policy_;
            bool is_constructed;
            mutable ndnboost::mutex mutex;
            ndnboost::condition_variable waiters;
            waiter_list external_waiters;
            ndnboost::function<void()> callback;
            // This declaration should be only included conditionally if interruptions are allowed, but is included to maintain the same layout.
            bool thread_was_interrupted;
            // This declaration should be only included conditionally, but is included to maintain the same layout.
            continuation_ptr_type continuation_ptr;

            // This declaration should be only included conditionally, but is included to maintain the same layout.
            virtual void launch_continuation(ndnboost::unique_lock<ndnboost::mutex>&)
            {
            }

            shared_state_base():
                done(false),
                is_deferred_(false),
                policy_(launch::none),
                is_constructed(false),
                thread_was_interrupted(false),
                continuation_ptr()
            {}
            virtual ~shared_state_base()
            {}

            void set_deferred()
            {
              is_deferred_ = true;
              policy_ = launch::deferred;
            }
            void set_async()
            {
              is_deferred_ = false;
              policy_ = launch::async;
            }
#ifdef NDNBOOST_THREAD_PROVIDES_EXECUTORS
            void set_executor()
            {
              is_deferred_ = false;
              policy_ = launch::executor;
            }
#endif
            waiter_list::iterator register_external_waiter(ndnboost::condition_variable_any& cv)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                do_callback(lock);
                return external_waiters.insert(external_waiters.end(),&cv);
            }

            void remove_external_waiter(waiter_list::iterator it)
            {
                ndnboost::lock_guard<ndnboost::mutex> lock(mutex);
                external_waiters.erase(it);
            }

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            void do_continuation(ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                if (continuation_ptr) {
                  continuation_ptr_type this_continuation_ptr = continuation_ptr;
                  continuation_ptr.reset();
                  this_continuation_ptr->launch_continuation(lock);
                  //if (! lock.owns_lock())
                  //  lock.lock();
                }
            }
#else
            void do_continuation(ndnboost::unique_lock<ndnboost::mutex>&)
            {
            }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            void set_continuation_ptr(continuation_ptr_type continuation, ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
              continuation_ptr= continuation;
              if (done) {
                do_continuation(lock);
              }
            }
#endif
            void mark_finished_internal(ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                done=true;
                waiters.notify_all();
                for(waiter_list::const_iterator it=external_waiters.begin(),
                        end=external_waiters.end();it!=end;++it)
                {
                    (*it)->notify_all();
                }
                do_continuation(lock);
            }
            void make_ready()
            {
              ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
              mark_finished_internal(lock);
            }

            void do_callback(ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                if(callback && !done)
                {
                    ndnboost::function<void()> local_callback=callback;
                    relocker relock(lock);
                    local_callback();
                }
            }

            void wait_internal(ndnboost::unique_lock<ndnboost::mutex> &lk, bool rethrow=true)
            {
              do_callback(lk);
              //if (!done) // fixme why this doesn't work?
              {
                if (is_deferred_)
                {
                  is_deferred_=false;
                  execute(lk);
                  //lk.unlock();
                }
                else
                {
                  while(!done)
                  {
                      waiters.wait(lk);
                  }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                  if(rethrow && thread_was_interrupted)
                  {
                      throw ndnboost::thread_interrupted();
                  }
#endif
                  if(rethrow && exception)
                  {
                      ndnboost::rethrow_exception(exception);
                  }
                }
              }
            }

            virtual void wait(bool rethrow=true)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                wait_internal(lock, rethrow);
            }

#if defined NDNBOOST_THREAD_USES_DATETIME
            bool timed_wait_until(ndnboost::system_time const& target_time)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                if (is_deferred_)
                    return false;

                do_callback(lock);
                while(!done)
                {
                    bool const success=waiters.timed_wait(lock,target_time);
                    if(!success && !done)
                    {
                        return false;
                    }
                }
                return true;
            }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO

            template <class Clock, class Duration>
            future_status
            wait_until(const chrono::time_point<Clock, Duration>& abs_time)
            {
              ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
              if (is_deferred_)
                  return future_status::deferred;
              do_callback(lock);
              while(!done)
              {
                  cv_status const st=waiters.wait_until(lock,abs_time);
                  if(st==cv_status::timeout && !done)
                  {
                    return future_status::timeout;
                  }
              }
              return future_status::ready;
            }
#endif
            void mark_exceptional_finish_internal(ndnboost::exception_ptr const& e, ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                exception=e;
                mark_finished_internal(lock);
            }

            void mark_exceptional_finish()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                mark_exceptional_finish_internal(ndnboost::current_exception(), lock);
            }

#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            void mark_interrupted_finish()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                thread_was_interrupted=true;
                mark_finished_internal(lock);
            }

            void set_interrupted_ndnboostat_thread_exit()
            {
              unique_lock<ndnboost::mutex> lk(mutex);
              thread_was_interrupted=true;
              if (has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());
            }
#endif

            void set_exception_ndnboostat_thread_exit(exception_ptr e)
            {
              unique_lock<ndnboost::mutex> lk(mutex);
              if (has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              exception=e;
              this->is_constructed = true;
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());

            }

            bool has_value() const
            {
                ndnboost::lock_guard<ndnboost::mutex> lock(mutex);
                return done && !(exception
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                );
            }

            bool has_value(unique_lock<ndnboost::mutex>& )  const
            {
                return done && !(exception
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                );
            }

            bool has_exception()  const
            {
                ndnboost::lock_guard<ndnboost::mutex> lock(mutex);
                return done && (exception
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                    );
            }

            bool has_exception(unique_lock<ndnboost::mutex>&) const
            {
                return done && (exception
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                    );
            }

            bool is_deferred(ndnboost::lock_guard<ndnboost::mutex>&)  const {
                return is_deferred_;
            }

            launch launch_policy(ndnboost::unique_lock<ndnboost::mutex>&) const
            {
                return policy_;
            }

            future_state::state get_state() const
            {
                ndnboost::lock_guard<ndnboost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }

            exception_ptr get_exception_ptr()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                return get_exception_ptr(lock);
            }
            exception_ptr get_exception_ptr(ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                wait_internal(lock, false);
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                if(thread_was_interrupted)
                {
                    return copy_exception(ndnboost::thread_interrupted());
                }
#endif
                return exception;
            }

            template<typename F,typename U>
            void set_wait_callback(F f,U* u)
            {
                ndnboost::lock_guard<ndnboost::mutex> lock(mutex);
                callback=ndnboost::bind(f,ndnboost::ref(*u));
            }

            virtual void execute(ndnboost::unique_lock<ndnboost::mutex>&) {}

        private:
            shared_state_base(shared_state_base const&);
            shared_state_base& operator=(shared_state_base const&);
        };

        template<typename T>
        struct future_traits
        {
          typedef ndnboost::scoped_ptr<T> storage_type;
          struct dummy;
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
          typedef T const& source_reference_type;
          //typedef typename conditional<ndnboost::is_fundamental<T>::value,dummy&,NDNBOOST_THREAD_RV_REF(T)>::type rvalue_source_type;
          typedef NDNBOOST_THREAD_RV_REF(T) rvalue_source_type;
          //typedef typename conditional<ndnboost::is_fundamental<T>::value,T,NDNBOOST_THREAD_RV_REF(T)>::type move_dest_type;
          typedef T move_dest_type;
#elif defined NDNBOOST_THREAD_USES_MOVE
          typedef typename conditional<ndnboost::is_fundamental<T>::value,T,T&>::type source_reference_type;
          //typedef typename conditional<ndnboost::is_fundamental<T>::value,T,NDNBOOST_THREAD_RV_REF(T)>::type rvalue_source_type;
          //typedef typename conditional<ndnboost::enable_move_utility_emulation<T>::value,NDNBOOST_THREAD_RV_REF(T),T>::type move_dest_type;
          typedef NDNBOOST_THREAD_RV_REF(T) rvalue_source_type;
          typedef T move_dest_type;
#else
          typedef T& source_reference_type;
          typedef typename conditional<ndnboost::thread_detail::is_convertible<T&,NDNBOOST_THREAD_RV_REF(T) >::value, NDNBOOST_THREAD_RV_REF(T),T const&>::type rvalue_source_type;
          typedef typename conditional<ndnboost::thread_detail::is_convertible<T&,NDNBOOST_THREAD_RV_REF(T) >::value, NDNBOOST_THREAD_RV_REF(T),T>::type move_dest_type;
#endif


            typedef const T& shared_future_get_result_type;

            static void init(storage_type& storage,source_reference_type t)
            {
                storage.reset(new T(t));
            }

            static void init(storage_type& storage,rvalue_source_type t)
            {
#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
              storage.reset(new T(ndnboost::forward<T>(t)));
#else
              storage.reset(new T(static_cast<rvalue_source_type>(t)));
#endif
            }

            static void cleanup(storage_type& storage)
            {
                storage.reset();
            }
        };

        template<typename T>
        struct future_traits<T&>
        {
            typedef T* storage_type;
            typedef T& source_reference_type;
            //struct rvalue_source_type
            //{};
            typedef T& move_dest_type;
            typedef T& shared_future_get_result_type;

            static void init(storage_type& storage,T& t)
            {
                storage=&t;
            }

            static void cleanup(storage_type& storage)
            {
                storage=0;
            }
        };

        template<>
        struct future_traits<void>
        {
            typedef bool storage_type;
            typedef void move_dest_type;
            typedef void shared_future_get_result_type;

            static void init(storage_type& storage)
            {
                storage=true;
            }

            static void cleanup(storage_type& storage)
            {
                storage=false;
            }

        };

        // Used to create stand-alone futures
        template<typename T>
        struct shared_state:
            detail::shared_state_base
        {
            typedef typename future_traits<T>::storage_type storage_type;
            typedef typename future_traits<T>::source_reference_type source_reference_type;
            typedef typename future_traits<T>::rvalue_source_type rvalue_source_type;
            typedef typename future_traits<T>::move_dest_type move_dest_type;
            typedef typename future_traits<T>::shared_future_get_result_type shared_future_get_result_type;

            storage_type result;

            shared_state():
                result(0)
            {}

            ~shared_state()
            {}

            void mark_finished_with_result_internal(source_reference_type result_, ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                future_traits<T>::init(result,result_);
                this->mark_finished_internal(lock);
            }

            void mark_finished_with_result_internal(rvalue_source_type result_, ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
                future_traits<T>::init(result,ndnboost::forward<T>(result_));
#else
                future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
#endif
                this->mark_finished_internal(lock);
            }

            void mark_finished_with_result(source_reference_type result_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                this->mark_finished_with_result_internal(result_, lock);
            }

            void mark_finished_with_result(rvalue_source_type result_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);

#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
                mark_finished_with_result_internal(ndnboost::forward<T>(result_), lock);
#else
                mark_finished_with_result_internal(static_cast<rvalue_source_type>(result_), lock);
#endif
            }

            virtual move_dest_type get()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                wait_internal(lock);
                return ndnboost::move(*result);
            }

            virtual shared_future_get_result_type get_sh()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                wait_internal(lock);
                return *result;
            }

            //void set_value_ndnboostat_thread_exit(const T & result_)
            void set_value_ndnboostat_thread_exit(source_reference_type result_)
            {
              unique_lock<ndnboost::mutex> lk(this->mutex);
              if (this->has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              //future_traits<T>::init(result,result_);
              result.reset(new T(result_));

              this->is_constructed = true;
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());
            }
            //void set_value_ndnboostat_thread_exit(NDNBOOST_THREAD_RV_REF(T) result_)
            void set_value_ndnboostat_thread_exit(rvalue_source_type result_)
            {
              unique_lock<ndnboost::mutex> lk(this->mutex);
              if (this->has_value(lk))
                  throw_exception(promise_already_satisfied());
              result.reset(new T(ndnboost::move(result_)));
              //future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
              this->is_constructed = true;
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());
            }


        private:
            shared_state(shared_state const&);
            shared_state& operator=(shared_state const&);
        };

        template<typename T>
        struct shared_state<T&>:
            detail::shared_state_base
        {
            typedef typename future_traits<T&>::storage_type storage_type;
            typedef typename future_traits<T&>::source_reference_type source_reference_type;
            typedef typename future_traits<T&>::move_dest_type move_dest_type;
            typedef typename future_traits<T&>::shared_future_get_result_type shared_future_get_result_type;

            T* result;

            shared_state():
                result(0)
            {}

            ~shared_state()
            {
            }

            void mark_finished_with_result_internal(source_reference_type result_, ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                //future_traits<T>::init(result,result_);
                result= &result_;
                mark_finished_internal(lock);
            }

            void mark_finished_with_result(source_reference_type result_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                mark_finished_with_result_internal(result_, lock);
            }

            virtual T& get()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                wait_internal(lock);
                return *result;
            }

            virtual T& get_sh()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                wait_internal(lock);
                return *result;
            }

            void set_value_ndnboostat_thread_exit(T& result_)
            {
              unique_lock<ndnboost::mutex> lk(this->mutex);
              if (this->has_value(lk))
                  throw_exception(promise_already_satisfied());
              //future_traits<T>::init(result,result_);
              result= &result_;
              this->is_constructed = true;
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());
            }

        private:
            shared_state(shared_state const&);
            shared_state& operator=(shared_state const&);
        };

        template<>
        struct shared_state<void>:
            detail::shared_state_base
        {
          typedef void shared_future_get_result_type;

            shared_state()
            {}

            void mark_finished_with_result_internal(ndnboost::unique_lock<ndnboost::mutex>& lock)
            {
                mark_finished_internal(lock);
            }

            void mark_finished_with_result()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                mark_finished_with_result_internal(lock);
            }

            virtual void get()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                this->wait_internal(lock);
            }

            virtual void get_sh()
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
                this->wait_internal(lock);
            }

            void set_value_ndnboostat_thread_exit()
            {
              unique_lock<ndnboost::mutex> lk(this->mutex);
              if (this->has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              this->is_constructed = true;
              detail::make_ready_ndnboostat_thread_exit(shared_from_this());
            }
        private:
            shared_state(shared_state const&);
            shared_state& operator=(shared_state const&);
        };

        /////////////////////////
        /// future_async_shared_state_base
        /////////////////////////
        template<typename Rp>
        struct future_async_shared_state_base: shared_state<Rp>
        {
          typedef shared_state<Rp> base_type;
        protected:
          ndnboost::thread thr_;
          void join()
          {
              if (thr_.joinable()) thr_.join();
          }
        public:
          future_async_shared_state_base()
          {
            this->set_async();
          }
          explicit future_async_shared_state_base(NDNBOOST_THREAD_RV_REF(ndnboost::thread) th) :
            thr_(ndnboost::move(th))
          {
            this->set_async();
          }

          ~future_async_shared_state_base()
          {
            join();
          }

          virtual void wait(bool rethrow)
          {
              join();
              this->base_type::wait(rethrow);
          }
        };

        /////////////////////////
        /// future_async_shared_state
        /////////////////////////
        template<typename Rp, typename Fp>
        struct future_async_shared_state: future_async_shared_state_base<Rp>
        {
          typedef future_async_shared_state_base<Rp> base_type;

        public:
          explicit future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f) :
          base_type(thread(&future_async_shared_state::run, this, ndnboost::forward<Fp>(f)))
          {
          }

          static void run(future_async_shared_state* that, NDNBOOST_THREAD_FWD_REF(Fp) f)
          {
            try
            {
              that->mark_finished_with_result(f());
            }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            catch(thread_interrupted& )
            {
              that->mark_interrupted_finish();
            }
#endif
            catch(...)
            {
              that->mark_exceptional_finish();
            }
          }
        };

        template<typename Fp>
        struct future_async_shared_state<void, Fp>: public future_async_shared_state_base<void>
        {
          typedef future_async_shared_state_base<void> base_type;

        public:
          explicit future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f) :
          base_type(thread(&future_async_shared_state::run, this, ndnboost::forward<Fp>(f)))
          {
          }

          static void run(future_async_shared_state* that, NDNBOOST_THREAD_FWD_REF(Fp) f)
          {
            try
            {
              f();
              that->mark_finished_with_result();
            }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            catch(thread_interrupted& )
            {
              that->mark_interrupted_finish();
            }
#endif
            catch(...)
            {
              that->mark_exceptional_finish();
            }
          }
        };

        template<typename Rp, typename Fp>
        struct future_async_shared_state<Rp&, Fp>: future_async_shared_state_base<Rp&>
        {
          typedef future_async_shared_state_base<Rp&> base_type;

        public:
          explicit future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f) :
          base_type(thread(&future_async_shared_state::run, this, ndnboost::forward<Fp>(f)))
          {
          }

          static void run(future_async_shared_state* that, NDNBOOST_THREAD_FWD_REF(Fp) f)
          {
            try
            {
              that->mark_finished_with_result(f());
            }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
            catch(thread_interrupted& )
            {
              that->mark_interrupted_finish();
            }
#endif
            catch(...)
            {
              that->mark_exceptional_finish();
            }
          }
        };

        //////////////////////////
        /// future_deferred_shared_state
        //////////////////////////
        template<typename Rp, typename Fp>
        struct future_deferred_shared_state: shared_state<Rp>
        {
          typedef shared_state<Rp> base_type;
          Fp func_;

        public:
          explicit future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f)
          : func_(ndnboost::forward<Fp>(f))
          {
            this->set_deferred();
          }

          virtual void execute(ndnboost::unique_lock<ndnboost::mutex>& lck) {
            try
            {
              Fp local_fuct=ndnboost::move(func_);
              relocker relock(lck);
              Rp res = local_fuct();
              relock.lock();
              this->mark_finished_with_result_internal(ndnboost::move(res), lck);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lck);
            }
          }
        };
        template<typename Rp, typename Fp>
        struct future_deferred_shared_state<Rp&,Fp>: shared_state<Rp&>
        {
          typedef shared_state<Rp&> base_type;
          Fp func_;

        public:
          explicit future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f)
          : func_(ndnboost::forward<Fp>(f))
          {
            this->set_deferred();
          }

          virtual void execute(ndnboost::unique_lock<ndnboost::mutex>& lck) {
            try
            {
              this->mark_finished_with_result_internal(func_(), lck);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lck);
            }
          }
        };

        template<typename Fp>
        struct future_deferred_shared_state<void,Fp>: shared_state<void>
        {
          typedef shared_state<void> base_type;
          Fp func_;

        public:
          explicit future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f)
          : func_(ndnboost::forward<Fp>(f))
          {
            this->set_deferred();
          }

          virtual void execute(ndnboost::unique_lock<ndnboost::mutex>& lck) {
            try
            {
              Fp local_fuct=ndnboost::move(func_);
              relocker relock(lck);
              local_fuct();
              relock.lock();
              this->mark_finished_with_result_internal(lck);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lck);
            }
          }
        };

//        template<typename T, typename Allocator>
//        struct shared_state_alloc: public shared_state<T>
//        {
//          typedef shared_state<T> base;
//          Allocator alloc_;
//
//        public:
//          explicit shared_state_alloc(const Allocator& a)
//              : alloc_(a) {}
//
//        };
        class future_waiter
        {
            struct registered_waiter;
            typedef std::vector<int>::size_type count_type;

            struct registered_waiter
            {
                ndnboost::shared_ptr<detail::shared_state_base> future_;
                detail::shared_state_base::waiter_list::iterator wait_iterator;
                count_type index;

                registered_waiter(ndnboost::shared_ptr<detail::shared_state_base> const& a_future,
                                  detail::shared_state_base::waiter_list::iterator wait_iterator_,
                                  count_type index_):
                    future_(a_future),wait_iterator(wait_iterator_),index(index_)
                {}
            };

            struct all_futures_lock
            {
#ifdef _MANAGED
                   typedef std::ptrdiff_t count_type_portable;
#else
                   typedef count_type count_type_portable;
#endif
                   count_type_portable count;
                   ndnboost::scoped_array<ndnboost::unique_lock<ndnboost::mutex> > locks;

                all_futures_lock(std::vector<registered_waiter>& futures):
                    count(futures.size()),locks(new ndnboost::unique_lock<ndnboost::mutex>[count])
                {
                    for(count_type_portable i=0;i<count;++i)
                    {
                        locks[i]=NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::unique_lock<ndnboost::mutex>(futures[i].future_->mutex));
                    }
                }

                void lock()
                {
                    ndnboost::lock(locks.get(),locks.get()+count);
                }

                void unlock()
                {
                    for(count_type_portable i=0;i<count;++i)
                    {
                        locks[i].unlock();
                    }
                }
            };

            ndnboost::condition_variable_any cv;
            std::vector<registered_waiter> futures;
            count_type future_count;

        public:
            future_waiter():
                future_count(0)
            {}

            template<typename F>
            void add(F& f)
            {
                if(f.future_)
                {
                  registered_waiter waiter(f.future_,f.future_->register_external_waiter(cv),future_count);
                  try {
                      futures.push_back(waiter);
                  } catch(...) {
                    f.future_->remove_external_waiter(waiter.wait_iterator);
                    throw;
                  }
                }
                ++future_count;
            }

#ifndef NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
            template<typename F1, typename... Fs>
            void add(F1& f1, Fs&... fs)
            {
              add(f1); add(fs...);
            }
#endif

            count_type wait()
            {
                all_futures_lock lk(futures);
                for(;;)
                {
                    for(count_type i=0;i<futures.size();++i)
                    {
                        if(futures[i].future_->done)
                        {
                            return futures[i].index;
                        }
                    }
                    cv.wait(lk);
                }
            }

            ~future_waiter()
            {
                for(count_type i=0;i<futures.size();++i)
                {
                    futures[i].future_->remove_external_waiter(futures[i].wait_iterator);
                }
            }

        };

    }

    template <typename R>
    class NDNBOOST_THREAD_FUTURE;

    template <typename R>
    class shared_future;

    template<typename T>
    struct is_future_type
    {
        NDNBOOST_STATIC_CONSTANT(bool, value=false);
        typedef void type;
    };

    template<typename T>
    struct is_future_type<NDNBOOST_THREAD_FUTURE<T> >
    {
        NDNBOOST_STATIC_CONSTANT(bool, value=true);
        typedef T type;
    };

    template<typename T>
    struct is_future_type<shared_future<T> >
    {
        NDNBOOST_STATIC_CONSTANT(bool, value=true);
        typedef T type;
    };

    template<typename Iterator>
    typename ndnboost::disable_if<is_future_type<Iterator>,void>::type wait_for_all(Iterator begin,Iterator end)
    {
        for(Iterator current=begin;current!=end;++current)
        {
            current->wait();
        }
    }

#ifdef NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
    template<typename F1,typename F2>
    typename ndnboost::enable_if<is_future_type<F1>,void>::type wait_for_all(F1& f1,F2& f2)
    {
        f1.wait();
        f2.wait();
    }

    template<typename F1,typename F2,typename F3>
    void wait_for_all(F1& f1,F2& f2,F3& f3)
    {
        f1.wait();
        f2.wait();
        f3.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
        f5.wait();
    }
#else
    template<typename F1, typename... Fs>
    void wait_for_all(F1& f1, Fs&... fs)
    {
        bool dummy[] = { (f1.wait(), true), (fs.wait(), true)... };

        // prevent unused parameter warning
        (void) dummy;
    }
#endif // !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

    template<typename Iterator>
    typename ndnboost::disable_if<is_future_type<Iterator>,Iterator>::type wait_for_any(Iterator begin,Iterator end)
    {
        if(begin==end)
            return end;

        detail::future_waiter waiter;
        for(Iterator current=begin;current!=end;++current)
        {
            waiter.add(*current);
        }
        return ndnboost::next(begin,waiter.wait());
    }

#ifdef NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
    template<typename F1,typename F2>
    typename ndnboost::enable_if<is_future_type<F1>,unsigned>::type wait_for_any(F1& f1,F2& f2)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        waiter.add(f5);
        return waiter.wait();
    }
#else
    template<typename F1, typename... Fs>
    typename ndnboost::enable_if<is_future_type<F1>, unsigned>::type wait_for_any(F1& f1, Fs&... fs)
    {
      detail::future_waiter waiter;
      waiter.add(f1, fs...);
      return waiter.wait();
    }
#endif // !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

    template <typename R>
    class promise;

    template <typename R>
    class packaged_task;

    namespace detail
    {
      /// Common implementation for all the futures independently of the return type
      class base_future
      {
        //NDNBOOST_THREAD_MOVABLE(base_future)

      };
      /// Common implementation for future and shared_future.
      template <typename R>
      class basic_future : public base_future
      {
      protected:
      public:

        typedef ndnboost::shared_ptr<detail::shared_state<R> > future_ptr;
        static //NDNBOOST_CONSTEXPR
        future_ptr make_exceptional_future_ptr(exceptional_ptr const& ex) {
          promise<R> p;
          p.set_exception(ex.ptr_);
          return p.get_future().future_;
        }

        future_ptr future_;

        basic_future(future_ptr a_future):
          future_(a_future)
        {
        }
        // Copy construction from a shared_future
        explicit basic_future(const shared_future<R>&) NDNBOOST_NOEXCEPT;

      public:
        typedef future_state::state state;

        NDNBOOST_THREAD_MOVABLE(basic_future)
        basic_future(): future_() {}


        //NDNBOOST_CONSTEXPR
        basic_future(exceptional_ptr const& ex)
          : future_(make_exceptional_future_ptr(ex))
        {
        }

        ~basic_future() {}

        basic_future(NDNBOOST_THREAD_RV_REF(basic_future) other) NDNBOOST_NOEXCEPT:
        future_(NDNBOOST_THREAD_RV(other).future_)
        {
            NDNBOOST_THREAD_RV(other).future_.reset();
        }
        basic_future& operator=(NDNBOOST_THREAD_RV_REF(basic_future) other) NDNBOOST_NOEXCEPT
        {
            future_=NDNBOOST_THREAD_RV(other).future_;
            NDNBOOST_THREAD_RV(other).future_.reset();
            return *this;
        }
        void swap(basic_future& that) NDNBOOST_NOEXCEPT
        {
          future_.swap(that.future_);
        }
        // functions to check state, and wait for ready
        state get_state() const
        {
            if(!future_)
            {
                return future_state::uninitialized;
            }
            return future_->get_state();
        }

        bool is_ready() const
        {
            return get_state()==future_state::ready;
        }

        bool has_exception() const
        {
            return future_ && future_->has_exception();
        }

        bool has_value() const
        {
            return future_ && future_->has_value();
        }

        launch launch_policy(ndnboost::unique_lock<ndnboost::mutex>& lk) const
        {
            if ( future_ ) return future_->launch_policy(lk);
            else return launch(launch::none);
        }

        exception_ptr get_exception_ptr()
        {
            return future_
                ? future_->get_exception_ptr()
                : exception_ptr();
        }

        bool valid() const NDNBOOST_NOEXCEPT
        {
            return future_ != 0;
        }


        void wait() const
        {
            if(!future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            future_->wait(false);
        }

#if defined NDNBOOST_THREAD_USES_DATETIME
        template<typename Duration>
        bool timed_wait(Duration const& rel_time) const
        {
            return timed_wait_until(ndnboost::get_system_time()+rel_time);
        }

        bool timed_wait_until(ndnboost::system_time const& abs_time) const
        {
            if(!future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            return future_->timed_wait_until(abs_time);
        }
#endif
#ifdef NDNBOOST_THREAD_USES_CHRONO
        template <class Rep, class Period>
        future_status
        wait_for(const chrono::duration<Rep, Period>& rel_time) const
        {
          return wait_until(chrono::steady_clock::now() + rel_time);

        }
        template <class Clock, class Duration>
        future_status
        wait_until(const chrono::time_point<Clock, Duration>& abs_time) const
        {
          if(!future_)
          {
              ndnboost::throw_exception(future_uninitialized());
          }
          return future_->wait_until(abs_time);
        }
#endif

      };

    } // detail
    NDNBOOST_THREAD_DCL_MOVABLE_BEG(R) detail::basic_future<R> NDNBOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
#if (!defined _MSC_VER || _MSC_VER >= 1400) // _MSC_VER == 1400 on MSVC 2005
        template <class Rp, class Fp>
        NDNBOOST_THREAD_FUTURE<Rp>
        make_future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);

        template <class Rp, class Fp>
        NDNBOOST_THREAD_FUTURE<Rp>
        make_future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);
#endif // #if (!defined _MSC_VER || _MSC_VER >= 1400)
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
        template<typename F, typename Rp, typename Fp>
        struct future_deferred_continuation_shared_state;
        template<typename F, typename Rp, typename Fp>
        struct future_async_continuation_shared_state;

        template <class F, class Rp, class Fp>
        NDNBOOST_THREAD_FUTURE<Rp>
        make_future_async_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);

        template <class F, class Rp, class Fp>
        NDNBOOST_THREAD_FUTURE<Rp>
        make_future_deferred_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);
#endif
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
        template<typename F, typename Rp>
        struct future_unwrap_shared_state;
        template <class F, class Rp>
        inline NDNBOOST_THREAD_FUTURE<Rp>
        make_future_unwrap_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f);
#endif
    }

    template <typename R>
    class NDNBOOST_THREAD_FUTURE : public detail::basic_future<R>
    {
    private:
        typedef detail::basic_future<R> base_type;
        typedef typename base_type::future_ptr future_ptr;

        friend class shared_future<R>;
        friend class promise<R>;
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
        template <typename, typename, typename>
        friend struct detail::future_async_continuation_shared_state;
        template <typename, typename, typename>
        friend struct detail::future_deferred_continuation_shared_state;

        template <class F, class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_async_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);

        template <class F, class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_deferred_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);
#endif
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
        template<typename F, typename Rp>
        friend struct detail::future_unwrap_shared_state;
        template <class F, class Rp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_unwrap_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f);
#endif
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template <class> friend class packaged_task; // todo check if this works in windows
#else
        friend class packaged_task<R>;
#endif
        friend class detail::future_waiter;

        template <class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);

        template <class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);


        typedef typename detail::future_traits<R>::move_dest_type move_dest_type;
    public: // when_all

        NDNBOOST_THREAD_FUTURE(future_ptr a_future):
          base_type(a_future)
        {
        }

    public:
        NDNBOOST_THREAD_MOVABLE_ONLY(NDNBOOST_THREAD_FUTURE)
        typedef future_state::state state;
        typedef R value_type; // EXTENSION

        NDNBOOST_CONSTEXPR NDNBOOST_THREAD_FUTURE() {}
        //NDNBOOST_CONSTEXPR
        NDNBOOST_THREAD_FUTURE(exceptional_ptr const& ex):
            base_type(ex) {}

        ~NDNBOOST_THREAD_FUTURE() {}

        NDNBOOST_THREAD_FUTURE(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE) other) NDNBOOST_NOEXCEPT:
        base_type(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))))
        {
        }
        inline NDNBOOST_THREAD_FUTURE(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE<NDNBOOST_THREAD_FUTURE<R> >) other); // EXTENSION

        NDNBOOST_THREAD_FUTURE& operator=(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE) other) NDNBOOST_NOEXCEPT
        {
            this->base_type::operator=(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))));
            return *this;
        }

//        NDNBOOST_THREAD_FUTURE& operator=(exceptional_ptr const& ex)
//        {
//          this->future_=base_type::make_exceptional_future_ptr(ex);
//          return *this;
//        }

        shared_future<R> share()
        {
          return shared_future<R>(::ndnboost::move(*this));
        }

        void swap(NDNBOOST_THREAD_FUTURE& other)
        {
            static_cast<base_type*>(this)->swap(other);
        }

        // todo this function must be private and friendship provided to the internal users.
        void set_async()
        {
          this->future_->set_async();
        }
        // todo this function must be private and friendship provided to the internal users.
        void set_deferred()
        {
          this->future_->set_deferred();
        }

        // retrieving the value
        move_dest_type get()
        {
            if(!this->future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            future_ptr fut_=this->future_;
#ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
            this->future_.reset();
#endif
            return fut_->get();
        }

        template <typename R2>
        typename ndnboost::disable_if< is_void<R2>, move_dest_type>::type
        get_or(NDNBOOST_THREAD_RV_REF(R2) v)
        {
            if(!this->future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            this->future_->wait(false);
            future_ptr fut_=this->future_;
#ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
            this->future_.reset();
#endif
            if (fut_->has_value()) {
              return fut_->get();
            }
            else {
              return ndnboost::move(v);
            }
        }

        template <typename R2>
        typename ndnboost::disable_if< is_void<R2>, move_dest_type>::type
        get_or(R2 const& v)  // EXTENSION
        {
            if(!this->future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            this->future_->wait(false);
            future_ptr fut_=this->future_;
#ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
            this->future_.reset();
#endif
            if (fut_->has_value()) {
              return fut_->get();
            }
            else {
              return v;
            }
        }


#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

//        template<typename F>
//        auto then(F&& func) -> NDNBOOST_THREAD_FUTURE<decltype(func(*this))>;

//#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
//        template<typename RF>
//        inline NDNBOOST_THREAD_FUTURE<RF> then(RF(*func)(NDNBOOST_THREAD_FUTURE&));
//        template<typename RF>
//        inline NDNBOOST_THREAD_FUTURE<RF> then(launch policy, RF(*func)(NDNBOOST_THREAD_FUTURE&));
//#endif
        template<typename F>
        inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE)>::type>
        then(NDNBOOST_THREAD_FWD_REF(F) func);  // EXTENSION
        template<typename F>
        inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE)>::type>
        then(launch policy, NDNBOOST_THREAD_FWD_REF(F) func);  // EXTENSION

        template <typename R2>
        inline typename ndnboost::disable_if< is_void<R2>, NDNBOOST_THREAD_FUTURE<R> >::type
        fallback_to(NDNBOOST_THREAD_RV_REF(R2) v);  // EXTENSION
        template <typename R2>
        inline typename ndnboost::disable_if< is_void<R2>, NDNBOOST_THREAD_FUTURE<R> >::type
        fallback_to(R2 const& v);  // EXTENSION

#endif

//#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
//        inline
//        typename ndnboost::enable_if<
//          is_future_type<value_type>,
//          value_type
//        //NDNBOOST_THREAD_FUTURE<typename is_future_type<value_type>::type>
//        >::type
//        unwrap();
//#endif

    };

    NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) NDNBOOST_THREAD_FUTURE<T> NDNBOOST_THREAD_DCL_MOVABLE_END

        template <typename R2>
        class NDNBOOST_THREAD_FUTURE<NDNBOOST_THREAD_FUTURE<R2> > : public detail::basic_future<NDNBOOST_THREAD_FUTURE<R2> >
        {
          typedef NDNBOOST_THREAD_FUTURE<R2> R;

        private:
            typedef detail::basic_future<R> base_type;
            typedef typename base_type::future_ptr future_ptr;

            friend class shared_future<R>;
            friend class promise<R>;
    #if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            template <typename, typename, typename>
            friend struct detail::future_async_continuation_shared_state;
            template <typename, typename, typename>
            friend struct detail::future_deferred_continuation_shared_state;

            template <class F, class Rp, class Fp>
            friend NDNBOOST_THREAD_FUTURE<Rp>
            detail::make_future_async_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);

            template <class F, class Rp, class Fp>
            friend NDNBOOST_THREAD_FUTURE<Rp>
            detail::make_future_deferred_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);
    #endif
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
            template<typename F, typename Rp>
            friend struct detail::future_unwrap_shared_state;
        template <class F, class Rp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_unwrap_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f);
#endif
    #if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
            template <class> friend class packaged_task; // todo check if this works in windows
    #else
            friend class packaged_task<R>;
    #endif
            friend class detail::future_waiter;

            template <class Rp, class Fp>
            friend NDNBOOST_THREAD_FUTURE<Rp>
            detail::make_future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);

            template <class Rp, class Fp>
            friend NDNBOOST_THREAD_FUTURE<Rp>
            detail::make_future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f);


            typedef typename detail::future_traits<R>::move_dest_type move_dest_type;

            NDNBOOST_THREAD_FUTURE(future_ptr a_future):
              base_type(a_future)
            {
            }

        public:
            NDNBOOST_THREAD_MOVABLE_ONLY(NDNBOOST_THREAD_FUTURE)
            typedef future_state::state state;
            typedef R value_type; // EXTENSION

            NDNBOOST_CONSTEXPR NDNBOOST_THREAD_FUTURE() {}
            //NDNBOOST_CONSTEXPR
            NDNBOOST_THREAD_FUTURE(exceptional_ptr const& ex):
                base_type(ex) {}

            ~NDNBOOST_THREAD_FUTURE() {}

            NDNBOOST_THREAD_FUTURE(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE) other) NDNBOOST_NOEXCEPT:
            base_type(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))))
            {
            }

            NDNBOOST_THREAD_FUTURE& operator=(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE) other) NDNBOOST_NOEXCEPT
            {
                this->base_type::operator=(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))));
                return *this;
            }
//            NDNBOOST_THREAD_FUTURE& operator=(exceptional_ptr const& ex)
//            {
//              this->future_=base_type::make_exceptional_future_ptr(ex);
//              return *this;
//            }

            shared_future<R> share()
            {
              return shared_future<R>(::ndnboost::move(*this));
            }

            void swap(NDNBOOST_THREAD_FUTURE& other)
            {
                static_cast<base_type*>(this)->swap(other);
            }

            // todo this function must be private and friendship provided to the internal users.
            void set_async()
            {
              this->future_->set_async();
            }
            // todo this function must be private and friendship provided to the internal users.
            void set_deferred()
            {
              this->future_->set_deferred();
            }

            // retrieving the value
            move_dest_type get()
            {
                if(!this->future_)
                {
                    ndnboost::throw_exception(future_uninitialized());
                }
                future_ptr fut_=this->future_;
    #ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
                this->future_.reset();
    #endif
                return fut_->get();
            }
            move_dest_type get_or(NDNBOOST_THREAD_RV_REF(R) v) // EXTENSION
            {
                if(!this->future_)
                {
                    ndnboost::throw_exception(future_uninitialized());
                }
                this->future_->wait(false);
                future_ptr fut_=this->future_;
    #ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
                this->future_.reset();
    #endif
                if (fut_->has_value()) return fut_->get();
                else return ndnboost::move(v);
            }

            move_dest_type get_or(R const& v) // EXTENSION
            {
                if(!this->future_)
                {
                    ndnboost::throw_exception(future_uninitialized());
                }
                this->future_->wait(false);
                future_ptr fut_=this->future_;
    #ifdef NDNBOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
                this->future_.reset();
    #endif
                if (fut_->has_value()) return fut_->get();
                else return v;
            }


    #if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

    //        template<typename F>
    //        auto then(F&& func) -> NDNBOOST_THREAD_FUTURE<decltype(func(*this))>;

    //#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
    //        template<typename RF>
    //        inline NDNBOOST_THREAD_FUTURE<RF> then(RF(*func)(NDNBOOST_THREAD_FUTURE&));
    //        template<typename RF>
    //        inline NDNBOOST_THREAD_FUTURE<RF> then(launch policy, RF(*func)(NDNBOOST_THREAD_FUTURE&));
    //#endif
            template<typename F>
            inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE)>::type>
            then(NDNBOOST_THREAD_FWD_REF(F) func); // EXTENSION
            template<typename F>
            inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE)>::type>
            then(launch policy, NDNBOOST_THREAD_FWD_REF(F) func); // EXTENSION
    #endif

    #if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
            inline
            NDNBOOST_THREAD_FUTURE<R2>
            unwrap(); // EXTENSION
    #endif

  };

    template <typename R>
    class shared_future : public detail::basic_future<R>
    {

        typedef detail::basic_future<R> base_type;
        typedef typename base_type::future_ptr future_ptr;

        friend class detail::future_waiter;
        friend class promise<R>;

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
        template <typename, typename, typename>
        friend struct detail::future_async_continuation_shared_state;
        template <typename, typename, typename>
        friend struct detail::future_deferred_continuation_shared_state;

        template <class F, class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_async_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);

        template <class F, class Rp, class Fp>
        friend NDNBOOST_THREAD_FUTURE<Rp>
        detail::make_future_deferred_continuation_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c);
#endif
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template <class> friend class packaged_task;// todo check if this works in windows
#else
        friend class packaged_task<R>;
#endif
        shared_future(future_ptr a_future):
          base_type(a_future)
        {}

    public:
        NDNBOOST_THREAD_MOVABLE(shared_future)
        typedef R value_type; // EXTENSION

        shared_future(shared_future const& other):
        base_type(other)
        {}

        typedef future_state::state state;

        NDNBOOST_CONSTEXPR shared_future()
        {}
        //NDNBOOST_CONSTEXPR
        shared_future(exceptional_ptr const& ex):
            base_type(ex) {}
        ~shared_future()
        {}

        shared_future& operator=(shared_future const& other)
        {
            shared_future(other).swap(*this);
            return *this;
        }
//        shared_future& operator=(exceptional_ptr const& ex)
//        {
//          this->future_=base_type::make_exceptional_future_ptr(ex);
//          return *this;
//        }

        shared_future(NDNBOOST_THREAD_RV_REF(shared_future) other) NDNBOOST_NOEXCEPT :
        base_type(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))))
        {
            NDNBOOST_THREAD_RV(other).future_.reset();
        }
        shared_future(NDNBOOST_THREAD_RV_REF( NDNBOOST_THREAD_FUTURE<R> ) other) NDNBOOST_NOEXCEPT :
        base_type(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))))
        {
        }

        shared_future& operator=(NDNBOOST_THREAD_RV_REF(shared_future) other) NDNBOOST_NOEXCEPT
        {
            base_type::operator=(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))));
            return *this;
        }
        shared_future& operator=(NDNBOOST_THREAD_RV_REF( NDNBOOST_THREAD_FUTURE<R> ) other) NDNBOOST_NOEXCEPT
        {
            base_type::operator=(ndnboost::move(static_cast<base_type&>(NDNBOOST_THREAD_RV(other))));
            return *this;
        }

        void swap(shared_future& other) NDNBOOST_NOEXCEPT
        {
            static_cast<base_type*>(this)->swap(other);
        }

        // retrieving the value
        typename detail::shared_state<R>::shared_future_get_result_type get()
        {
            if(!this->future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }

            return this->future_->get_sh();
        }

        template <typename R2>
        typename ndnboost::disable_if< is_void<R2>, typename detail::shared_state<R>::shared_future_get_result_type>::type
        get_or(NDNBOOST_THREAD_RV_REF(R2) v) // EXTENSION
        {
            if(!this->future_)
            {
                ndnboost::throw_exception(future_uninitialized());
            }
            future_ptr fut_=this->future_;
            fut_->wait();
            if (fut_->has_value()) return fut_->get_sh();
            else return ndnboost::move(v);
        }

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

//        template<typename F>
//        auto then(F&& func) -> NDNBOOST_THREAD_FUTURE<decltype(func(*this))>;
//        template<typename F>
//        auto then(launch, F&& func) -> NDNBOOST_THREAD_FUTURE<decltype(func(*this))>;

//#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
//        template<typename RF>
//        inline NDNBOOST_THREAD_FUTURE<RF> then(RF(*func)(shared_future&));
//        template<typename RF>
//        inline NDNBOOST_THREAD_FUTURE<RF> then(launch policy, RF(*func)(shared_future&));
//#endif
        template<typename F>
        inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(shared_future)>::type>
        then(NDNBOOST_THREAD_FWD_REF(F) func); // EXTENSION
        template<typename F>
        inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(shared_future)>::type>
        then(launch policy, NDNBOOST_THREAD_FWD_REF(F) func); // EXTENSION
#endif
//#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
//        inline
//        typename ndnboost::enable_if_c<
//          is_future_type<value_type>::value,
//          NDNBOOST_THREAD_FUTURE<typename is_future_type<value_type>::type>
//        >::type
//        unwrap();
//#endif

    };

    NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) shared_future<T> NDNBOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
      /// Copy construction from a shared_future
      template <typename R>
      inline basic_future<R>::basic_future(const shared_future<R>& other) NDNBOOST_NOEXCEPT
      : future_(other.future_)
      {
      }
    }

    template <typename R>
    class promise
    {
        typedef ndnboost::shared_ptr<detail::shared_state<R> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
#include <ndnboost/detail/atomic_undef_macros.hpp>
          if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::shared_state<R>));
            }
#include <ndnboost/detail/atomic_redef_macros.hpp>
#endif
        }

    public:
        NDNBOOST_THREAD_MOVABLE_ONLY(promise)
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(ndnboost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::shared_state<R> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::shared_state<R>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::shared_state<R>()),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);

                if(!future_->done && !future_->is_constructed)
                {
                    future_->mark_exceptional_finish_internal(ndnboost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT :
            future_(NDNBOOST_THREAD_RV(rhs).future_),future_obtained(NDNBOOST_THREAD_RV(rhs).future_obtained)
        {
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
        }
        promise & operator=(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT
        {
            future_=NDNBOOST_THREAD_RV(rhs).future_;
            future_obtained=NDNBOOST_THREAD_RV(rhs).future_obtained;
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        NDNBOOST_THREAD_FUTURE<R> get_future()
        {
            lazy_init();
            if (future_.get()==0)
            {
                ndnboost::throw_exception(promise_moved());
            }
            if (future_obtained)
            {
                ndnboost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return NDNBOOST_THREAD_FUTURE<R>(future_);
        }

        void set_value(typename detail::future_traits<R>::source_reference_type r)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(r, lock);
        }

//         void set_value(R && r);
        void set_value(typename detail::future_traits<R>::rvalue_source_type r)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
            future_->mark_finished_with_result_internal(ndnboost::forward<R>(r), lock);
#else
            future_->mark_finished_with_result_internal(static_cast<typename detail::future_traits<R>::rvalue_source_type>(r), lock);
#endif
        }

        void set_exception(ndnboost::exception_ptr p)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p, lock);
        }
        template <typename E>
        void set_exception(E ex)
        {
          set_exception(copy_exception(ex));
        }
        // setting the result with deferred notification
        void set_value_ndnboostat_thread_exit(const R& r)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_value_ndnboostat_thread_exit(r);
        }

        void set_value_ndnboostat_thread_exit(NDNBOOST_THREAD_RV_REF(R) r)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_value_ndnboostat_thread_exit(ndnboost::move(r));
        }
        void set_exception_ndnboostat_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_exception_ndnboostat_thread_exit(e);
        }
        template <typename E>
        void set_exception_ndnboostat_thread_exit(E ex)
        {
          set_exception_ndnboostat_thread_exit(copy_exception(ex));
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };

    template <typename R>
    class promise<R&>
    {
        typedef ndnboost::shared_ptr<detail::shared_state<R&> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
#include <ndnboost/detail/atomic_undef_macros.hpp>
            if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::shared_state<R&>));
            }
#include <ndnboost/detail/atomic_redef_macros.hpp>
#endif
        }

    public:
        NDNBOOST_THREAD_MOVABLE_ONLY(promise)
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(ndnboost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::shared_state<R&> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::shared_state<R&>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::shared_state<R&>()),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);

                if(!future_->done && !future_->is_constructed)
                {
                    future_->mark_exceptional_finish_internal(ndnboost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT :
            future_(NDNBOOST_THREAD_RV(rhs).future_),future_obtained(NDNBOOST_THREAD_RV(rhs).future_obtained)
        {
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
        }
        promise & operator=(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT
        {
            future_=NDNBOOST_THREAD_RV(rhs).future_;
            future_obtained=NDNBOOST_THREAD_RV(rhs).future_obtained;
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        NDNBOOST_THREAD_FUTURE<R&> get_future()
        {
            lazy_init();
            if (future_.get()==0)
            {
                ndnboost::throw_exception(promise_moved());
            }
            if (future_obtained)
            {
                ndnboost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return NDNBOOST_THREAD_FUTURE<R&>(future_);
        }

        void set_value(R& r)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(r, lock);
        }

        void set_exception(ndnboost::exception_ptr p)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p, lock);
        }
        template <typename E>
        void set_exception(E ex)
        {
          set_exception(copy_exception(ex));
        }

        // setting the result with deferred notification
        void set_value_ndnboostat_thread_exit(R& r)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_value_ndnboostat_thread_exit(r);
        }

        void set_exception_ndnboostat_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_exception_ndnboostat_thread_exit(e);
        }
        template <typename E>
        void set_exception_ndnboostat_thread_exit(E ex)
        {
          set_exception_ndnboostat_thread_exit(copy_exception(ex));
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };
    template <>
    class promise<void>
    {
        typedef ndnboost::shared_ptr<detail::shared_state<void> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
            if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::shared_state<void>));
            }
#endif
        }
    public:
        NDNBOOST_THREAD_MOVABLE_ONLY(promise)

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(ndnboost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::shared_state<void> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::shared_state<void>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined NDNBOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::shared_state<void>),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);

                if(!future_->done && !future_->is_constructed)
                {
                    future_->mark_exceptional_finish_internal(ndnboost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT :
            future_(NDNBOOST_THREAD_RV(rhs).future_),future_obtained(NDNBOOST_THREAD_RV(rhs).future_obtained)
        {
          // we need to release the future as shared_ptr doesn't implements move semantics
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
        }

        promise & operator=(NDNBOOST_THREAD_RV_REF(promise) rhs) NDNBOOST_NOEXCEPT
        {
            future_=NDNBOOST_THREAD_RV(rhs).future_;
            future_obtained=NDNBOOST_THREAD_RV(rhs).future_obtained;
            NDNBOOST_THREAD_RV(rhs).future_.reset();
            NDNBOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        NDNBOOST_THREAD_FUTURE<void> get_future()
        {
            lazy_init();

            if (future_.get()==0)
            {
                ndnboost::throw_exception(promise_moved());
            }
            if(future_obtained)
            {
                ndnboost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            //return NDNBOOST_THREAD_MAKE_RV_REF(NDNBOOST_THREAD_FUTURE<void>(future_));
            return NDNBOOST_THREAD_FUTURE<void>(future_);
        }

        void set_value()
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(lock);
        }

        void set_exception(ndnboost::exception_ptr p)
        {
            lazy_init();
            ndnboost::unique_lock<ndnboost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                ndnboost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p,lock);
        }
        template <typename E>
        void set_exception(E ex)
        {
          set_exception(copy_exception(ex));
        }

        // setting the result with deferred notification
        void set_value_ndnboostat_thread_exit()
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_value_ndnboostat_thread_exit();
        }

        void set_exception_ndnboostat_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              ndnboost::throw_exception(promise_moved());
          }
          future_->set_exception_ndnboostat_thread_exit(e);
        }
        template <typename E>
        void set_exception_ndnboostat_thread_exit(E ex)
        {
          set_exception_ndnboostat_thread_exit(copy_exception(ex));
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };
}
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
namespace ndnboost { namespace container {
    template <class R, class Alloc>
    struct uses_allocator< ::ndnboost::promise<R> , Alloc> : true_type
    {
    };
}}
#if ! defined  NDNBOOST_NO_CXX11_ALLOCATOR
namespace std {
    template <class R, class Alloc>
    struct uses_allocator< ::ndnboost::promise<R> , Alloc> : true_type
    {
    };
}
#endif
#endif

namespace ndnboost
{

    NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) promise<T> NDNBOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
      template<typename R>
      struct task_base_shared_state;
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
      template<typename R, typename ...ArgTypes>
      struct task_base_shared_state<R(ArgTypes...)>:
#else
      template<typename R>
      struct task_base_shared_state<R()>:
#endif
#else
      template<typename R>
      struct task_base_shared_state:
#endif
            detail::shared_state<R>
        {
            bool started;

            task_base_shared_state():
                started(false)
            {}

            void reset()
            {
              started=false;
            }
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            virtual void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)=0;
            void run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
#else
            virtual void do_run()=0;
            void run()
#endif
            {
                {
                    ndnboost::lock_guard<ndnboost::mutex> lk(this->mutex);
                    if(started)
                    {
                        ndnboost::throw_exception(task_already_started());
                    }
                    started=true;
                }
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                do_run(ndnboost::forward<ArgTypes>(args)...);
#else
                do_run();
#endif
            }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            virtual void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)=0;
            void apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
#else
            virtual void do_apply()=0;
            void apply()
#endif
            {
                {
                    ndnboost::lock_guard<ndnboost::mutex> lk(this->mutex);
                    if(started)
                    {
                        ndnboost::throw_exception(task_already_started());
                    }
                    started=true;
                }
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                do_apply(ndnboost::forward<ArgTypes>(args)...);
#else
                do_apply();
#endif
            }

            void owner_destroyed()
            {
                ndnboost::unique_lock<ndnboost::mutex> lk(this->mutex);
                if(!started)
                {
                    started=true;
                    this->mark_exceptional_finish_internal(ndnboost::copy_exception(ndnboost::broken_promise()), lk);
                }
            }

        };

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template<typename F, typename R>
        struct task_shared_state;
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename F, typename R, typename ...ArgTypes>
        struct task_shared_state<F, R(ArgTypes...)>:
          task_base_shared_state<R(ArgTypes...)>
#else
        template<typename F, typename R>
        struct task_shared_state<F, R()>:
          task_base_shared_state<R()>
#endif
#else
        template<typename F, typename R>
        struct task_shared_state:
            task_base_shared_state<R>
#endif
        {
        private:
          task_shared_state(task_shared_state&);
        public:
            F f;
            task_shared_state(F const& f_):
                f(f_)
            {}
            task_shared_state(NDNBOOST_THREAD_RV_REF(F) f_):
              f(ndnboost::move(f_))
            {}

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->set_value_ndnboostat_thread_exit(f(ndnboost::forward<ArgTypes>(args)...));
                }
#else
            void do_apply()
            {
                try
                {
                    this->set_value_ndnboostat_thread_exit(f());
                }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_ndnboostat_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_ndnboostat_thread_exit(current_exception());
                }
            }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->mark_finished_with_result(f(ndnboost::forward<ArgTypes>(args)...));
                }
#else
            void do_run()
            {
                try
                {
#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
                  R res((f()));
                  this->mark_finished_with_result(ndnboost::move(res));
#else
                  this->mark_finished_with_result(f());
#endif
                  }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename F, typename R, typename ...ArgTypes>
        struct task_shared_state<F, R&(ArgTypes...)>:
          task_base_shared_state<R&(ArgTypes...)>
#else
        template<typename F, typename R>
        struct task_shared_state<F, R&()>:
          task_base_shared_state<R&()>
#endif
#else
        template<typename F, typename R>
        struct task_shared_state<F,R&>:
            task_base_shared_state<R&>
#endif
        {
        private:
          task_shared_state(task_shared_state&);
        public:
            F f;
            task_shared_state(F const& f_):
                f(f_)
            {}
            task_shared_state(NDNBOOST_THREAD_RV_REF(F) f_):
                f(ndnboost::move(f_))
            {}

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->set_value_ndnboostat_thread_exit(f(ndnboost::forward<ArgTypes>(args)...));
                }
#else
            void do_apply()
            {
                try
                {
                    this->set_value_ndnboostat_thread_exit(f());
                }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_ndnboostat_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_ndnboostat_thread_exit(current_exception());
                }
            }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->mark_finished_with_result(f(ndnboost::forward<ArgTypes>(args)...));
                }
#else
            void do_run()
            {
                try
                {
                  R& res((f()));
                  this->mark_finished_with_result(res);
                }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename R, typename ...ArgTypes>
        struct task_shared_state<R (*)(ArgTypes...), R(ArgTypes...)>:
          task_base_shared_state<R(ArgTypes...)>
#else
        template<typename R>
        struct task_shared_state<R (*)(), R()>:
          task_base_shared_state<R()>
#endif
#else
        template<typename R>
        struct task_shared_state<R (*)(), R> :
           task_base_shared_state<R>
#endif
            {
            private:
              task_shared_state(task_shared_state&);
            public:
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                R (*f)(NDNBOOST_THREAD_RV_REF(ArgTypes) ... );
                task_shared_state(R (*f_)(NDNBOOST_THREAD_RV_REF(ArgTypes) ... )):
                    f(f_)
                {}
#else
                R (*f)();
                task_shared_state(R (*f_)()):
                    f(f_)
                {}
#endif

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->set_value_ndnboostat_thread_exit(f(ndnboost::forward<ArgTypes>(args)...));
                    }
#else
                void do_apply()
                {
                    try
                    {
                        R r((f()));
                        this->set_value_ndnboostat_thread_exit(ndnboost::move(r));
                    }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->set_interrupted_ndnboostat_thread_exit();
                    }
#endif
                    catch(...)
                    {
                        this->set_exception_ndnboostat_thread_exit(current_exception());
                    }
                }


#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->mark_finished_with_result(f(ndnboost::forward<ArgTypes>(args)...));
                    }
#else
                void do_run()
                {
                    try
                    {
                        R res((f()));
                        this->mark_finished_with_result(ndnboost::move(res));
                    }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->mark_interrupted_finish();
                    }
#endif
                    catch(...)
                    {
                        this->mark_exceptional_finish();
                    }
                }
            };
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename R, typename ...ArgTypes>
        struct task_shared_state<R& (*)(ArgTypes...), R&(ArgTypes...)>:
          task_base_shared_state<R&(ArgTypes...)>
#else
        template<typename R>
        struct task_shared_state<R& (*)(), R&()>:
          task_base_shared_state<R&()>
#endif
#else
        template<typename R>
        struct task_shared_state<R& (*)(), R&> :
           task_base_shared_state<R&>
#endif
            {
            private:
              task_shared_state(task_shared_state&);
            public:
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                R& (*f)(NDNBOOST_THREAD_RV_REF(ArgTypes) ... );
                task_shared_state(R& (*f_)(NDNBOOST_THREAD_RV_REF(ArgTypes) ... )):
                    f(f_)
                {}
#else
                R& (*f)();
                task_shared_state(R& (*f_)()):
                    f(f_)
                {}
#endif

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->set_value_ndnboostat_thread_exit(f(ndnboost::forward<ArgTypes>(args)...));
                    }
#else
                void do_apply()
                {
                    try
                    {
                      this->set_value_ndnboostat_thread_exit(f());
                    }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->set_interrupted_ndnboostat_thread_exit();
                    }
#endif
                    catch(...)
                    {
                        this->set_exception_ndnboostat_thread_exit(current_exception());
                    }
                }


#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->mark_finished_with_result(f(ndnboost::forward<ArgTypes>(args)...));
                    }
#else
                void do_run()
                {
                    try
                    {
                        this->mark_finished_with_result(f());
                    }
#endif
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->mark_interrupted_finish();
                    }
#endif
                    catch(...)
                    {
                        this->mark_exceptional_finish();
                    }
                }
            };
#endif
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename F, typename ...ArgTypes>
        struct task_shared_state<F, void(ArgTypes...)>:
          task_base_shared_state<void(ArgTypes...)>
#else
        template<typename F>
        struct task_shared_state<F, void()>:
          task_base_shared_state<void()>
#endif
#else
        template<typename F>
        struct task_shared_state<F,void>:
          task_base_shared_state<void>
#endif
        {
        private:
          task_shared_state(task_shared_state&);
        public:
            F f;
            task_shared_state(F const& f_):
                f(f_)
            {}
            task_shared_state(NDNBOOST_THREAD_RV_REF(F) f_):
                f(ndnboost::move(f_))
            {}

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
              try
              {
                f(ndnboost::forward<ArgTypes>(args)...);
#else
            void do_apply()
            {
                try
                {
                    f();
#endif
                  this->set_value_ndnboostat_thread_exit();
                }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_ndnboostat_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_ndnboostat_thread_exit(current_exception());
                }
            }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(ndnboost::forward<ArgTypes>(args)...);
#else
            void do_run()
            {
                try
                {
                    f();
#endif
                    this->mark_finished_with_result();
                }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename ...ArgTypes>
        struct task_shared_state<void (*)(ArgTypes...), void(ArgTypes...)>:
        task_base_shared_state<void(ArgTypes...)>
#else
        template<>
        struct task_shared_state<void (*)(), void()>:
        task_base_shared_state<void()>
#endif
#else
        template<>
        struct task_shared_state<void (*)(),void>:
          task_base_shared_state<void>
#endif
        {
        private:
          task_shared_state(task_shared_state&);
        public:
            void (*f)();
            task_shared_state(void (*f_)()):
                f(f_)
            {}

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(ndnboost::forward<ArgTypes>(args)...);
#else
            void do_apply()
            {
                try
                {
                    f();
#endif
                    this->set_value_ndnboostat_thread_exit();
                }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_ndnboostat_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_ndnboostat_thread_exit(current_exception());
                }
            }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(NDNBOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(ndnboost::forward<ArgTypes>(args)...);
#else
            void do_run()
            {
                try
                {
                  f();
#endif
                  this->mark_finished_with_result();
                }
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };
    }

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
    template<typename R, typename ...ArgTypes>
    class packaged_task<R(ArgTypes...)>
    {
      typedef ndnboost::shared_ptr<detail::task_base_shared_state<R(ArgTypes...)> > task_ptr;
      ndnboost::shared_ptr<detail::task_base_shared_state<R(ArgTypes...)> > task;
  #else
    template<typename R>
    class packaged_task<R()>
    {
      typedef ndnboost::shared_ptr<detail::task_base_shared_state<R()> > task_ptr;
      ndnboost::shared_ptr<detail::task_base_shared_state<R()> > task;
  #endif
#else
    template<typename R>
    class packaged_task
    {
      typedef ndnboost::shared_ptr<detail::task_base_shared_state<R> > task_ptr;
      ndnboost::shared_ptr<detail::task_base_shared_state<R> > task;
#endif
        bool future_obtained;
        struct dummy;

    public:
        typedef R result_type;
        NDNBOOST_THREAD_MOVABLE_ONLY(packaged_task)

        packaged_task():
            future_obtained(false)
        {}

        // construction and destruction
#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        explicit packaged_task(R(*f)(), NDNBOOST_THREAD_FWD_REF(ArgTypes)... args)
        {
            typedef R(*FR)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...);
            typedef detail::task_shared_state<FR,R(ArgTypes...)> task_shared_state_type;
            task= task_ptr(new task_shared_state_type(f, ndnboost::forward<ArgTypes>(args)...));
            future_obtained=false;
        }
  #else
        explicit packaged_task(R(*f)())
        {
            typedef R(*FR)();
            typedef detail::task_shared_state<FR,R()> task_shared_state_type;
            task= task_ptr(new task_shared_state_type(f));
            future_obtained=false;
        }
  #endif
#else
        explicit packaged_task(R(*f)())
        {
              typedef R(*FR)();
            typedef detail::task_shared_state<FR,R> task_shared_state_type;
            task= task_ptr(new task_shared_state_type(f));
            future_obtained=false;
        }
#endif
#endif
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F>
        explicit packaged_task(NDNBOOST_THREAD_FWD_REF(F) f
            , typename ndnboost::disable_if<is_same<typename decay<F>::type, packaged_task>, dummy* >::type=0
            )
        {
          //typedef typename remove_cv<typename remove_reference<F>::type>::type FR;
          typedef typename decay<F>::type FR;
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_shared_state<FR,R(ArgTypes...)> task_shared_state_type;
  #else
            typedef detail::task_shared_state<FR,R()> task_shared_state_type;
  #endif
#else
            typedef detail::task_shared_state<FR,R> task_shared_state_type;
#endif
            task = task_ptr(new task_shared_state_type(ndnboost::forward<F>(f)));
            future_obtained = false;

        }

#else
        template <class F>
        explicit packaged_task(F const& f
            , typename ndnboost::disable_if<is_same<typename decay<F>::type, packaged_task>, dummy* >::type=0
            )
        {
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_shared_state<F,R(ArgTypes...)> task_shared_state_type;
  #else
            typedef detail::task_shared_state<F,R()> task_shared_state_type;
  #endif
#else
            typedef detail::task_shared_state<F,R> task_shared_state_type;
#endif
            task = task_ptr(new task_shared_state_type(f));
            future_obtained=false;
        }
        template <class F>
        explicit packaged_task(NDNBOOST_THREAD_RV_REF(F) f)
        {
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_shared_state<F,R(ArgTypes...)> task_shared_state_type;
            task = task_ptr(new task_shared_state_type(ndnboost::forward<F>(f)));
#else
            typedef detail::task_shared_state<F,R()> task_shared_state_type;
            task = task_ptr(new task_shared_state_type(ndnboost::move(f))); // TODO forward
#endif
#else
            typedef detail::task_shared_state<F,R> task_shared_state_type;
            task = task_ptr(new task_shared_state_type(ndnboost::forward<F>(f)));
#endif
            future_obtained=false;

        }
#endif

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
        template <class Allocator>
        packaged_task(ndnboost::allocator_arg_t, Allocator a, R(*f)())
        {
          typedef R(*FR)();
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_shared_state<FR,R(ArgTypes...)> task_shared_state_type;
  #else
          typedef detail::task_shared_state<FR,R()> task_shared_state_type;
  #endif
#else
          typedef detail::task_shared_state<FR,R> task_shared_state_type;
#endif
          typedef typename Allocator::template rebind<task_shared_state_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_shared_state_type(f), D(a2, 1) );
          future_obtained = false;
        }
#endif // NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

#if ! defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F, class Allocator>
        packaged_task(ndnboost::allocator_arg_t, Allocator a, NDNBOOST_THREAD_FWD_REF(F) f)
        {
          //typedef typename remove_cv<typename remove_reference<F>::type>::type FR;
          typedef typename decay<F>::type FR;

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_shared_state<FR,R(ArgTypes...)> task_shared_state_type;
  #else
          typedef detail::task_shared_state<FR,R()> task_shared_state_type;
  #endif
#else
          typedef detail::task_shared_state<FR,R> task_shared_state_type;
#endif
          typedef typename Allocator::template rebind<task_shared_state_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_shared_state_type(ndnboost::forward<F>(f)), D(a2, 1) );
          future_obtained = false;
        }
#else // ! defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F, class Allocator>
        packaged_task(ndnboost::allocator_arg_t, Allocator a, const F& f)
        {
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_shared_state<F,R(ArgTypes...)> task_shared_state_type;
  #else
          typedef detail::task_shared_state<F,R()> task_shared_state_type;
  #endif
#else
          typedef detail::task_shared_state<F,R> task_shared_state_type;
#endif
          typedef typename Allocator::template rebind<task_shared_state_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_shared_state_type(f), D(a2, 1) );
          future_obtained = false;
        }
        template <class F, class Allocator>
        packaged_task(ndnboost::allocator_arg_t, Allocator a, NDNBOOST_THREAD_RV_REF(F) f)
        {
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_shared_state<F,R(ArgTypes...)> task_shared_state_type;
  #else
          typedef detail::task_shared_state<F,R()> task_shared_state_type;
  #endif
#else
          typedef detail::task_shared_state<F,R> task_shared_state_type;
#endif
          typedef typename Allocator::template rebind<task_shared_state_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          task = task_ptr(::new(a2.allocate(1)) task_shared_state_type(ndnboost::forward<F>(f)), D(a2, 1) );
#else
          task = task_ptr(::new(a2.allocate(1)) task_shared_state_type(ndnboost::move(f)), D(a2, 1) );  // TODO forward
#endif
          future_obtained = false;
        }

#endif //NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#endif // NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS

        ~packaged_task() {
            if(task) {
                task->owner_destroyed();
            }
        }

        // assignment
        packaged_task(NDNBOOST_THREAD_RV_REF(packaged_task) other) NDNBOOST_NOEXCEPT
        : future_obtained(NDNBOOST_THREAD_RV(other).future_obtained) {
            task.swap(NDNBOOST_THREAD_RV(other).task);
            NDNBOOST_THREAD_RV(other).future_obtained=false;
        }
        packaged_task& operator=(NDNBOOST_THREAD_RV_REF(packaged_task) other) NDNBOOST_NOEXCEPT {

            // todo use forward
#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
            packaged_task temp(ndnboost::move(other));
#else
            packaged_task temp(static_cast<NDNBOOST_THREAD_RV_REF(packaged_task)>(other));
#endif
            swap(temp);
            return *this;
        }

        void reset() {
            if (!valid())
                throw future_error(system::make_error_code(future_errc::no_state));
            task->reset();
            future_obtained=false;
        }

        void swap(packaged_task& other) NDNBOOST_NOEXCEPT {
            task.swap(other.task);
            std::swap(future_obtained,other.future_obtained);
        }
        bool valid() const NDNBOOST_NOEXCEPT {
          return task.get()!=0;
        }

        // result retrieval
        NDNBOOST_THREAD_FUTURE<R> get_future() {
            if(!task) {
                ndnboost::throw_exception(task_moved());
            } else if(!future_obtained) {
                future_obtained=true;
                return NDNBOOST_THREAD_FUTURE<R>(task);
            } else {
                ndnboost::throw_exception(future_already_retrieved());
            }
        }

        // execution
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        void operator()(NDNBOOST_THREAD_RV_REF(ArgTypes)... args) {
            if(!task) {
                ndnboost::throw_exception(task_moved());
            }
            task->run(ndnboost::forward<ArgTypes>(args)...);
        }
        void make_ready_ndnboostat_thread_exit(ArgTypes... args) {
          if(!task) {
              ndnboost::throw_exception(task_moved());
          }
          if (task->has_value()) {
                ndnboost::throw_exception(promise_already_satisfied());
          }
          task->apply(ndnboost::forward<ArgTypes>(args)...);
        }
#else
        void operator()() {
            if(!task) {
                ndnboost::throw_exception(task_moved());
            }
            task->run();
        }
        void make_ready_ndnboostat_thread_exit() {
          if(!task) {
              ndnboost::throw_exception(task_moved());
          }
          if (task->has_value()) ndnboost::throw_exception(promise_already_satisfied());
          task->apply();
        }
#endif
        template<typename F>
        void set_wait_callback(F f) {
            task->set_wait_callback(f,this);
        }
    };
}
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
namespace ndnboost { namespace container {
    template <class R, class Alloc>
    struct uses_allocator< ::ndnboost::packaged_task<R> , Alloc> : true_type
    {};
}}
#if ! defined  NDNBOOST_NO_CXX11_ALLOCATOR
namespace std {
    template <class R, class Alloc>
    struct uses_allocator< ::ndnboost::packaged_task<R> , Alloc> : true_type
    {};
}
#endif
#endif

namespace ndnboost
{
  NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) packaged_task<T> NDNBOOST_THREAD_DCL_MOVABLE_END

namespace detail
{
  ////////////////////////////////
  // make_future_deferred_shared_state
  ////////////////////////////////
  template <class Rp, class Fp>
  NDNBOOST_THREAD_FUTURE<Rp>
  make_future_deferred_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f) {
    shared_ptr<future_deferred_shared_state<Rp, Fp> >
        h(new future_deferred_shared_state<Rp, Fp>(ndnboost::forward<Fp>(f)));
    return NDNBOOST_THREAD_FUTURE<Rp>(h);
  }

  ////////////////////////////////
  // make_future_async_shared_state
  ////////////////////////////////
  template <class Rp, class Fp>
  NDNBOOST_THREAD_FUTURE<Rp>
  make_future_async_shared_state(NDNBOOST_THREAD_FWD_REF(Fp) f) {
    shared_ptr<future_async_shared_state<Rp, Fp> >
        h(new future_async_shared_state<Rp, Fp>(ndnboost::forward<Fp>(f)));
    return NDNBOOST_THREAD_FUTURE<Rp>(h);
  }
}

    ////////////////////////////////
    // template <class F, class... ArgTypes>
    // future<R> async(launch policy, F&&, ArgTypes&&...);
    ////////////////////////////////

#if defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
  template <class R, class... ArgTypes>
  NDNBOOST_THREAD_FUTURE<R>
  async(launch policy, R(*f)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...), NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
    typedef R(*F)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...);
    typedef detail::invoker<typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
    typedef typename BF::result_type Rp;

    if (underlying_cast<int>(policy) & int(launch::async)) {
      return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_async_shared_state<Rp>(
              BF(
                  thread_detail::decay_copy(ndnboost::forward<F>(f))
                  , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
              )
          ));
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_deferred_shared_state<Rp>(
              BF(
                  thread_detail::decay_copy(ndnboost::forward<F>(f))
                  , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
              )
          ));
    } else {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
    }
  }

#else // defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)

  template <class R>
  NDNBOOST_THREAD_FUTURE<R>
  async(launch policy, R(*f)()) {
  #if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
    typedef packaged_task<R()> packaged_task_type;
  #else
    typedef packaged_task<R> packaged_task_type;
  #endif

    if (underlying_cast<int>(policy) & int(launch::async)) {
      packaged_task_type pt( f );
      NDNBOOST_THREAD_FUTURE<R> ret = NDNBOOST_THREAD_MAKE_RV_REF(pt.get_future());
      ret.set_async();
      ndnboost::thread( ndnboost::move(pt) ).detach();
      return ::ndnboost::move(ret);
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
    } else {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
    }
  }
#endif
#endif // defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)

#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)

  template <class F, class ...ArgTypes>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
      typename decay<ArgTypes>::type...
  )>::type>
  async(launch policy, NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
    typedef typename ndnboost::result_of<typename decay<F>::type(
        typename decay<ArgTypes>::type...
    )>::type R;
    typedef detail::invoker<typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
    typedef typename BF::result_type Rp;

    if (underlying_cast<int>(policy) & int(launch::async)) {
      return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_async_shared_state<Rp>(
              BF(
                  thread_detail::decay_copy(ndnboost::forward<F>(f))
                , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
              )
          ));
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_deferred_shared_state<Rp>(
              BF(
                  thread_detail::decay_copy(ndnboost::forward<F>(f))
                , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
              )
          ));
    } else {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
    }
  }

#else // defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)

  template <class F>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type()>::type>
  async(launch policy, NDNBOOST_THREAD_FWD_REF(F) f) {
    typedef typename ndnboost::result_of<typename decay<F>::type()>::type R;
#if defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
    typedef packaged_task<R()> packaged_task_type;
#else // defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
    typedef packaged_task<R> packaged_task_type;
#endif // defined NDNBOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK

    if (underlying_cast<int>(policy) & int(launch::async)) {
      packaged_task_type pt( ndnboost::forward<F>(f) );
      NDNBOOST_THREAD_FUTURE<R> ret = pt.get_future();
      ret.set_async();
      ndnboost::thread( ndnboost::move(pt) ).detach();
      return ::ndnboost::move(ret);
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
      //          return ndnboost::detail::make_future_deferred_shared_state<Rp>(
      //              BF(
      //                  thread_detail::decay_copy(ndnboost::forward<F>(f))
      //              )
      //          );
    } else {
      std::terminate();
      NDNBOOST_THREAD_FUTURE<R> ret;
      return ::ndnboost::move(ret);
    }
  }
#endif // defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)

#ifdef NDNBOOST_THREAD_PROVIDES_EXECUTORS
namespace detail {
    /////////////////////////
    /// shared_state_nullary_task
    /////////////////////////
    template<typename Rp, typename Fp>
    struct shared_state_nullary_task
    {
      shared_state<Rp>* that;
      Fp f_;
    public:

      shared_state_nullary_task(shared_state<Rp>* st, NDNBOOST_THREAD_FWD_REF(Fp) f)
      : that(st), f_(ndnboost::forward<Fp>(f))
      {};
#if ! defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
      NDNBOOST_THREAD_MOVABLE(shared_state_nullary_task)
      shared_state_nullary_task(shared_state_nullary_task const& x) //NDNBOOST_NOEXCEPT
      : that(x.that), f_(x.f_)
      {}
      shared_state_nullary_task& operator=(NDNBOOST_COPY_ASSIGN_REF(shared_state_nullary_task) x) //NDNBOOST_NOEXCEPT
      {
        if (this != &x) {
          that=x.that;
          f_=x.f_;
        }
        return *this;
      }
      // move
      shared_state_nullary_task(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) //NDNBOOST_NOEXCEPT
      : that(x.that), f_(ndnboost::move(x.f_))
      {
        x.that=0;
      }
      shared_state_nullary_task& operator=(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) //NDNBOOST_NOEXCEPT
      {
        if (this != &x) {
          that=x.that;
          f_=ndnboost::move(x.f_);
          x.that=0;
        }
        return *this;
      }
#endif
      void operator()() {
        try {
          that->mark_finished_with_result(f_());
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(thread_interrupted& ) {
          that->mark_interrupted_finish();
#endif // defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(...) {
          that->mark_exceptional_finish();
        }

      }
    };

    template<typename Fp>
    struct shared_state_nullary_task<void, Fp>
    {
      shared_state<void>* that;
      Fp f_;
    public:
      shared_state_nullary_task(shared_state<void>* st, NDNBOOST_THREAD_FWD_REF(Fp) f)
      : that(st), f_(ndnboost::forward<Fp>(f))
      {};
#if ! defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
      NDNBOOST_THREAD_MOVABLE(shared_state_nullary_task)
      shared_state_nullary_task(shared_state_nullary_task const& x) //NDNBOOST_NOEXCEPT
      : that(x.that), f_(x.f_)
      {}
      shared_state_nullary_task& operator=(NDNBOOST_COPY_ASSIGN_REF(shared_state_nullary_task) x) //NDNBOOST_NOEXCEPT
      {
        if (this != &x) {
          that=x.that;
          f_=x.f_;
        }
        return *this;
      }
      // move
      shared_state_nullary_task(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) NDNBOOST_NOEXCEPT
      : that(x.that), f_(ndnboost::move(x.f_))
      {
        x.that=0;
      }
      shared_state_nullary_task& operator=(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) NDNBOOST_NOEXCEPT {
        if (this != &x) {
          that=x.that;
          f_=ndnboost::move(x.f_);
          x.that=0;
        }
        return *this;
      }
#endif
      void operator()() {
        try {
          f_();
          that->mark_finished_with_result();
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(thread_interrupted& ) {
          that->mark_interrupted_finish();
#endif // defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(...) {
          that->mark_exceptional_finish();
        }
      }
    };

    template<typename Rp, typename Fp>
    struct shared_state_nullary_task<Rp&, Fp>
    {
      shared_state<Rp&>* that;
      Fp f_;
    public:
      shared_state_nullary_task(shared_state<Rp&>* st, NDNBOOST_THREAD_FWD_REF(Fp) f)
        : that(st), f_(ndnboost::forward<Fp>(f))
      {}
#if ! defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
      NDNBOOST_THREAD_MOVABLE(shared_state_nullary_task)
      shared_state_nullary_task(shared_state_nullary_task const& x) NDNBOOST_NOEXCEPT
      : that(x.that), f_(x.f_) {}

      shared_state_nullary_task& operator=(NDNBOOST_COPY_ASSIGN_REF(shared_state_nullary_task) x) NDNBOOST_NOEXCEPT {
        if (this != &x){
          that=x.that;
          f_=x.f_;
        }
        return *this;
      }
      // move
      shared_state_nullary_task(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) NDNBOOST_NOEXCEPT
      : that(x.that), f_(ndnboost::move(x.f_))
      {
        x.that=0;
      }
      shared_state_nullary_task& operator=(NDNBOOST_THREAD_RV_REF(shared_state_nullary_task) x) NDNBOOST_NOEXCEPT {
        if (this != &x) {
          that=x.that;
          f_=ndnboost::move(x.f_);
          x.that=0;
        }
        return *this;
      }
#endif
      void operator()() {
        try {
          that->mark_finished_with_result(f_());
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(thread_interrupted& ) {
          that->mark_interrupted_finish();
#endif // defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
        } catch(...) {
          that->mark_exceptional_finish();
        }
      }
    };

    /////////////////////////
    /// future_executor_shared_state_base
    /////////////////////////
    template<typename Rp, typename Executor>
    struct future_executor_shared_state: shared_state<Rp>
    {
      typedef shared_state<Rp> base_type;
    protected:
    public:
      template<typename Fp>
      future_executor_shared_state(Executor& ex, NDNBOOST_THREAD_FWD_REF(Fp) f) {
        this->set_executor();
        shared_state_nullary_task<Rp,Fp> t(this, ndnboost::forward<Fp>(f));
        ex.submit(ndnboost::move(t));
      }

      ~future_executor_shared_state() {
        this->wait(false);
      }
    };

    ////////////////////////////////
    // make_future_executor_shared_state
    ////////////////////////////////
    template <class Rp, class Fp, class Executor>
    NDNBOOST_THREAD_FUTURE<Rp>
    make_future_executor_shared_state(Executor& ex, NDNBOOST_THREAD_FWD_REF(Fp) f) {
      shared_ptr<future_executor_shared_state<Rp, Executor> >
          h(new future_executor_shared_state<Rp, Executor>(ex, ndnboost::forward<Fp>(f)));
      return NDNBOOST_THREAD_FUTURE<Rp>(h);
    }

} // detail

    ////////////////////////////////
    // template <class Executor, class F, class... ArgTypes>
    // future<R> async(Executor& ex, F&&, ArgTypes&&...);
    ////////////////////////////////

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
#if defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

  template <class Executor, class R, class... ArgTypes>
  NDNBOOST_THREAD_FUTURE<R>
  async(Executor& ex, R(*f)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...), NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
    typedef R(*F)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...);
    typedef detail::invoker<typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            thread_detail::decay_copy(ndnboost::forward<F>(f))
            , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
        )
    ));
  }
#endif // defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

  template <class Executor, class F, class ...ArgTypes>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
      typename decay<ArgTypes>::type...
  )>::type>
  async(Executor& ex, NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
    typedef detail::invoker<typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            thread_detail::decay_copy(ndnboost::forward<F>(f))
            , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
        )
    ));
  }

//  template <class R, class Executor, class F, class ...ArgTypes>
//  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
//      typename decay<ArgTypes>::type...
//  )>::type>
//  async(Executor& ex, NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
//    typedef detail::invoker_ret<R, typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
//    typedef typename BF::result_type Rp;
//
//    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
//        BF(
//            thread_detail::decay_copy(ndnboost::forward<F>(f))
//            , thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
//        )
//    ));
//  }

#else // ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
#if defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

  template <class Executor, class R>
  NDNBOOST_THREAD_FUTURE<R>
  async(Executor& ex, R(*f)()) {
    typedef R(*F)();
    typedef detail::invoker<F> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            f
        )
    ));
  }

  template <class Executor, class R, class A1>
  NDNBOOST_THREAD_FUTURE<R>
  async(Executor& ex, R(*f)(NDNBOOST_THREAD_FWD_REF(A1)), NDNBOOST_THREAD_FWD_REF(A1) a1) {
    typedef R(*F)(NDNBOOST_THREAD_FWD_REF(A1));
    typedef detail::invoker<F, typename decay<A1>::type> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            f
            , thread_detail::decay_copy(ndnboost::forward<A1>(a1))
        )
    ));
  }
#endif // defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

  template <class Executor, class F>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type()>::type>
  async(Executor& ex, NDNBOOST_THREAD_FWD_REF(F) f)  {
    typedef detail::invoker<typename decay<F>::type> BF;
    typedef typename BF::result_type Rp;

    return ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            thread_detail::decay_copy(ndnboost::forward<F>(f))
        )
    );
  }

  template <class Executor, class F, class A1>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
      typename decay<A1>::type
  )>::type>
  async(Executor& ex, NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(A1) a1) {
    typedef detail::invoker<typename decay<F>::type, typename decay<A1>::type> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            thread_detail::decay_copy(ndnboost::forward<F>(f))
          , thread_detail::decay_copy(ndnboost::forward<A1>(a1))
        )
    ));
  }

  template <class Executor, class F, class A1, class A2>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
      typename decay<A1>::type, typename decay<A2>::type
  )>::type>
  async(Executor& ex, NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(A1) a1, NDNBOOST_THREAD_FWD_REF(A2) a2) {
    typedef detail::invoker<typename decay<F>::type, typename decay<A1>::type, typename decay<A2>::type> BF;
    typedef typename BF::result_type Rp;

    return NDNBOOST_THREAD_MAKE_RV_REF(ndnboost::detail::make_future_executor_shared_state<Rp>(ex,
        BF(
            thread_detail::decay_copy(ndnboost::forward<F>(f))
          , thread_detail::decay_copy(ndnboost::forward<A1>(a1))
          , thread_detail::decay_copy(ndnboost::forward<A2>(a2))
        )
    ));
  }

#endif //! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
#endif

  ////////////////////////////////
  // template <class F, class... ArgTypes>
  // future<R> async(F&&, ArgTypes&&...);
  ////////////////////////////////

#if defined NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR
  #if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
  template <class R, class... ArgTypes>
  NDNBOOST_THREAD_FUTURE<R>
  async(R(*f)(NDNBOOST_THREAD_FWD_REF(ArgTypes)...), NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
    return NDNBOOST_THREAD_MAKE_RV_REF(async(launch(launch::any), f, ndnboost::forward<ArgTypes>(args)...));
  }
  #else
  template <class R>
  NDNBOOST_THREAD_FUTURE<R>
  async(R(*f)()) {
    return NDNBOOST_THREAD_MAKE_RV_REF(async(launch(launch::any), f));
  }
  #endif
#endif

#if defined(NDNBOOST_THREAD_PROVIDES_VARIADIC_THREAD)
  template <class F, class ...ArgTypes>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<typename decay<F>::type(
      typename decay<ArgTypes>::type...
  )>::type>
  async(NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(ArgTypes)... args) {
      return NDNBOOST_THREAD_MAKE_RV_REF(async(launch(launch::any), ndnboost::forward<F>(f), ndnboost::forward<ArgTypes>(args)...));
  }
#else
  template <class F>
  NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F()>::type>
  async(NDNBOOST_THREAD_RV_REF(F) f) {
      return NDNBOOST_THREAD_MAKE_RV_REF(async(launch(launch::any), ndnboost::forward<F>(f)));
  }
#endif

  ////////////////////////////////
  // make_future deprecated
  ////////////////////////////////
  template <typename T>
  NDNBOOST_THREAD_FUTURE<typename decay<T>::type> make_future(NDNBOOST_THREAD_FWD_REF(T) value) {
    typedef typename decay<T>::type future_value_type;
    promise<future_value_type> p;
    p.set_value(ndnboost::forward<future_value_type>(value));
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

#if defined NDNBOOST_THREAD_USES_MOVE
  inline NDNBOOST_THREAD_FUTURE<void> make_future() {
    promise<void> p;
    p.set_value();
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }
#endif

  ////////////////////////////////
  // make_ready_future
  ////////////////////////////////
  template <typename T>
  NDNBOOST_THREAD_FUTURE<typename decay<T>::type> make_ready_future(NDNBOOST_THREAD_FWD_REF(T) value) {
    typedef typename decay<T>::type future_value_type;
    promise<future_value_type> p;
    p.set_value(ndnboost::forward<future_value_type>(value));
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

  template <typename T, typename T1>
  NDNBOOST_THREAD_FUTURE<T> make_ready_no_decay_future(T1 value) {
    typedef T future_value_type;
    promise<future_value_type> p;
    p.set_value(value);
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

#if defined NDNBOOST_THREAD_USES_MOVE
  inline NDNBOOST_THREAD_FUTURE<void> make_ready_future() {
    promise<void> p;
    p.set_value();
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }
#endif

  template <typename T>
  NDNBOOST_THREAD_FUTURE<T> make_ready_future(exception_ptr ex)  {
    promise<T> p;
    p.set_exception(ex);
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }
//  template <typename T, typename E>
//  NDNBOOST_THREAD_FUTURE<T> make_ready_future(E ex)
//  {
//    promise<T> p;
//    p.set_exception(ndnboost::copy_exception(ex));
//    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
//  }

  template <typename T>
  NDNBOOST_THREAD_FUTURE<T> make_exceptional_future(exception_ptr ex) {
    promise<T> p;
    p.set_exception(ex);
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

  template <typename T, typename E>
  NDNBOOST_THREAD_FUTURE<T> make_exceptional_future(E ex) {
    promise<T> p;
    p.set_exception(ndnboost::copy_exception(ex));
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

  template <typename T>
  NDNBOOST_THREAD_FUTURE<T> make_exceptional_future() {
    promise<T> p;
    p.set_exception(ndnboost::current_exception());
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }

#if 0
  template<typename CLOSURE>
  make_future(CLOSURE closure) -> NDNBOOST_THREAD_FUTURE<decltype(closure())> {
      typedef decltype(closure()) T;
      promise<T> p;
      try {
        p.set_value(closure());
      } catch(...) {
        p.set_exception(std::current_exception());
      }
      return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future());
  }
#endif

  ////////////////////////////////
  // make_shared_future deprecated
  ////////////////////////////////
  template <typename T>
  shared_future<typename decay<T>::type> make_shared_future(NDNBOOST_THREAD_FWD_REF(T) value) {
    typedef typename decay<T>::type future_type;
    promise<future_type> p;
    p.set_value(ndnboost::forward<T>(value));
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future().share());
  }

  inline shared_future<void> make_shared_future()  {
    promise<void> p;
    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future().share());
  }

//  ////////////////////////////////
//  // make_ready_shared_future
//  ////////////////////////////////
//  template <typename T>
//  shared_future<typename decay<T>::type> make_ready_shared_future(NDNBOOST_THREAD_FWD_REF(T) value)
//  {
//    typedef typename decay<T>::type future_type;
//    promise<future_type> p;
//    p.set_value(ndnboost::forward<T>(value));
//    return p.get_future().share();
//  }
//
//
//  inline shared_future<void> make_ready_shared_future()
//  {
//    promise<void> p;
//    return NDNBOOST_THREAD_MAKE_RV_REF(p.get_future().share());
//
//  }
//
//  ////////////////////////////////
//  // make_exceptional_shared_future
//  ////////////////////////////////
//  template <typename T>
//  shared_future<T> make_exceptional_shared_future(exception_ptr ex)
//  {
//    promise<T> p;
//    p.set_exception(ex);
//    return p.get_future().share();
//  }

  ////////////////////////////////
  // detail::future_async_continuation_shared_state
  ////////////////////////////////
#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
namespace detail
{

  /////////////////////////
  /// future_async_continuation_shared_state
  /////////////////////////

  template<typename F, typename Rp, typename Fp>
  struct future_async_continuation_shared_state: future_async_shared_state_base<Rp>
  {
    F parent;
    Fp continuation;

  public:
    future_async_continuation_shared_state(NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c)
    : parent(ndnboost::move(f)),
      continuation(ndnboost::move(c)) {
    }

    void launch_continuation(ndnboost::unique_lock<ndnboost::mutex>& ) {
      //lock.unlock();
      this->thr_ = thread(&future_async_continuation_shared_state::run, this);
    }

    static void run(future_async_continuation_shared_state* that) {
      try {
        that->mark_finished_with_result(that->continuation(ndnboost::move(that->parent)));
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
      } catch(thread_interrupted& ) {
        that->mark_interrupted_finish();
#endif
      } catch(...) {
        that->mark_exceptional_finish();
      }
    }

    ~future_async_continuation_shared_state() {
      this->join();
    }
  };

  template<typename F, typename Fp>
  struct future_async_continuation_shared_state<F, void, Fp>: public future_async_shared_state_base<void>
  {
    F parent;
    Fp continuation;

  public:
    future_async_continuation_shared_state(NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c)
    : parent(ndnboost::move(f)),
      continuation(ndnboost::move(c)) {
    }

    void launch_continuation(ndnboost::unique_lock<ndnboost::mutex>& ) {
      //lk.unlock();
      this->thr_ = thread(&future_async_continuation_shared_state::run, this);
    }

    static void run(future_async_continuation_shared_state* that) {
      try {
        that->continuation(ndnboost::move(that->parent));
        that->mark_finished_with_result();
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
      } catch(thread_interrupted& ) {
        that->mark_interrupted_finish();
#endif
      } catch(...) {
        that->mark_exceptional_finish();
      }
    }

    ~future_async_continuation_shared_state() {
      this->join();
    }
  };

  //////////////////////////
  /// future_deferred_continuation_shared_state
  //////////////////////////
  template<typename F, typename Rp, typename Fp>
  struct future_deferred_continuation_shared_state: shared_state<Rp>
  {
    F parent;
    Fp continuation;

  public:
    future_deferred_continuation_shared_state(NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c)
    : parent(ndnboost::move(f)),
      continuation(ndnboost::move(c)) {
      this->set_deferred();
    }

    virtual void launch_continuation(ndnboost::unique_lock<ndnboost::mutex>& ) {
      //execute(lk);
    }

    virtual void execute(ndnboost::unique_lock<ndnboost::mutex>& lck) {
      try {
        Fp local_fuct=ndnboost::move(continuation);
        F ftmp = ndnboost::move(parent);
        relocker relock(lck);
        Rp res = local_fuct(ndnboost::move(ftmp));
        relock.lock();
        this->mark_finished_with_result_internal(ndnboost::move(res), lck);
      } catch (...) {
        this->mark_exceptional_finish_internal(current_exception(), lck);
      }
    }
  };

  template<typename F, typename Fp>
  struct future_deferred_continuation_shared_state<F,void,Fp>: shared_state<void>
  {
    F parent;
    Fp continuation;

  public:
    future_deferred_continuation_shared_state(NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c)
    : parent(ndnboost::move(f)),
      continuation(ndnboost::move(c)) {
      this->set_deferred();
    }

    virtual void launch_continuation(ndnboost::unique_lock<ndnboost::mutex>& ) {
      //execute(lk);
    }
    virtual void execute(ndnboost::unique_lock<ndnboost::mutex>& lck) {
      try {
        Fp local_fuct=ndnboost::move(continuation);
        F ftmp = ndnboost::move(parent);
        relocker relock(lck);
        local_fuct(ndnboost::move(ftmp));
        relock.lock();
        this->mark_finished_with_result_internal(lck);
      } catch (...) {
        this->mark_exceptional_finish_internal(current_exception(), lck);
      }
    }
  };

  ////////////////////////////////
  // make_future_deferred_continuation_shared_state
  ////////////////////////////////
  template<typename F, typename Rp, typename Fp>
  NDNBOOST_THREAD_FUTURE<Rp>
  make_future_deferred_continuation_shared_state(
      ndnboost::unique_lock<ndnboost::mutex> &lock,
      NDNBOOST_THREAD_RV_REF(F) f, NDNBOOST_THREAD_FWD_REF(Fp) c) {
    shared_ptr<future_deferred_continuation_shared_state<F, Rp, Fp> >
        h(new future_deferred_continuation_shared_state<F, Rp, Fp>(ndnboost::move(f), ndnboost::forward<Fp>(c)));
    h->parent.future_->set_continuation_ptr(h, lock);
    return NDNBOOST_THREAD_FUTURE<Rp>(h);
  }

  ////////////////////////////////
  // make_future_async_continuation_shared_state
  ////////////////////////////////
  template<typename F, typename Rp, typename Fp>
  NDNBOOST_THREAD_FUTURE<Rp>
  make_future_async_continuation_shared_state(
      ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f,
      NDNBOOST_THREAD_FWD_REF(Fp) c) {
    shared_ptr<future_async_continuation_shared_state<F,Rp, Fp> >
        h(new future_async_continuation_shared_state<F,Rp, Fp>(ndnboost::move(f), ndnboost::forward<Fp>(c)));
    h->parent.future_->set_continuation_ptr(h, lock);

    return NDNBOOST_THREAD_FUTURE<Rp>(h);
  }
}

  ////////////////////////////////
  // template<typename F>
  // auto future<R>::then(F&& func) -> NDNBOOST_THREAD_FUTURE<decltype(func(*this))>;
  ////////////////////////////////

  template <typename R>
  template <typename F>
  inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE<R>)>::type>
  NDNBOOST_THREAD_FUTURE<R>::then(launch policy, NDNBOOST_THREAD_FWD_REF(F) func) {
    typedef typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE<R>)>::type future_type;
    NDNBOOST_THREAD_ASSERT_PRECONDITION(this->future_!=0, future_uninitialized());

    ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
    if (underlying_cast<int>(policy) & int(launch::async)) {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_async_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_deferred_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    } else {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_async_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    }
  }

  template <typename R>
  template <typename F>
  inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE<R>)>::type>
  NDNBOOST_THREAD_FUTURE<R>::then(NDNBOOST_THREAD_FWD_REF(F) func)  {
    typedef typename ndnboost::result_of<F(NDNBOOST_THREAD_FUTURE<R>)>::type future_type;
    NDNBOOST_THREAD_ASSERT_PRECONDITION(this->future_!=0, future_uninitialized());

    ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
    if (underlying_cast<int>(this->launch_policy(lock)) & int(launch::async)) {
      return ndnboost::detail::make_future_async_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func)
      );
    } else if (underlying_cast<int>(this->launch_policy(lock)) & int(launch::deferred)) {
      this->future_->wait_internal(lock);
      return ndnboost::detail::make_future_deferred_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func)
      );
    } else {
      return ndnboost::detail::make_future_async_continuation_shared_state<NDNBOOST_THREAD_FUTURE<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func)
      );
    }
  }


//#if 0 && defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
//  template <typename R>
//  template<typename RF>
//  NDNBOOST_THREAD_FUTURE<RF>
//  NDNBOOST_THREAD_FUTURE<R>::then(RF(*func)(NDNBOOST_THREAD_FUTURE<R>&))
//  {
//
//    typedef RF future_type;
//
//    if (this->future_)
//    {
//      ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
//      detail::future_continuation<NDNBOOST_THREAD_FUTURE<R>, future_type, RF(*)(NDNBOOST_THREAD_FUTURE&) > *ptr =
//          new detail::future_continuation<NDNBOOST_THREAD_FUTURE<R>, future_type, RF(*)(NDNBOOST_THREAD_FUTURE&)>(*this, func);
//      if (ptr==0)
//      {
//        return NDNBOOST_THREAD_MAKE_RV_REF(NDNBOOST_THREAD_FUTURE<future_type>());
//      }
//      this->future_->set_continuation_ptr(ptr, lock);
//      return ptr->get_future();
//    } else {
//      // fixme what to do when the future has no associated state?
//      return NDNBOOST_THREAD_MAKE_RV_REF(NDNBOOST_THREAD_FUTURE<future_type>());
//    }
//
//  }
//  template <typename R>
//  template<typename RF>
//  NDNBOOST_THREAD_FUTURE<RF>
//  NDNBOOST_THREAD_FUTURE<R>::then(launch policy, RF(*func)(NDNBOOST_THREAD_FUTURE<R>&))
//  {
//
//    typedef RF future_type;
//
//    if (this->future_)
//    {
//      ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
//      detail::future_continuation<NDNBOOST_THREAD_FUTURE<R>, future_type, RF(*)(NDNBOOST_THREAD_FUTURE&) > *ptr =
//          new detail::future_continuation<NDNBOOST_THREAD_FUTURE<R>, future_type, RF(*)(NDNBOOST_THREAD_FUTURE&)>(*this, func, policy);
//      if (ptr==0)
//      {
//        return NDNBOOST_THREAD_MAKE_RV_REF(NDNBOOST_THREAD_FUTURE<future_type>());
//      }
//      this->future_->set_continuation_ptr(ptr, lock);
//      return ptr->get_future();
//    } else {
//      // fixme what to do when the future has no associated state?
//      return NDNBOOST_THREAD_MAKE_RV_REF(NDNBOOST_THREAD_FUTURE<future_type>());
//    }
//
//  }
//#endif

  template <typename R>
  template <typename F>
  inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(shared_future<R>)>::type>
  shared_future<R>::then(launch policy, NDNBOOST_THREAD_FWD_REF(F) func)
  {
    typedef typename ndnboost::result_of<F(shared_future<R>)>::type future_type;
    NDNBOOST_THREAD_ASSERT_PRECONDITION(this->future_!=0, future_uninitialized());

    ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
    if (underlying_cast<int>(policy) & int(launch::async)) {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_async_continuation_shared_state<shared_future<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    } else if (underlying_cast<int>(policy) & int(launch::deferred)) {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_deferred_continuation_shared_state<shared_future<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    } else {
      return NDNBOOST_THREAD_MAKE_RV_REF((ndnboost::detail::make_future_async_continuation_shared_state<shared_future<R>, future_type, F>(
                  lock, ndnboost::move(*this), ndnboost::forward<F>(func)
              )));
    }
  }

  template <typename R>
  template <typename F>
  inline NDNBOOST_THREAD_FUTURE<typename ndnboost::result_of<F(shared_future<R>)>::type>
  shared_future<R>::then(NDNBOOST_THREAD_FWD_REF(F) func) {
    typedef typename ndnboost::result_of<F(shared_future<R>)>::type future_type;

    NDNBOOST_THREAD_ASSERT_PRECONDITION(this->future_!=0, future_uninitialized());

    ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
    if (underlying_cast<int>(this->launch_policy(lock)) & int(launch::async)) {
      return ndnboost::detail::make_future_async_continuation_shared_state<shared_future<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func));
    } else if (underlying_cast<int>(this->launch_policy(lock)) & int(launch::deferred)) {
      this->future_->wait_internal(lock);
      return ndnboost::detail::make_future_deferred_continuation_shared_state<shared_future<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func));
    } else {
      return ndnboost::detail::make_future_async_continuation_shared_state<shared_future<R>, future_type, F>(
          lock, ndnboost::move(*this), ndnboost::forward<F>(func));
    }
  }

namespace detail
{
  template <typename T>
  struct mfallbacker_to
  {
    T value_;
    typedef T result_type;
    mfallbacker_to(NDNBOOST_THREAD_RV_REF(T) v)
    : value_(ndnboost::move(v))
    {}

    T operator()(NDNBOOST_THREAD_FUTURE<T> fut) {
      return fut.get_or(ndnboost::move(value_));
    }
  };
  template <typename T>
  struct cfallbacker_to
  {
    T value_;
    typedef T result_type;
    cfallbacker_to(T const& v)
    : value_(v)
    {}

    T operator()(NDNBOOST_THREAD_FUTURE<T> fut) {
      return fut.get_or(value_);

    }
  };
}
  ////////////////////////////////
  // future<R> future<R>::fallback_to(R&& v);
  ////////////////////////////////

  template <typename R>
  template <typename R2>
  inline typename ndnboost::disable_if< is_void<R2>, NDNBOOST_THREAD_FUTURE<R> >::type
  NDNBOOST_THREAD_FUTURE<R>::fallback_to(NDNBOOST_THREAD_RV_REF(R2) v) {
    return then(detail::mfallbacker_to<R>(ndnboost::move(v)));
  }

  template <typename R>
  template <typename R2>
  inline typename ndnboost::disable_if< is_void<R2>, NDNBOOST_THREAD_FUTURE<R> >::type
  NDNBOOST_THREAD_FUTURE<R>::fallback_to(R2 const& v) {
    return then(detail::cfallbacker_to<R>(v));
  }

#endif

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_UNWRAP
namespace detail
{
  /////////////////////////
  /// future_unwrap_shared_state
  /////////////////////////

  template<typename F, typename Rp>
  struct future_unwrap_shared_state: shared_state<Rp>
  {
    F parent;
  public:
    explicit future_unwrap_shared_state(NDNBOOST_THREAD_RV_REF(F) f)
    : parent(ndnboost::move(f)) {}
    virtual void wait(bool ) { // todo see if rethrow must be used
        ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
        parent.get().wait();
    }
    virtual Rp get() {
        ndnboost::unique_lock<ndnboost::mutex> lock(mutex);
        return parent.get().get();
    }
  };

  template <class F, class Rp>
  NDNBOOST_THREAD_FUTURE<Rp>
  make_future_unwrap_shared_state(ndnboost::unique_lock<ndnboost::mutex> &lock, NDNBOOST_THREAD_RV_REF(F) f) {
    shared_ptr<future_unwrap_shared_state<F, Rp> >
        h(new future_unwrap_shared_state<F, Rp>(ndnboost::move(f)));
    h->parent.future_->set_continuation_ptr(h, lock);
    return NDNBOOST_THREAD_FUTURE<Rp>(h);
  }
}

  template <typename R>
  inline NDNBOOST_THREAD_FUTURE<R>::NDNBOOST_THREAD_FUTURE(NDNBOOST_THREAD_RV_REF(NDNBOOST_THREAD_FUTURE<NDNBOOST_THREAD_FUTURE<R> >) other)
  : base_type(other.unwrap()) {}

  template <typename R2>
  NDNBOOST_THREAD_FUTURE<R2>
  NDNBOOST_THREAD_FUTURE<NDNBOOST_THREAD_FUTURE<R2> >::unwrap()
  {
    NDNBOOST_THREAD_ASSERT_PRECONDITION(this->future_!=0, future_uninitialized());
    ndnboost::unique_lock<ndnboost::mutex> lock(this->future_->mutex);
    return ndnboost::detail::make_future_unwrap_shared_state<NDNBOOST_THREAD_FUTURE<NDNBOOST_THREAD_FUTURE<R2> >, R2>(lock, ndnboost::move(*this));
  }
#endif

#if defined NDNBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
namespace detail
{
  struct input_iterator_tag {};
  struct vector_tag {};
  struct values_tag {};
  template <typename T>
  struct alias_t { typedef T type; };

  NDNBOOST_CONSTEXPR_OR_CONST input_iterator_tag input_iterator_tag_value = {};
  NDNBOOST_CONSTEXPR_OR_CONST vector_tag vector_tag_value = {};
  NDNBOOST_CONSTEXPR_OR_CONST values_tag values_tag_value = {};
  ////////////////////////////////
  // detail::future_async_when_all_shared_state
  ////////////////////////////////
  template<typename F>
  struct future_when_all_vector_shared_state: future_async_shared_state_base<csbl::vector<F> >
  {
    typedef csbl::vector<F> vector_type;
    typedef typename F::value_type value_type;
    csbl::vector<F> vec_;

    static void run(future_when_all_vector_shared_state* that) {
      try {
        ndnboost::wait_for_all(that->vec_.begin(), that->vec_.end());
        that->mark_finished_with_result(ndnboost::move(that->vec_));
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
      } catch(thread_interrupted& ) {
        that->mark_interrupted_finish();
#endif
      } catch(...) {
        that->mark_exceptional_finish();
      }
    }
    void init() {
      this->thr_ = thread(&future_when_all_vector_shared_state::run, this);
    }

  public:
    template< typename InputIterator>
    future_when_all_vector_shared_state(input_iterator_tag, InputIterator first, InputIterator last)
    : vec_(std::make_move_iterator(first), std::make_move_iterator(last))
    {
      init();
    }

    future_when_all_vector_shared_state(vector_tag, NDNBOOST_THREAD_RV_REF(csbl::vector<F>) v)
    : vec_(ndnboost::move(v))
    {
      init();
    }

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
    template< typename T0, typename ...T>
    future_when_all_vector_shared_state(values_tag, NDNBOOST_THREAD_RV_REF(T0) f, NDNBOOST_THREAD_RV_REF(T) ... futures) {
      vec_.push_back(ndnboost::forward<T0>(f));
      typename alias_t<char[]>::type{
          ( //first part of magic unpacker
          vec_.push_back(ndnboost::forward<T>(futures)),'0'
          )..., '0'
      }; //second part of magic unpacker
      init();
    }
#else
#endif
    ~future_when_all_vector_shared_state() {
      this->join();
    }

  };

  ////////////////////////////////
  // detail::future_async_when_any_shared_state
  ////////////////////////////////
  template<typename F>
  struct future_when_any_vector_shared_state: future_async_shared_state_base<csbl::vector<F> >
  {
    typedef csbl::vector<F> vector_type;
    typedef typename F::value_type value_type;
    csbl::vector<F> vec_;

    static void run(future_when_any_vector_shared_state* that)
    {
      try {
        ndnboost::wait_for_any(that->vec_.begin(), that->vec_.end());
        that->mark_finished_with_result(ndnboost::move(that->vec_));
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
      } catch(thread_interrupted& ) {
        that->mark_interrupted_finish();
#endif
      } catch(...) {
        that->mark_exceptional_finish();
      }
    }
    void init() {
      this->thr_ = thread(&future_when_any_vector_shared_state::run, this);
    }

  public:
    template< typename InputIterator>
    future_when_any_vector_shared_state(input_iterator_tag, InputIterator first, InputIterator last)
    : vec_(std::make_move_iterator(first), std::make_move_iterator(last))
    {
      init();
    }

    future_when_any_vector_shared_state(vector_tag, NDNBOOST_THREAD_RV_REF(csbl::vector<F>) v)
    : vec_(ndnboost::move(v))
    {
      init();
    }

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
    template< typename T0, typename ...T>
    future_when_any_vector_shared_state(values_tag,
        NDNBOOST_THREAD_RV_REF(T0) f, NDNBOOST_THREAD_RV_REF(T) ... futures
    ) {
      vec_.push_back(ndnboost::forward<T0>(f));
      typename alias_t<char[]>::type{
          ( //first part of magic unpacker
          vec_.push_back(ndnboost::forward<T>(futures))
          ,'0'
          )...,
          '0'
      }; //second part of magic unpacker
      init();
    }
#endif

    ~future_when_any_vector_shared_state()    {
      this->join();
    }

  };

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
//#if ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)
  template< typename T0, typename ...T>
  struct future_when_all_tuple_shared_state: future_async_shared_state_base<
    csbl::tuple<NDNBOOST_THREAD_FUTURE<typename T0::value_type>, NDNBOOST_THREAD_FUTURE<typename T::value_type>... >
  >
  {

  };
  template< typename T0, typename ...T>
  struct future_when_any_tuple_shared_state: future_async_shared_state_base<
    csbl::tuple<NDNBOOST_THREAD_FUTURE<typename T0::value_type>, NDNBOOST_THREAD_FUTURE<typename T::value_type>... >
  >
  {
  };
//#endif
#endif

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
  template< typename ...T>
  struct are_same : true_type {};
  template< typename T0 >
  struct are_same<T0> : true_type {};
  template< typename T0, typename T1, typename ...T>
  struct are_same<T0, T1, T...> : integral_constant<bool, is_same<T0,T1>::value && are_same<T1, T...>::value> {};

  template< bool AreSame, typename T0, typename ...T>
  struct when_type_impl;

  template< typename T0, typename ...T>
  struct when_type_impl<true, T0, T...>
  {
    typedef csbl::vector<typename decay<T0>::type> container_type;
    typedef typename container_type::value_type value_type;
    typedef detail::future_when_all_vector_shared_state<value_type> factory_all_type;
    typedef detail::future_when_any_vector_shared_state<value_type> factory_any_type;
  };
//#if ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)
  template< typename T0, typename ...T>
  struct when_type_impl<false, T0, T...>
  {
    typedef csbl::tuple<NDNBOOST_THREAD_FUTURE<typename T0::value_type>, NDNBOOST_THREAD_FUTURE<typename T::value_type>... > container_type;
    typedef detail::future_when_all_tuple_shared_state<T0, T...> factory_all_type;
    typedef detail::future_when_any_tuple_shared_state<T0, T...> factory_any_type;
  };
//#endif

  template< typename T0, typename ...T>
  struct when_type : when_type_impl<are_same<T0, T...>::value, T0, T...> {};
#endif
}

  template< typename InputIterator>
  typename ndnboost::disable_if<is_future_type<InputIterator>,
    NDNBOOST_THREAD_FUTURE<csbl::vector<typename InputIterator::value_type>  >
  >::type
  when_all(InputIterator first, InputIterator last) {
    typedef  typename InputIterator::value_type value_type;
    typedef  csbl::vector<value_type> container_type;
    typedef  detail::future_when_all_vector_shared_state<value_type> factory_type;

    if (first==last) return make_ready_future(container_type());
    shared_ptr<factory_type >
        h(new factory_type>(detail::input_iterator_tag_value, first,last));
    return NDNBOOST_THREAD_FUTURE<container_type>(h);
  }

//#if ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)
  inline NDNBOOST_THREAD_FUTURE<csbl::tuple<> > when_all() {
    return make_ready_future(csbl::tuple<>());
  }
//#endif

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
  template< typename T0, typename ...T>
  NDNBOOST_THREAD_FUTURE<typename detail::when_type<T0, T...>::container_type>
  when_all(NDNBOOST_THREAD_RV_REF(T0) f, NDNBOOST_THREAD_RV_REF(T) ... futures) {
    typedef  typename detail::when_type<T0, T...>::container_type container_type;
    typedef  typename detail::when_type<T0, T...>::factory_all_type factory_type;

    shared_ptr<factory_type>
        h(new factory_type(detail::values_tag_value, ndnboost::forward<T0>(f), ndnboost::forward<T>(futures)...));
    return NDNBOOST_THREAD_FUTURE<container_type>(h);
  }
#endif

  template< typename InputIterator>
  typename ndnboost::disable_if<is_future_type<InputIterator>,
    NDNBOOST_THREAD_FUTURE<csbl::vector<typename InputIterator::value_type>  >
  >::type
  when_any(InputIterator first, InputIterator last) {
    typedef  typename InputIterator::value_type value_type;
    typedef  csbl::vector<value_type> container_type;
    typedef  detail::future_when_any_vector_shared_state<value_type> factory_type;

    if (first==last) return make_ready_future(container_type());
    shared_ptr<factory_type >
        h(new factory_type>(detail::input_iterator_tag_value, first,last));
    return NDNBOOST_THREAD_FUTURE<container_type>(h);
  }

//#if ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)
  inline NDNBOOST_THREAD_FUTURE<csbl::tuple<> > when_any() {
    return make_ready_future(csbl::tuple<>());
  }
//#endif

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
  template< typename T0, typename ...T>
  NDNBOOST_THREAD_FUTURE<typename detail::when_type<T0, T...>::container_type>
  when_any(NDNBOOST_THREAD_RV_REF(T0) f, NDNBOOST_THREAD_RV_REF(T) ... futures) {
    typedef  typename detail::when_type<T0, T...>::container_type container_type;
    typedef  typename detail::when_type<T0, T...>::factory_any_type factory_type;

    shared_ptr<factory_type>
        h(new factory_type(detail::values_tag_value, ndnboost::forward<T0>(f), ndnboost::forward<T>(futures)...));
    return NDNBOOST_THREAD_FUTURE<container_type>(h);
  }
#endif
#endif // NDNBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
}

#endif // NDNBOOST_NO_EXCEPTION
#endif // header
