// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba

#ifndef NDNBOOST_THREAD_LOCKABLE_TRAITS_HPP
#define NDNBOOST_THREAD_LOCKABLE_TRAITS_HPP

#include <ndnboost/thread/detail/config.hpp>

#include <ndnboost/assert.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#include <ndnboost/config/abi_prefix.hpp>

// todo make use of integral_constant, true_type and false_type

namespace ndnboost
{
  namespace sync
  {

#if defined(NDNBOOST_NO_SFINAE) ||                           \
    NDNBOOST_WORKAROUND(__IBMCPP__, NDNBOOST_TESTED_AT(600)) || \
    NDNBOOST_WORKAROUND(__SUNPRO_CC, NDNBOOST_TESTED_AT(0x590))
#if ! defined NDNBOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
#define NDNBOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
#endif
#endif

#ifndef NDNBOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
    namespace detail
    {
#define NDNBOOST_THREAD_DEFINE_HAS_MEMBER_CALLED(member_name)                     \
        template<typename T, bool=ndnboost::is_class<T>::value>            \
        struct has_member_called_##member_name                          \
        {                                                               \
            NDNBOOST_STATIC_CONSTANT(bool, value=false);                   \
        };                                                              \
                                                                        \
        template<typename T>                                            \
        struct has_member_called_##member_name<T,true>                  \
        {                                                               \
            typedef char true_type;                                     \
            struct false_type                                           \
            {                                                           \
                true_type dummy[2];                                     \
            };                                                          \
                                                                        \
            struct fallback { int member_name; };                       \
            struct derived:                                             \
                T, fallback                                             \
            {                                                           \
                derived();                                              \
            };                                                          \
                                                                        \
            template<int fallback::*> struct tester;                    \
                                                                        \
            template<typename U>                                        \
                static false_type has_member(tester<&U::member_name>*); \
            template<typename U>                                        \
                static true_type has_member(...);                       \
                                                                        \
            NDNBOOST_STATIC_CONSTANT(                                      \
                bool, value=sizeof(has_member<derived>(0))==sizeof(true_type)); \
        }

      NDNBOOST_THREAD_DEFINE_HAS_MEMBER_CALLED(lock)
;      NDNBOOST_THREAD_DEFINE_HAS_MEMBER_CALLED(unlock);
      NDNBOOST_THREAD_DEFINE_HAS_MEMBER_CALLED(try_lock);

      template<typename T,bool=has_member_called_lock<T>::value >
      struct has_member_lock
      {
        NDNBOOST_STATIC_CONSTANT(bool, value=false);
      };

      template<typename T>
      struct has_member_lock<T,true>
      {
        typedef char true_type;
        struct false_type
        {
          true_type dummy[2];
        };

        template<typename U,typename V>
        static true_type has_member(V (U::*)());
        template<typename U>
        static false_type has_member(U);

        NDNBOOST_STATIC_CONSTANT(
            bool,value=sizeof(has_member_lock<T>::has_member(&T::lock))==sizeof(true_type));
      };

      template<typename T,bool=has_member_called_unlock<T>::value >
      struct has_member_unlock
      {
        NDNBOOST_STATIC_CONSTANT(bool, value=false);
      };

      template<typename T>
      struct has_member_unlock<T,true>
      {
        typedef char true_type;
        struct false_type
        {
          true_type dummy[2];
        };

        template<typename U,typename V>
        static true_type has_member(V (U::*)());
        template<typename U>
        static false_type has_member(U);

        NDNBOOST_STATIC_CONSTANT(
            bool,value=sizeof(has_member_unlock<T>::has_member(&T::unlock))==sizeof(true_type));
      };

      template<typename T,bool=has_member_called_try_lock<T>::value >
      struct has_member_try_lock
      {
        NDNBOOST_STATIC_CONSTANT(bool, value=false);
      };

      template<typename T>
      struct has_member_try_lock<T,true>
      {
        typedef char true_type;
        struct false_type
        {
          true_type dummy[2];
        };

        template<typename U>
        static true_type has_member(bool (U::*)());
        template<typename U>
        static false_type has_member(U);

        NDNBOOST_STATIC_CONSTANT(
            bool,value=sizeof(has_member_try_lock<T>::has_member(&T::try_lock))==sizeof(true_type));
      };

    }

    template<typename T>
    struct is_basic_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = detail::has_member_lock<T>::value &&
          detail::has_member_unlock<T>::value);
    };
    template<typename T>
    struct is_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value =
          is_basic_lockable<T>::value &&
          detail::has_member_try_lock<T>::value);
    };

#else
    template<typename T>
    struct is_basic_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = false);
    };
    template<typename T>
    struct is_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = false);
    };
#endif

    template<typename T>
    struct is_recursive_mutex_sur_parole
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = false);
    };
    template<typename T>
    struct is_recursive_mutex_sur_parolle : is_recursive_mutex_sur_parole<T>
    {
    };

    template<typename T>
    struct is_recursive_basic_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = is_basic_lockable<T>::value &&
          is_recursive_mutex_sur_parolle<T>::value);
    };
    template<typename T>
    struct is_recursive_lockable
    {
      NDNBOOST_STATIC_CONSTANT(bool, value = is_lockable<T>::value &&
          is_recursive_mutex_sur_parolle<T>::value);
    };
  }
  template<typename T>
  struct is_mutex_type
  {
    NDNBOOST_STATIC_CONSTANT(bool, value = sync::is_lockable<T>::value);
  };

}
#include <ndnboost/config/abi_suffix.hpp>

#endif
