#ifndef NDNBOOST_THREAD_WIN32_ONCE_HPP
#define NDNBOOST_THREAD_WIN32_ONCE_HPP

//  once.hpp
//
//  (C) Copyright 2005-7 Anthony Williams
//  (C) Copyright 2005 John Maddock
//  (C) Copyright 2011-2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include <cstddef>
#include <ndnboost/assert.hpp>
#include <ndnboost/static_assert.hpp>
#include <ndnboost/detail/interlocked.hpp>
#include <ndnboost/thread/win32/thread_primitives.hpp>
#include <ndnboost/thread/win32/interlocked_read.hpp>
#include <ndnboost/core/no_exceptions_support.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/invoke.hpp>

#include <ndnboost/bind.hpp>

#include <ndnboost/config/abi_prefix.hpp>

#ifdef NDNBOOST_NO_STDC_NAMESPACE
namespace std
{
    using ::memcpy;
    using ::ptrdiff_t;
}
#endif

namespace ndnboost
{
  struct once_flag;
  namespace detail
  {
  struct once_context;

  inline bool enter_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT;
  inline void commit_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT;
  inline void rollback_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT;
  }

#ifdef NDNBOOST_THREAD_PROVIDES_ONCE_CXX11

  struct once_flag
  {
      NDNBOOST_THREAD_NO_COPYABLE(once_flag)
      NDNBOOST_CONSTEXPR once_flag() NDNBOOST_NOEXCEPT
        : status(0), count(0)
      {}
      long status;
      long count;
  private:
      friend inline bool enter_once_region(once_flag& flag, detail::once_context& ctx) NDNBOOST_NOEXCEPT;
      friend inline void commit_once_region(once_flag& flag, detail::once_context& ctx) NDNBOOST_NOEXCEPT;
      friend inline void rollback_once_region(once_flag& flag, detail::once_context& ctx) NDNBOOST_NOEXCEPT;
  };

#define NDNBOOST_ONCE_INIT once_flag()
#else // NDNBOOST_THREAD_PROVIDES_ONCE_CXX11

    struct once_flag
    {
        long status;
        long count;
    };

#define NDNBOOST_ONCE_INIT {0,0}
#endif  // NDNBOOST_THREAD_PROVIDES_ONCE_CXX11

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

    namespace detail
    {
#ifdef NDNBOOST_NO_ANSI_APIS
        typedef wchar_t once_char_type;
#else
        typedef char once_char_type;
#endif
        unsigned const once_mutex_name_fixed_length=54;
        unsigned const once_mutex_name_length=once_mutex_name_fixed_length+
            sizeof(void*)*2+sizeof(unsigned long)*2+1;

        template <class I>
        void int_to_string(I p, once_char_type* buf)
        {
            for(unsigned i=0; i < sizeof(I)*2; ++i,++buf)
            {
#ifdef NDNBOOST_NO_ANSI_APIS
                once_char_type const a=L'A';
#else
                once_char_type const a='A';
#endif
                *buf = a + static_cast<once_char_type>((p >> (i*4)) & 0x0f);
            }
            *buf = 0;
        }

        inline void name_once_mutex(once_char_type* mutex_name,void* flag_address)
        {
#ifdef NDNBOOST_NO_ANSI_APIS
            static const once_char_type fixed_mutex_name[]=L"Local\\{C15730E2-145C-4c5e-B005-3BC753F42475}-once-flag";
#else
            static const once_char_type fixed_mutex_name[]="Local\\{C15730E2-145C-4c5e-B005-3BC753F42475}-once-flag";
#endif
            NDNBOOST_STATIC_ASSERT(sizeof(fixed_mutex_name) ==
                                (sizeof(once_char_type)*(once_mutex_name_fixed_length+1)));

            std::memcpy(mutex_name,fixed_mutex_name,sizeof(fixed_mutex_name));
            detail::int_to_string(reinterpret_cast<std::ptrdiff_t>(flag_address),
                                  mutex_name + once_mutex_name_fixed_length);
            detail::int_to_string(win32::GetCurrentProcessId(),
                                  mutex_name + once_mutex_name_fixed_length + sizeof(void*)*2);
        }

        inline void* open_once_event(once_char_type* mutex_name,void* flag_address)
        {
            if(!*mutex_name)
            {
                name_once_mutex(mutex_name,flag_address);
            }

#ifdef NDNBOOST_NO_ANSI_APIS
            return ::ndnboost::detail::win32::OpenEventW(
#else
            return ::ndnboost::detail::win32::OpenEventA(
#endif
                ::ndnboost::detail::win32::synchronize |
                ::ndnboost::detail::win32::event_modify_state,
                false,
                mutex_name);
        }

        inline void* create_once_event(once_char_type* mutex_name,void* flag_address)
        {
            if(!*mutex_name)
            {
                name_once_mutex(mutex_name,flag_address);
            }
#ifdef NDNBOOST_NO_ANSI_APIS
            return ::ndnboost::detail::win32::CreateEventW(
#else
            return ::ndnboost::detail::win32::CreateEventA(
#endif
                0,::ndnboost::detail::win32::manual_reset_event,
                ::ndnboost::detail::win32::event_initially_reset,
                mutex_name);
        }

        struct once_context {
          long const function_complete_flag_value;
          long const running_value;
          bool counted;
          detail::win32::handle_manager event_handle;
          detail::once_char_type mutex_name[once_mutex_name_length];
          once_context() :
            function_complete_flag_value(0xc15730e2),
            running_value(0x7f0725e3),
            counted(false)
          {
            mutex_name[0]=0;
          }
        };
        enum once_action {try_, break_, continue_};

        inline bool enter_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT
        {
          long status=NDNBOOST_INTERLOCKED_COMPARE_EXCHANGE(&flag.status,ctx.running_value,0);
          if(!status)
          {
            if(!ctx.event_handle)
            {
                ctx.event_handle=detail::open_once_event(ctx.mutex_name,&flag);
            }
            if(ctx.event_handle)
            {
                ::ndnboost::detail::win32::ResetEvent(ctx.event_handle);
            }
            return true;
          }
          return false;
        }
        inline void commit_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT
        {
          if(!ctx.counted)
          {
              NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
              ctx.counted=true;
          }
          NDNBOOST_INTERLOCKED_EXCHANGE(&flag.status,ctx.function_complete_flag_value);
          if(!ctx.event_handle &&
             (::ndnboost::detail::interlocked_read_acquire(&flag.count)>1))
          {
              ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
          }
          if(ctx.event_handle)
          {
              ::ndnboost::detail::win32::SetEvent(ctx.event_handle);
          }
        }
        inline void rollback_once_region(once_flag& flag, once_context& ctx) NDNBOOST_NOEXCEPT
        {
          NDNBOOST_INTERLOCKED_EXCHANGE(&flag.status,0);
          if(!ctx.event_handle)
          {
              ctx.event_handle=detail::open_once_event(ctx.mutex_name,&flag);
          }
          if(ctx.event_handle)
          {
              ::ndnboost::detail::win32::SetEvent(ctx.event_handle);
          }
        }
    }

#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
//#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
    inline void call_once(once_flag& flag, void (*f)())
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  f();
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
//#endif
    template<typename Function>
    inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                    f();
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, class A, class ...ArgTypes>
    inline void call_once(once_flag& flag, NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(A) a, NDNBOOST_THREAD_RV_REF(ArgTypes)... args)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(
                        thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                        thread_detail::decay_copy(ndnboost::forward<A>(a)),
                        thread_detail::decay_copy(ndnboost::forward<ArgTypes>(args))...
                  ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
#else
#if ! defined(NDNBOOST_MSVC) && ! defined(NDNBOOST_INTEL)
    template<typename Function>
    void call_once(once_flag& flag,Function f)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                    f();
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1>
    void call_once(once_flag& flag,Function f, T1 p1)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2>
    void call_once(once_flag& flag,Function f, T1 p1, T2 p2)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1,p2) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2, typename T3>
    void call_once(once_flag& flag,Function f, T1 p1, T2 p2, T3 p3)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1,p2,p3) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
#elif defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES

    template<typename Function>
    void call_once(once_flag& flag,Function const&f)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                    f();
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1>
    void call_once(once_flag& flag,Function const&f, T1 const&p1)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2>
    void call_once(once_flag& flag,Function const&f, T1 const&p1, T2 const&p2)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1,p2) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2, typename T3>
    void call_once(once_flag& flag,Function const&f, T1 const&p1, T2 const&p2, T3 const&p3)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(f,p1,p2,p3) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
#endif
#if 1
#if defined(NDNBOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
        inline void call_once(once_flag& flag, void (*f)())
        {
            // Try for a quick win: if the procedure has already been called
            // just skip through:
            detail::once_context ctx;
            while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
                  !=ctx.function_complete_flag_value)
            {
                if(detail::enter_once_region(flag, ctx))
                {
                    NDNBOOST_TRY
                    {
                      f();
                    }
                    NDNBOOST_CATCH(...)
                    {
                        detail::rollback_once_region(flag, ctx);
                        NDNBOOST_RETHROW
                    }
                    NDNBOOST_CATCH_END
                    detail::commit_once_region(flag, ctx);
                    break;
                }
                if(!ctx.counted)
                {
                    NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                    ctx.counted=true;
                    long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                    if(status==ctx.function_complete_flag_value)
                    {
                        break;
                    }
                    if(!ctx.event_handle)
                    {
                        ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                        continue;
                    }
                }
                NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                                 ctx.event_handle,::ndnboost::detail::win32::infinite));
            }
        }
        template<typename T1>
        void call_once(once_flag& flag,void (*f)(NDNBOOST_THREAD_RV_REF(T1)), NDNBOOST_THREAD_RV_REF(T1) p1)
        {
            // Try for a quick win: if the procedure has already been called
            // just skip through:
            detail::once_context ctx;
            while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
                  !=ctx.function_complete_flag_value)
            {
                if(detail::enter_once_region(flag, ctx))
                {
                    NDNBOOST_TRY
                    {
                       f(
                           thread_detail::decay_copy(ndnboost::forward<T1>(p1))
                       );
                    }
                    NDNBOOST_CATCH(...)
                    {
                        detail::rollback_once_region(flag, ctx);
                        NDNBOOST_RETHROW
                    }
                    NDNBOOST_CATCH_END
                    detail::commit_once_region(flag, ctx);
                    break;
                }
                if(!ctx.counted)
                {
                    NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                    ctx.counted=true;
                    long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                    if(status==ctx.function_complete_flag_value)
                    {
                        break;
                    }
                    if(!ctx.event_handle)
                    {
                        ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                        continue;
                    }
                }
                NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                                 ctx.event_handle,::ndnboost::detail::win32::infinite));
            }
        }
        template<typename Function, typename T1, typename T2>
        void call_once(once_flag& flag,void (*f)(NDNBOOST_THREAD_RV_REF(T1),NDNBOOST_THREAD_RV_REF(T2)), NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2)
        {
            // Try for a quick win: if the procedure has already been called
            // just skip through:
            detail::once_context ctx;
            while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
                  !=ctx.function_complete_flag_value)
            {
                if(detail::enter_once_region(flag, ctx))
                {
                    NDNBOOST_TRY
                    {
                      f(
                          thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                          thread_detail::decay_copy(ndnboost::forward<T2>(p2))
                      );
                    }
                    NDNBOOST_CATCH(...)
                    {
                        detail::rollback_once_region(flag, ctx);
                        NDNBOOST_RETHROW
                    }
                    NDNBOOST_CATCH_END
                    detail::commit_once_region(flag, ctx);
                    break;
                }
                if(!ctx.counted)
                {
                    NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                    ctx.counted=true;
                    long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                    if(status==ctx.function_complete_flag_value)
                    {
                        break;
                    }
                    if(!ctx.event_handle)
                    {
                        ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                        continue;
                    }
                }
                NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                                 ctx.event_handle,::ndnboost::detail::win32::infinite));
            }
        }
        template<typename Function, typename T1, typename T2, typename T3>
        void call_once(once_flag& flag,void (*f)(NDNBOOST_THREAD_RV_REF(T1),NDNBOOST_THREAD_RV_REF(T2)), NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2, NDNBOOST_THREAD_RV_REF(T3) p3)
        {
            // Try for a quick win: if the procedure has already been called
            // just skip through:
            detail::once_context ctx;
            while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
                  !=ctx.function_complete_flag_value)
            {
                if(detail::enter_once_region(flag, ctx))
                {
                    NDNBOOST_TRY
                    {
                      f(
                          thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                          thread_detail::decay_copy(ndnboost::forward<T2>(p2)),
                          thread_detail::decay_copy(ndnboost::forward<T3>(p3))
                      );
                    }
                    NDNBOOST_CATCH(...)
                    {
                        detail::rollback_once_region(flag, ctx);
                        NDNBOOST_RETHROW
                    }
                    NDNBOOST_CATCH_END
                    detail::commit_once_region(flag, ctx);
                    break;
                }
                if(!ctx.counted)
                {
                    NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                    ctx.counted=true;
                    long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                    if(status==ctx.function_complete_flag_value)
                    {
                        break;
                    }
                    if(!ctx.event_handle)
                    {
                        ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                        continue;
                    }
                }
                NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                                 ctx.event_handle,::ndnboost::detail::win32::infinite));
            }
        }
#endif
    template<typename Function>
    void call_once(once_flag& flag,NDNBOOST_THREAD_RV_REF(Function) f)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                    f();
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }

    template<typename Function, typename T1>
    void call_once(once_flag& flag,NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(
                      thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                      thread_detail::decay_copy(ndnboost::forward<T1>(p1))
                  ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2>
    void call_once(once_flag& flag,NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(
                      thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                      thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                      thread_detail::decay_copy(ndnboost::forward<T2>(p2))
                  ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;
                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }
    template<typename Function, typename T1, typename T2, typename T3>
    void call_once(once_flag& flag,NDNBOOST_THREAD_RV_REF(Function) f, NDNBOOST_THREAD_RV_REF(T1) p1, NDNBOOST_THREAD_RV_REF(T2) p2, NDNBOOST_THREAD_RV_REF(T3) p3)
    {
        // Try for a quick win: if the procedure has already been called
        // just skip through:
        detail::once_context ctx;
        while(::ndnboost::detail::interlocked_read_acquire(&flag.status)
              !=ctx.function_complete_flag_value)
        {
            if(detail::enter_once_region(flag, ctx))
            {
                NDNBOOST_TRY
                {
                  NDNBOOST_THREAD_INVOKE_RET_VOID(
                      thread_detail::decay_copy(ndnboost::forward<Function>(f)),
                      thread_detail::decay_copy(ndnboost::forward<T1>(p1)),
                      thread_detail::decay_copy(ndnboost::forward<T2>(p2)),
                      thread_detail::decay_copy(ndnboost::forward<T3>(p3))
                  ) NDNBOOST_THREAD_INVOKE_RET_VOID_CALL;

                }
                NDNBOOST_CATCH(...)
                {
                    detail::rollback_once_region(flag, ctx);
                    NDNBOOST_RETHROW
                }
                NDNBOOST_CATCH_END
                detail::commit_once_region(flag, ctx);
                break;
            }
            if(!ctx.counted)
            {
                NDNBOOST_INTERLOCKED_INCREMENT(&flag.count);
                ctx.counted=true;
                long status=::ndnboost::detail::interlocked_read_acquire(&flag.status);
                if(status==ctx.function_complete_flag_value)
                {
                    break;
                }
                if(!ctx.event_handle)
                {
                    ctx.event_handle=detail::create_once_event(ctx.mutex_name,&flag);
                    continue;
                }
            }
            NDNBOOST_VERIFY(!::ndnboost::detail::win32::WaitForSingleObject(
                             ctx.event_handle,::ndnboost::detail::win32::infinite));
        }
    }

#endif
#endif
}

#include <ndnboost/config/abi_suffix.hpp>

#endif
