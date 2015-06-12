// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007-8 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba

#ifndef NDNBOOST_THREAD_MOVE_HPP
#define NDNBOOST_THREAD_MOVE_HPP

#include <ndnboost/thread/detail/config.hpp>
#ifndef NDNBOOST_NO_SFINAE
#include <ndnboost/core/enable_if.hpp>
#include <ndnboost/type_traits/is_convertible.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>
#include <ndnboost/type_traits/remove_cv.hpp>
#include <ndnboost/type_traits/decay.hpp>
#include <ndnboost/type_traits/conditional.hpp>
#include <ndnboost/type_traits/remove_extent.hpp>
#include <ndnboost/type_traits/is_array.hpp>
#include <ndnboost/type_traits/is_function.hpp>
#include <ndnboost/type_traits/remove_cv.hpp>
#include <ndnboost/type_traits/add_pointer.hpp>
#include <ndnboost/type_traits/decay.hpp>
#endif

#include <ndnboost/thread/detail/delete.hpp>
#include <ndnboost/move/utility.hpp>
#include <ndnboost/move/traits.hpp>
#include <ndnboost/config/abi_prefix.hpp>
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#include <type_traits>
#endif
namespace ndnboost
{

    namespace detail
    {
      template <typename T>
      struct enable_move_utility_emulation_dummy_specialization;
        template<typename T>
        struct thread_move_t
        {
            T& t;
            explicit thread_move_t(T& t_):
                t(t_)
            {}

            T& operator*() const
            {
                return t;
            }

            T* operator->() const
            {
                return &t;
            }
        private:
            void operator=(thread_move_t&);
        };
    }

#if !defined NDNBOOST_THREAD_USES_MOVE

#ifndef NDNBOOST_NO_SFINAE
    template<typename T>
    typename enable_if<ndnboost::is_convertible<T&,ndnboost::detail::thread_move_t<T> >, ndnboost::detail::thread_move_t<T> >::type move(T& t)
    {
        return ndnboost::detail::thread_move_t<T>(t);
    }
#endif

    template<typename T>
    ndnboost::detail::thread_move_t<T> move(ndnboost::detail::thread_move_t<T> t)
    {
        return t;
    }

#endif   //#if !defined NDNBOOST_THREAD_USES_MOVE
}

#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES

#define NDNBOOST_THREAD_RV_REF(TYPE) NDNBOOST_RV_REF(TYPE)
#define NDNBOOST_THREAD_RV_REF_2_TEMPL_ARGS(TYPE) NDNBOOST_RV_REF_2_TEMPL_ARGS(TYPE)
#define NDNBOOST_THREAD_RV_REF_BEG NDNBOOST_RV_REF_BEG
#define NDNBOOST_THREAD_RV_REF_END NDNBOOST_RV_REF_END
#define NDNBOOST_THREAD_RV(V) V
#define NDNBOOST_THREAD_MAKE_RV_REF(RVALUE) RVALUE
#define NDNBOOST_THREAD_FWD_REF(TYPE) NDNBOOST_FWD_REF(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) \
  namespace detail { \
    template <typename T> \
    struct enable_move_utility_emulation_dummy_specialization<

#define NDNBOOST_THREAD_DCL_MOVABLE_END > \
      : integral_constant<bool, false> \
      {}; \
    }

#elif ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES && defined  NDNBOOST_MSVC

#define NDNBOOST_THREAD_RV_REF(TYPE) NDNBOOST_RV_REF(TYPE)
#define NDNBOOST_THREAD_RV_REF_2_TEMPL_ARGS(TYPE) NDNBOOST_RV_REF_2_TEMPL_ARGS(TYPE)
#define NDNBOOST_THREAD_RV_REF_BEG NDNBOOST_RV_REF_BEG
#define NDNBOOST_THREAD_RV_REF_END NDNBOOST_RV_REF_END
#define NDNBOOST_THREAD_RV(V) V
#define NDNBOOST_THREAD_MAKE_RV_REF(RVALUE) RVALUE
#define NDNBOOST_THREAD_FWD_REF(TYPE) NDNBOOST_FWD_REF(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) \
  namespace detail { \
    template <typename T> \
    struct enable_move_utility_emulation_dummy_specialization<

#define NDNBOOST_THREAD_DCL_MOVABLE_END > \
      : integral_constant<bool, false> \
      {}; \
    }

#else

#if defined NDNBOOST_THREAD_USES_MOVE
#define NDNBOOST_THREAD_RV_REF(TYPE) NDNBOOST_RV_REF(TYPE)
#define NDNBOOST_THREAD_RV_REF_2_TEMPL_ARGS(TYPE) NDNBOOST_RV_REF_2_TEMPL_ARGS(TYPE)
#define NDNBOOST_THREAD_RV_REF_BEG NDNBOOST_RV_REF_BEG
#define NDNBOOST_THREAD_RV_REF_END NDNBOOST_RV_REF_END
#define NDNBOOST_THREAD_RV(V) V
#define NDNBOOST_THREAD_FWD_REF(TYPE) NDNBOOST_FWD_REF(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE(TYPE)
#define NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) \
  namespace detail { \
    template <typename T> \
    struct enable_move_utility_emulation_dummy_specialization<

#define NDNBOOST_THREAD_DCL_MOVABLE_END > \
      : integral_constant<bool, false> \
      {}; \
    }

#else

#define NDNBOOST_THREAD_RV_REF(TYPE) ndnboost::detail::thread_move_t< TYPE >
#define NDNBOOST_THREAD_RV_REF_BEG ndnboost::detail::thread_move_t<
#define NDNBOOST_THREAD_RV_REF_END >
#define NDNBOOST_THREAD_RV(V) (*V)
#define NDNBOOST_THREAD_FWD_REF(TYPE) NDNBOOST_FWD_REF(TYPE)

#define NDNBOOST_THREAD_DCL_MOVABLE(TYPE) \
template <> \
struct enable_move_utility_emulation< TYPE > \
{ \
   static const bool value = false; \
};

#define NDNBOOST_THREAD_DCL_MOVABLE_BEG(T) \
template <typename T> \
struct enable_move_utility_emulation<

#define NDNBOOST_THREAD_DCL_MOVABLE_END > \
{ \
   static const bool value = false; \
};

#endif

namespace ndnboost
{
namespace detail
{
  template <typename T>
  NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type)
  make_rv_ref(T v)  NDNBOOST_NOEXCEPT
  {
    return (NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type))(v);
  }
//  template <typename T>
//  NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type)
//  make_rv_ref(T &v)  NDNBOOST_NOEXCEPT
//  {
//    return (NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type))(v);
//  }
//  template <typename T>
//  const NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type)
//  make_rv_ref(T const&v)  NDNBOOST_NOEXCEPT
//  {
//    return (const NDNBOOST_THREAD_RV_REF(typename ::ndnboost::remove_cv<typename ::ndnboost::remove_reference<T>::type>::type))(v);
//  }
}
}

#define NDNBOOST_THREAD_MAKE_RV_REF(RVALUE) RVALUE.move()
//#define NDNBOOST_THREAD_MAKE_RV_REF(RVALUE) ndnboost::detail::make_rv_ref(RVALUE)
#endif


#if ! defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES

#define NDNBOOST_THREAD_MOVABLE(TYPE)

#else

#if defined NDNBOOST_THREAD_USES_MOVE

#define NDNBOOST_THREAD_MOVABLE(TYPE) \
    ::ndnboost::rv<TYPE>& move()  NDNBOOST_NOEXCEPT \
    { \
      return *static_cast< ::ndnboost::rv<TYPE>* >(this); \
    } \
    const ::ndnboost::rv<TYPE>& move() const NDNBOOST_NOEXCEPT \
    { \
      return *static_cast<const ::ndnboost::rv<TYPE>* >(this); \
    } \
    operator ::ndnboost::rv<TYPE>&() \
    { \
      return *static_cast< ::ndnboost::rv<TYPE>* >(this); \
    } \
    operator const ::ndnboost::rv<TYPE>&() const \
    { \
      return *static_cast<const ::ndnboost::rv<TYPE>* >(this); \
    }\

#else

#define NDNBOOST_THREAD_MOVABLE(TYPE) \
    operator ::ndnboost::detail::thread_move_t<TYPE>() NDNBOOST_NOEXCEPT \
    { \
        return move(); \
    } \
    ::ndnboost::detail::thread_move_t<TYPE> move() NDNBOOST_NOEXCEPT \
    { \
      ::ndnboost::detail::thread_move_t<TYPE> x(*this); \
        return x; \
    } \

#endif
#endif

#define NDNBOOST_THREAD_MOVABLE_ONLY(TYPE) \
  NDNBOOST_THREAD_NO_COPYABLE(TYPE) \
  NDNBOOST_THREAD_MOVABLE(TYPE) \

#define NDNBOOST_THREAD_COPYABLE_AND_MOVABLE(TYPE) \
  NDNBOOST_THREAD_MOVABLE(TYPE) \



namespace ndnboost
{
  namespace thread_detail
  {
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    template <class Tp>
    struct remove_reference : ndnboost::remove_reference<Tp> {};
    template <class Tp>
    struct  decay : ndnboost::decay<Tp> {};
#else
  template <class Tp>
  struct remove_reference
  {
    typedef Tp type;
  };
  template <class Tp>
  struct remove_reference<Tp&>
  {
    typedef Tp type;
  };
  template <class Tp>
  struct remove_reference< rv<Tp> > {
    typedef Tp type;
  };

  template <class Tp>
  struct  decay
  {
  private:
    typedef typename ndnboost::move_detail::remove_rvalue_reference<Tp>::type Up0;
    typedef typename ndnboost::remove_reference<Up0>::type Up;
  public:
      typedef typename conditional
                       <
                           is_array<Up>::value,
                           typename remove_extent<Up>::type*,
                           typename conditional
                           <
                                is_function<Up>::value,
                                typename add_pointer<Up>::type,
                                typename remove_cv<Up>::type
                           >::type
                       >::type type;
  };
#endif

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
      template <class T>
      typename decay<T>::type
      decay_copy(T&& t)
      {
          return ndnboost::forward<T>(t);
      }
#else
  template <class T>
  typename decay<T>::type
  decay_copy(NDNBOOST_THREAD_FWD_REF(T) t)
  {
      return ndnboost::forward<T>(t);
  }
#endif
  }
}

#include <ndnboost/config/abi_suffix.hpp>

#endif
