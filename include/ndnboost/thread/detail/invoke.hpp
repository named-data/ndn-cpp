// Copyright (C) 2012-2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// 2013/04 Vicente J. Botet Escriba
//    Provide implementation up to 9 parameters when NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES is defined.
//    Make use of Boost.Move
//    Make use of Boost.Tuple (movable)
// 2012 Vicente J. Botet Escriba
//    Provide implementation _RET using bind when NDNBOOST_NO_CXX11_HDR_FUNCTIONAL and NDNBOOST_NO_SFINAE_EXPR are not defined
// 2012 Vicente J. Botet Escriba
//    Adapt to boost libc++ implementation

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
// The invoke code is based on the one from libcxx.
//===----------------------------------------------------------------------===//

#ifndef NDNBOOST_THREAD_DETAIL_INVOKE_HPP
#define NDNBOOST_THREAD_DETAIL_INVOKE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/static_assert.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/core/enable_if.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/type_traits/is_base_of.hpp>
#include <ndnboost/type_traits/is_pointer.hpp>
#include <ndnboost/type_traits/is_member_function_pointer.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>
#ifndef NDNBOOST_NO_CXX11_HDR_FUNCTIONAL
#include <functional>
#endif

namespace ndnboost
{
  namespace detail
  {


#if ! defined(NDNBOOST_NO_SFINAE_EXPR) && \
    ! defined(NDNBOOST_NO_CXX11_DECLTYPE) && \
    ! defined(NDNBOOST_NO_CXX11_DECLTYPE_N3276) && \
    ! defined(NDNBOOST_THREAD_NO_CXX11_DECLTYPE_N3276) && \
    ! defined(NDNBOOST_NO_CXX11_TRAILING_RESULT_TYPES)

#define NDNBOOST_THREAD_PROVIDES_INVOKE

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
    // bullets 1 and 2

    template <class Fp, class A0, class ...Args>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }
    template <class R, class Fp, class A0, class ...Args>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Fp, class A0, class ...Args>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }
    template <class R, class Fp, class A0, class ...Args>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }

    // bullets 3 and 4

    template <class Fp, class A0>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(ndnboost::forward<A0>(a0).*f)
    {
        return ndnboost::forward<A0>(a0).*f;
    }

    template <class Fp, class A0>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((*ndnboost::forward<A0>(a0)).*f)
    {
        return (*ndnboost::forward<A0>(a0)).*f;
    }

    template <class R, class Fp, class A0>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(ndnboost::forward<A0>(a0).*f)
    {
        return ndnboost::forward<A0>(a0).*f;
    }

    template <class R, class Fp, class A0>
    inline auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((*ndnboost::forward<A0>(a0)).*f)
    {
        return (*ndnboost::forward<A0>(a0)).*f;
    }


    // bullet 5

    template <class R, class Fp, class ...Args>
    inline auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(Args) ...args)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<Args>(args)...))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<Args>(args)...);
    }
    template <class Fp, class ...Args>
    inline auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(Args) ...args)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<Args>(args)...))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<Args>(args)...);
    }

#else // NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES

    // bullets 1 and 2

    template <class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((ndnboost::forward<A0>(a0).*f)())
    {
        return (ndnboost::forward<A0>(a0).*f)();
    }
    template <class R, class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((ndnboost::forward<A0>(a0).*f)())
    {
        return (ndnboost::forward<A0>(a0).*f)();
    }
    template <class Fp, class A0, class A1>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1)))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1));
    }
    template <class R, class Fp, class A0, class A1>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1)))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Fp, class A0, class A1, class A2>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class R, class Fp, class A0, class A1, class A2>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
        -> decltype((ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }

    template <class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)())
    {
        return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class R, class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)())
    {
        return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class Fp, class A0, class A1>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1)))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class R, class Fp, class A0, class A1>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1)))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Fp, class A0, class A1, class A2>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class R, class Fp, class A0, class A1, class A2>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
        -> decltype(((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
        return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }

    // bullets 3 and 4

    template <class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(ndnboost::forward<A0>(a0).*f)
    {
        return ndnboost::forward<A0>(a0).*f;
    }
    template <class R, class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype(ndnboost::forward<A0>(a0).*f)
    {
        return ndnboost::forward<A0>(a0).*f;
    }

    template <class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((*ndnboost::forward<A0>(a0)).*f)
    {
        return (*ndnboost::forward<A0>(a0)).*f;
    }
    template <class R, class Fp, class A0>
    inline
    auto
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A0) a0)
        -> decltype((*ndnboost::forward<A0>(a0)).*f)
    {
        return (*ndnboost::forward<A0>(a0)).*f;
    }

    // bullet 5

    template <class Fp>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f)
    -> decltype(ndnboost::forward<Fp>(f)())
    {
      return ndnboost::forward<Fp>(f)();
    }
    template <class Fp, class A1>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1));
    }    template <class Fp, class A1, class A2>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Fp, class A1, class A2, class A3>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }


    template <class R, class Fp>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f)
    -> decltype(ndnboost::forward<Fp>(f)())
    {
      return ndnboost::forward<Fp>(f)();
    }
    template <class R, class Fp, class A1>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1));
    }
    template <class R, class Fp, class A1, class A2>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class R, class Fp, class A1, class A2, class A3>
    inline
    auto invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    -> decltype(ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3)))
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }

#endif // NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES

#elif ! defined(NDNBOOST_NO_SFINAE_EXPR) && \
    ! defined NDNBOOST_NO_CXX11_HDR_FUNCTIONAL && \
    defined  NDNBOOST_MSVC

//#error
    template <class Ret, class Fp>
    inline
    Ret invoke(NDNBOOST_THREAD_RV_REF(Fp) f)
    {
      return f();
    }
    template <class Ret, class Fp, class A1>
    inline
    Ret invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return std::bind(ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1))();
    }
    template <class Ret, class Fp, class A1, class A2>
    inline
    Ret invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return std::bind(ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2))();
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline
    Ret invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return std::bind(ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3))();
    }

#define NDNBOOST_THREAD_PROVIDES_INVOKE_RET

#elif ! defined  NDNBOOST_MSVC
//!!!!!  WARNING !!!!! THIS DOESN'T WORKS YET
#define NDNBOOST_THREAD_PROVIDES_INVOKE_RET

#if ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

    // bullet 1
    // (t1.*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is an object of
    // type T or a reference to an object of type T or a reference to an object of a type derived from T
    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...), NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) const, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) const volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<Args>(args)...);
    }

    // bullet 2
    // ((*t1).*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is not one of
    // the types described in the previous item;
    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...), NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) const, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class A, class A0, class ...Args>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(Args...) const volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<Args>(args)...);
    }

    // bullet 3
    // t1.*f when N == 1 and f is a pointer to member data of a class T and t1 is an object of type T or a
    // reference to an object of type T or a reference to an object of a type derived from T;
//    template <class Ret, class A, class A0>
//    inline
//    typename enable_if_c
//    <
//        is_base_of<A, typename remove_reference<A0>::type>::value,
//        typename detail::apply_cv<A0, A>::type&
//    >::type
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return ndnboost::forward<A0>(a0).*f;
//    }

    // bullet 4
    // (*t1).*f when N == 1 and f is a pointer to member data of a class T and t1 is not one of the types
    //described in the previous item;

//    template <class A0, class Ret, bool>
//    struct d4th_helper
//    {
//    };
//
//    template <class A0, class Ret>
//    struct d4th_helper<A0, Ret, true>
//    {
//        typedef typename apply_cv<decltype(*declval<A0>()), Ret>::type type;
//    };
//
//    template <class Ret, class A, class A0>
//    inline
//    typename detail::4th_helper<A, Ret,
//                          !is_base_of<A,
//                                      typename remove_reference<A0>::type
//                                     >::value
//                         >::type&
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return (*ndnboost::forward<A0>(a0)).*f;
//    }

//    template <class Ret, class A, class A0>
//    inline
//    typename enable_if_c
//    <
//        !is_base_of<A, typename remove_reference<A0>::type>::value,
//        typename detail::ref_return1<Ret A::*, A0>::type
//    >::type
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return (*ndnboost::forward<A0>(a0)).*f;
//    }

    // bullet 5
    // f(t1, t2, ..., tN) in all other cases.

    template <class Ret, class Fp, class ...Args>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class Fp, class ...Args>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return f(ndnboost::forward<Args>(args)...);
    }

    template <class Ret, class Fp, class ...Args>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_RV_REF(Fp) f, NDNBOOST_THREAD_RV_REF(Args) ...args)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), ndnboost::forward<Args>(args)...);
    }
#else // NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
    // bullet 1
    // (t1.*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is an object of
    // type T or a reference to an object of type T or a reference to an object of a type derived from T

    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(), A0& a0)
    {
        return (a0.*f)();
    }
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(), A0* a0)
    {
        return ((*a0).*f)();
    }

    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1),
        A0& a0, NDNBOOST_THREAD_RV_REF(A1) a1
        )
    {
        return (a0.*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1), A0& a0, A1 a1)
    {
        return (a0.*f)(a1);
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1), A0* a0, NDNBOOST_THREAD_RV_REF(A1) a1
        )
    {
        return (*(a0).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1), A0* a0, A1 a1)
    {
        return (*a0.*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2),
        A0& a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2
        )
    {
        return (a0.*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2), A0* a0, A1 a1, A2 a2)
    {
        return ((*a0).*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3),
        A0& a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
        return (a0.*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3), A0* a0, A1 a1, A2 a2, A3 a3)
    {
        return ((*a0).*f)(a1, a2, a3);
    }

///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const, A0 const& a0)
    {
        return (a0.*f)();
    }
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const, A0 const* a0)
    {
        return ((*a0).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const, A0 const& a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
        return (a0.*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const, A0 const* a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
        return ((*a0).*f)(ndnboost::forward<A1>(a1));
    }

    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const, A0 const& a0, A1 a1)
    {
        return (a0.*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const,
        A0 const& a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2
        )
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2)
            );
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const, A0 const& a0, A1 a1, A2 a2)
    {
        return (a0.*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3
        )
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const, A0 a0, A1 a1, A2 a2, A3 a3)
    {
        return (a0.*f)(a1, a2, a3);
    }
    ///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() volatile, NDNBOOST_THREAD_RV_REF(A0) a0)
    {
        return (ndnboost::forward<A0>(a0).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) volatile, A0 a0, A1 a1)
    {
        return (a0.*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) volatile, A0 a0, A1 a1, A2 a2 )
    {
        return (a0.*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3
        )
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) volatile, A0 a0, A1 a1, A2 a2, A3 a3)
    {
        return (a0.*f)(a1, a2, a3);
    }
    ///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const volatile, NDNBOOST_THREAD_RV_REF(A0) a0)
    {
        return (ndnboost::forward<A0>(a0).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const volatile, NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const volatile, A0 a0, A1 a1)
    {
        return (a0.*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2
        )
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const volatile,
        A0 a0, A1 a1, A2 a2
        )
    {
        return (a0.*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3
        )
    {
        return (ndnboost::forward<A0>(a0).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const volatile,
        A0 a0, A1 a1, A2 a2, A3 a3
        )
    {
        return (a0.*f)(a1, a2, a3);
    }

    // bullet 2
    // ((*t1).*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is not one of
    // the types described in the previous item;
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(), NDNBOOST_THREAD_RV_REF(A0) a0)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1), NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1), A0 a0, A1 a1)
    {
      return ((*a0).*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, NDNBOOST_THREAD_RV_REF(A2)),
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2), A0 a0, A1 a1, A2 a2)
    {
      return ((*a0).*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, NDNBOOST_THREAD_RV_REF(A2), NDNBOOST_THREAD_RV_REF(A3)),
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3)
          );
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3), A0 a0, A1 a1, A2 a2, A3 a3)
    {
      return ((*a0).*f)(a1, a2, a3);
    }

///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const, NDNBOOST_THREAD_RV_REF(A0) a0)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const, NDNBOOST_THREAD_RV_REF(A0) a0, A1 a1)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(a1);
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const, A0 a0, A1 a1)
    {
      return ((*a0).*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const, A0 a0, A1 a1, A2 a2)
    {
      return ((*a0).*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const,
        A0 a0, A1 a1, A2 a2, A3 a3)
    {
      return ((*a0).*f)(a1, a2, a3);
    }
    ///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() volatile, NDNBOOST_THREAD_RV_REF(A0) a0)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) volatile, A0 a0, A1 a1)
    {
      return ((*a0).*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) volatile, A0 a0, A1 a1, A2 a2)
    {
      return ((*a0).*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) volatile, A0 a0, A1 a1, A2 a2, A3 a3)
    {
      return ((*a0).*f)(a1, a2, a3);
    }
    ///
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const volatile, NDNBOOST_THREAD_RV_REF(A0) a0)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)();
    }
    template <class Ret, class A, class A0>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)() const volatile, A0 a0)
    {
      return ((*a0).*f)();
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class A, class A0, class A1>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1) const volatile, A0 a0, A1 a1)
    {
      return ((*a0).*f)(a1);
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class A, class A0, class A1, class A2>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2) const volatile,
        A0 a0, A1 a1, A2 a2)
    {
      return ((*a0).*f)(a1, a2);
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const volatile,
        NDNBOOST_THREAD_RV_REF(A0) a0, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ((*ndnboost::forward<A0>(a0)).*f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class A, class A0, class A1, class A2, class A3>
    inline
    typename enable_if_c
    <
        ! is_base_of<A, typename remove_reference<A0>::type>::value,
        Ret
    >::type
    invoke(Ret (A::*f)(A1, A2, A3) const volatile,
        A0 a0, A1 a1, A2 a2, A3 a3)
    {
      return ((*a0).*f)(a1, a2, a3);
    }
    // bullet 3
    // t1.*f when N == 1 and f is a pointer to member data of a class T and t1 is an object of type T or a
    // reference to an object of type T or a reference to an object of a type derived from T;
//    template <class Ret, class A, class A0>
//    inline
//    typename enable_if_c
//    <
//        is_base_of<A, typename remove_reference<A0>::type>::value,
//        typename detail::apply_cv<A0, A>::type&
//    >::type
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return ndnboost::forward<A0>(a0).*f;
//    }

    // bullet 4
    // (*t1).*f when N == 1 and f is a pointer to member data of a class T and t1 is not one of the types
    //described in the previous item;

//    template <class A0, class Ret, bool>
//    struct d4th_helper
//    {
//    };
//
//    template <class A0, class Ret>
//    struct d4th_helper<A0, Ret, true>
//    {
//        typedef typename apply_cv<decltype(*declval<A0>()), Ret>::type type;
//    };
//
//    template <class Ret, class A, class A0>
//    inline
//    typename detail::4th_helper<A, Ret,
//                          !is_base_of<A,
//                                      typename remove_reference<A0>::type
//                                     >::value
//                         >::type&
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return (*ndnboost::forward<A0>(a0)).*f;
//    }

//    template <class Ret, class A, class A0>
//    inline
//    typename enable_if_c
//    <
//        !is_base_of<A, typename remove_reference<A0>::type>::value,
//        typename detail::ref_return1<Ret A::*, A0>::type
//    >::type
//    invoke(Ret A::* f, NDNBOOST_THREAD_RV_REF(A0) a0)
//    {
//        return (*ndnboost::forward<A0>(a0)).*f;
//    }

    // bullet 5
    // f(t1, t2, ..., tN) in all other cases.

    template <class Ret, class Fp>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f)
    {
      return ndnboost::forward<Fp>(f)();
    }
    template <class Ret, class Fp>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f)
    {
      return f();
    }
    template <class Ret, class Fp>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f));
    }

    template <class Ret, class Fp, class A1>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class Fp, class A1>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return f(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class Fp, class A1>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1));
    }

    template <class Ret, class Fp, class A1, class A2>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class Fp, class A1, class A2>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return f(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class Fp, class A1, class A2>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }

    template <class Ret, class Fp, class A1, class A2, class A3>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ndnboost::forward<Fp>(f)(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return f(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }


    template <class Ret, class Fp, class A1>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1)
    {
      return ndnboost::forward<Fp>(f)(a1);
    }
    template <class Ret, class Fp, class A1>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1)
    {
      return f(a1);
    }
    template <class Ret, class Fp, class A1>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), a1);
    }

    template <class Ret, class Fp, class A1, class A2>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2)
    {
      return ndnboost::forward<Fp>(f)(a1, a2);
    }
    template <class Ret, class Fp, class A1, class A2>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2)
    {
      return f(a1, a2);
    }
    template <class Ret, class Fp, class A1, class A2>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), a1, a2);
    }

    template <class Ret, class Fp, class A1, class A2, class A3>
    inline Ret do_invoke(mpl::false_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2, A3 a3)
    {
      return ndnboost::forward<Fp>(f)(a1, a2, a3);
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline Ret do_invoke(mpl::true_, NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2, A3 a3)
    {
      return f(a1, a2, a3);
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(NDNBOOST_THREAD_FWD_REF(Fp) f, A1 a1, A2 a2, A3 a3)
    {
      return ndnboost::detail::do_invoke<Ret>(ndnboost::is_pointer<Fp>(), ndnboost::forward<Fp>(f), a1, a2, a3);
    }


    ///
    template <class Ret, class Fp>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f)
    {
      return f();
    }
    template <class Ret, class Fp, class A1>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, NDNBOOST_THREAD_RV_REF(A1) a1)
    {
      return f(ndnboost::forward<A1>(a1));
    }
    template <class Ret, class Fp, class A1>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, A1 a1)
    {
      return f(a1);
    }
    template <class Ret, class Fp, class A1, class A2>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2)
    {
      return f(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2));
    }
    template <class Ret, class Fp, class A1, class A2>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, A1 a1, A2 a2)
    {
      return f(a1, a2);
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, NDNBOOST_THREAD_RV_REF(A1) a1, NDNBOOST_THREAD_RV_REF(A2) a2, NDNBOOST_THREAD_RV_REF(A3) a3)
    {
      return f(ndnboost::forward<A1>(a1), ndnboost::forward<A2>(a2), ndnboost::forward<A3>(a3));
    }
    template <class Ret, class Fp, class A1, class A2, class A3>
    inline
    typename disable_if_c
    <
        is_member_function_pointer<Fp>::value,
        Ret
    >::type
    invoke(Fp &f, A1 a1, A2 a2, A3 a3)
    {
      return f(a1, a2, a3);
    }
    ///

#endif // NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES

#endif  // all
      }
    }

#endif // header
