// Copyright (C) 2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// 2013/04 Vicente J. Botet Escriba
//    Provide implementation up to 9 parameters when NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES is defined.
//    Make use of Boost.Move
//    Make use of Boost.Tuple (movable)
// 2012/11 Vicente J. Botet Escriba
//    Adapt to boost libc++ implementation

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
// The invoker code is based on the one from libcxx.
//===----------------------------------------------------------------------===//

#ifndef NDNBOOST_THREAD_DETAIL_ASYNC_FUNCT_HPP
#define NDNBOOST_THREAD_DETAIL_ASYNC_FUNCT_HPP

#include <ndnboost/config.hpp>

#include <ndnboost/utility/result_of.hpp>
#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/thread/detail/invoke.hpp>
#include <ndnboost/thread/detail/make_tuple_indices.hpp>
#include <ndnboost/thread/csbl/tuple.hpp>
#include <ndnboost/tuple/tuple.hpp>

#include <ndnboost/thread/detail/variadic_header.hpp>

namespace ndnboost
{
  namespace detail
  {

#if defined(NDNBOOST_THREAD_PROVIDES_INVOKE) && ! defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && ! defined(NDNBOOST_NO_CXX11_HDR_TUPLE)

    template <class Fp, class ... Args>
    class invoker
    {
      //typedef typename decay<Fp>::type Fpd;
      //typedef tuple<typename decay<Args>::type...> Argsd;

      //csbl::tuple<Fpd, Argsd...> f_;
      csbl::tuple<Fp, Args...> f_;

    public:
      NDNBOOST_THREAD_MOVABLE_ONLY( invoker)
      //typedef typename invoke_of<_Fp, _Args...>::type Rp;
      typedef typename result_of<Fp(Args...)>::type result_type;

      template <class F, class ... As>
      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(As)... args)
      : f_(ndnboost::forward<F>(f), ndnboost::forward<As>(args)...)
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_RV_REF(invoker) f) : f_(ndnboost::move(NDNBOOST_THREAD_RV(f).f_))
      {}

      result_type operator()()
      {
        typedef typename make_tuple_indices<1+sizeof...(Args), 1>::type Index;
        return execute(Index());
      }
    private:
      template <size_t ...Indices>
      result_type
      execute(tuple_indices<Indices...>)
      {
        return invoke(ndnboost::move(csbl::get<0>(f_)), ndnboost::move(csbl::get<Indices>(f_))...);
      }
    };

    template <class R, class Fp, class ... Args>
    class invoker_ret
    {
      //typedef typename decay<Fp>::type Fpd;
      //typedef tuple<typename decay<Args>::type...> Argsd;

      //csbl::tuple<Fpd, Argsd...> f_;
      csbl::tuple<Fp, Args...> f_;

    public:
      NDNBOOST_THREAD_MOVABLE_ONLY( invoker_ret)
      typedef R result_type;

      template <class F, class ... As>
      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker_ret(NDNBOOST_THREAD_FWD_REF(F) f, NDNBOOST_THREAD_FWD_REF(As)... args)
      : f_(ndnboost::forward<F>(f), ndnboost::forward<As>(args)...)
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker_ret(NDNBOOST_THREAD_RV_REF(invoker_ret) f) : f_(ndnboost::move(NDNBOOST_THREAD_RV(f).f_))
      {}

      result_type operator()()
      {
        typedef typename make_tuple_indices<1+sizeof...(Args), 1>::type Index;
        return execute(Index());
      }
    private:
      template <size_t ...Indices>
      result_type
      execute(tuple_indices<Indices...>)
      {
        return invoke<R>(ndnboost::move(csbl::get<0>(f_)), ndnboost::move(csbl::get<Indices>(f_))...);
      }
    };
  //NDNBOOST_THREAD_DCL_MOVABLE_BEG(X) invoker<Fp> NDNBOOST_THREAD_DCL_MOVABLE_END
#else

#if ! defined NDNBOOST_MSVC

#define NDNBOOST_THREAD_RV_REF_ARG_T(z, n, unused) NDNBOOST_PP_COMMA_IF(n) NDNBOOST_THREAD_RV_REF(Arg##n)
#define NDNBOOST_THREAD_RV_REF_A_T(z, n, unused) NDNBOOST_PP_COMMA_IF(n) NDNBOOST_THREAD_RV_REF(A##n)
#define NDNBOOST_THREAD_RV_REF_ARG(z, n, unused) , NDNBOOST_THREAD_RV_REF(Arg##n) arg##n
#define NDNBOOST_THREAD_FWD_REF_A(z, n, unused)   , NDNBOOST_THREAD_FWD_REF(A##n) arg##n
#define NDNBOOST_THREAD_FWD_REF_ARG(z, n, unused) , NDNBOOST_THREAD_FWD_REF(Arg##n) arg##n
#define NDNBOOST_THREAD_FWD_PARAM(z, n, unused) , ndnboost::forward<Arg##n>(arg##n)
#define NDNBOOST_THREAD_FWD_PARAM_A(z, n, unused) , ndnboost::forward<A##n>(arg##n)
#define NDNBOOST_THREAD_DCL(z, n, unused) Arg##n v##n;
#define NDNBOOST_THREAD_MOVE_PARAM(z, n, unused) , v##n(ndnboost::move(arg##n))
#define NDNBOOST_THREAD_FORWARD_PARAM_A(z, n, unused) , v##n(ndnboost::forward<A##n>(arg##n))
#define NDNBOOST_THREAD_MOVE_RHS_PARAM(z, n, unused) , v##n(ndnboost::move(x.v##n))
#define NDNBOOST_THREAD_MOVE_DCL(z, n, unused) , ndnboost::move(v##n)
#define NDNBOOST_THREAD_MOVE_DCL_T(z, n, unused) NDNBOOST_PP_COMMA_IF(n) ndnboost::move(v##n)
#define NDNBOOST_THREAD_ARG_DEF(z, n, unused) , class Arg##n = tuples::null_type

  template  <class Fp, class Arg = tuples::null_type
    NDNBOOST_PP_REPEAT(NDNBOOST_THREAD_MAX_ARGS, NDNBOOST_THREAD_ARG_DEF, ~)
    >
    class invoker;

#define NDNBOOST_THREAD_ASYNC_FUNCT(z, n, unused) \
    template <class Fp NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, class Arg) > \
    class invoker<Fp NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, Arg)> \
    { \
      Fp fp_; \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_DCL, ~) \
    public: \
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker) \
      typedef typename result_of<Fp(NDNBOOST_PP_ENUM_PARAMS(n, Arg))>::type result_type; \
      \
      template <class F NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, class A) > \
      NDNBOOST_SYMBOL_VISIBLE \
      explicit invoker(NDNBOOST_THREAD_FWD_REF(F) f \
          NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_FWD_REF_A, ~) \
      ) \
      : fp_(ndnboost::forward<F>(f)) \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_FORWARD_PARAM_A, ~) \
      {} \
      \
      NDNBOOST_SYMBOL_VISIBLE \
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) x) \
      : fp_(ndnboost::move(x.fp_)) \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_MOVE_RHS_PARAM, ~) \
      {} \
      \
      result_type operator()() { \
        return invoke(ndnboost::move(fp_) \
            NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_MOVE_DCL, ~) \
        ); \
      } \
    }; \
    \
    template <class R NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, class Arg) > \
    class invoker<R(*)(NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_RV_REF_ARG_T, ~)) NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, Arg)> \
    { \
      typedef R(*Fp)(NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_RV_REF_ARG_T, ~)); \
      Fp fp_; \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_DCL, ~) \
    public: \
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker) \
      typedef typename result_of<Fp(NDNBOOST_PP_ENUM_PARAMS(n, Arg))>::type result_type; \
      \
      template <class R2 NDNBOOST_PP_COMMA_IF(n) NDNBOOST_PP_ENUM_PARAMS(n, class A) > \
      NDNBOOST_SYMBOL_VISIBLE \
      explicit invoker(R2(*f)(NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_RV_REF_A_T, ~))  \
          NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_FWD_REF_A, ~) \
      ) \
      : fp_(f) \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_FORWARD_PARAM_A, ~) \
      {} \
      \
      NDNBOOST_SYMBOL_VISIBLE \
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) x) \
      : fp_(x.fp_) \
      NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_MOVE_RHS_PARAM, ~) \
      {} \
      \
      result_type operator()() { \
        return fp_( \
            NDNBOOST_PP_REPEAT(n, NDNBOOST_THREAD_MOVE_DCL_T, ~) \
        ); \
      } \
    };

    NDNBOOST_PP_REPEAT(NDNBOOST_THREAD_MAX_ARGS, NDNBOOST_THREAD_ASYNC_FUNCT, ~)

    #undef NDNBOOST_THREAD_RV_REF_ARG_T
    #undef NDNBOOST_THREAD_RV_REF_ARG
    #undef NDNBOOST_THREAD_FWD_REF_ARG
    #undef NDNBOOST_THREAD_FWD_REF_A
    #undef NDNBOOST_THREAD_FWD_PARAM
    #undef NDNBOOST_THREAD_FWD_PARAM_A
    #undef NDNBOOST_THREAD_DCL
    #undef NDNBOOST_THREAD_MOVE_PARAM
    #undef NDNBOOST_THREAD_MOVE_RHS_PARAM
    #undef NDNBOOST_THREAD_MOVE_DCL
    #undef NDNBOOST_THREAD_ARG_DEF
    #undef NDNBOOST_THREAD_ASYNC_FUNCT

#else

    template <class Fp,
    class T0 = tuples::null_type, class T1 = tuples::null_type, class T2 = tuples::null_type,
    class T3 = tuples::null_type, class T4 = tuples::null_type, class T5 = tuples::null_type,
    class T6 = tuples::null_type, class T7 = tuples::null_type, class T8 = tuples::null_type
    , class T9 = tuples::null_type
    >
    class invoker;

    template <class Fp,
    class T0 , class T1 , class T2 ,
    class T3 , class T4 , class T5 ,
    class T6 , class T7 , class T8 >
    class invoker<Fp, T0, T1, T2, T3, T4, T5, T6, T7, T8>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
      T4 v4_;
      T5 v5_;
      T6 v6_;
      T7 v7_;
      T8 v8_;
      //::ndnboost::tuple<Fp, T0, T1, T2, T3, T4, T5, T6, T7, T8> f_;

    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3, T4, T5, T6, T7, T8)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
          , NDNBOOST_THREAD_RV_REF(T4) a4
          , NDNBOOST_THREAD_RV_REF(T5) a5
          , NDNBOOST_THREAD_RV_REF(T6) a6
          , NDNBOOST_THREAD_RV_REF(T7) a7
          , NDNBOOST_THREAD_RV_REF(T8) a8
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      , v4_(ndnboost::move(a4))
      , v5_(ndnboost::move(a5))
      , v6_(ndnboost::move(a6))
      , v7_(ndnboost::move(a7))
      , v8_(ndnboost::move(a8))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      , v4_(ndnboost::move(NDNBOOST_THREAD_RV(f).v4_))
      , v5_(ndnboost::move(NDNBOOST_THREAD_RV(f).v5_))
      , v6_(ndnboost::move(NDNBOOST_THREAD_RV(f).v6_))
      , v7_(ndnboost::move(NDNBOOST_THREAD_RV(f).v7_))
      , v8_(ndnboost::move(NDNBOOST_THREAD_RV(f).v8_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
            , ndnboost::move(v4_)
            , ndnboost::move(v5_)
            , ndnboost::move(v6_)
            , ndnboost::move(v7_)
            , ndnboost::move(v8_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
    class invoker<Fp, T0, T1, T2, T3, T4, T5, T6, T7>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
      T4 v4_;
      T5 v5_;
      T6 v6_;
      T7 v7_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3, T4, T5, T6, T7)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
          , NDNBOOST_THREAD_RV_REF(T4) a4
          , NDNBOOST_THREAD_RV_REF(T5) a5
          , NDNBOOST_THREAD_RV_REF(T6) a6
          , NDNBOOST_THREAD_RV_REF(T7) a7
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      , v4_(ndnboost::move(a4))
      , v5_(ndnboost::move(a5))
      , v6_(ndnboost::move(a6))
      , v7_(ndnboost::move(a7))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      , v4_(ndnboost::move(NDNBOOST_THREAD_RV(f).v4_))
      , v5_(ndnboost::move(NDNBOOST_THREAD_RV(f).v5_))
      , v6_(ndnboost::move(NDNBOOST_THREAD_RV(f).v6_))
      , v7_(ndnboost::move(NDNBOOST_THREAD_RV(f).v7_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
            , ndnboost::move(v4_)
            , ndnboost::move(v5_)
            , ndnboost::move(v6_)
            , ndnboost::move(v7_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
    class invoker<Fp, T0, T1, T2, T3, T4, T5, T6>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
      T4 v4_;
      T5 v5_;
      T6 v6_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3, T4, T5, T6)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
          , NDNBOOST_THREAD_RV_REF(T4) a4
          , NDNBOOST_THREAD_RV_REF(T5) a5
          , NDNBOOST_THREAD_RV_REF(T6) a6
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      , v4_(ndnboost::move(a4))
      , v5_(ndnboost::move(a5))
      , v6_(ndnboost::move(a6))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      , v4_(ndnboost::move(NDNBOOST_THREAD_RV(f).v4_))
      , v5_(ndnboost::move(NDNBOOST_THREAD_RV(f).v5_))
      , v6_(ndnboost::move(NDNBOOST_THREAD_RV(f).v6_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
            , ndnboost::move(v4_)
            , ndnboost::move(v5_)
            , ndnboost::move(v6_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2, class T3, class T4, class T5>
    class invoker<Fp, T0, T1, T2, T3, T4, T5>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
      T4 v4_;
      T5 v5_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3, T4, T5)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
          , NDNBOOST_THREAD_RV_REF(T4) a4
          , NDNBOOST_THREAD_RV_REF(T5) a5
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      , v4_(ndnboost::move(a4))
      , v5_(ndnboost::move(a5))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      , v4_(ndnboost::move(NDNBOOST_THREAD_RV(f).v4_))
      , v5_(ndnboost::move(NDNBOOST_THREAD_RV(f).v5_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
            , ndnboost::move(v4_)
            , ndnboost::move(v5_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2, class T3, class T4>
    class invoker<Fp, T0, T1, T2, T3, T4>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
      T4 v4_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3, T4)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
          , NDNBOOST_THREAD_RV_REF(T4) a4
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      , v4_(ndnboost::move(a4))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      , v4_(ndnboost::move(NDNBOOST_THREAD_RV(f).v4_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
            , ndnboost::move(v4_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2, class T3>
    class invoker<Fp, T0, T1, T2, T3>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
      T3 v3_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2, T3)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
          , NDNBOOST_THREAD_RV_REF(T3) a3
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      , v3_(ndnboost::move(a3))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      , v3_(ndnboost::move(NDNBOOST_THREAD_RV(f).v3_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
            , ndnboost::move(v3_)
        );
      }
    };
    template <class Fp, class T0, class T1, class T2>
    class invoker<Fp, T0, T1, T2>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
      T2 v2_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1, T2)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
          , NDNBOOST_THREAD_RV_REF(T2) a2
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      , v2_(ndnboost::move(a2))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      , v2_(ndnboost::move(NDNBOOST_THREAD_RV(f).v2_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
            , ndnboost::move(v2_)
        );
      }
    };
    template <class Fp, class T0, class T1>
    class invoker<Fp, T0, T1>
    {
      Fp fp_;
      T0 v0_;
      T1 v1_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0, T1)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
          , NDNBOOST_THREAD_RV_REF(T1) a1
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      , v1_(ndnboost::move(a1))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      , v1_(ndnboost::move(NDNBOOST_THREAD_RV(f).v1_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
            , ndnboost::move(v1_)
        );
      }
    };
    template <class Fp, class T0>
    class invoker<Fp, T0>
    {
      Fp fp_;
      T0 v0_;
    public:
      NDNBOOST_THREAD_MOVABLE_ONLY(invoker)
      typedef typename result_of<Fp(T0)>::type result_type;

      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f
          , NDNBOOST_THREAD_RV_REF(T0) a0
      )
      : fp_(ndnboost::move(f))
      , v0_(ndnboost::move(a0))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(NDNBOOST_THREAD_RV(f).fp))
      , v0_(ndnboost::move(NDNBOOST_THREAD_RV(f).v0_))
      {}

      result_type operator()()
      {
        return invoke(ndnboost::move(fp_)
            , ndnboost::move(v0_)
        );
      }
    };
    template <class Fp>
    class invoker<Fp>
    {
      Fp fp_;
    public:
      NDNBOOST_THREAD_COPYABLE_AND_MOVABLE(invoker)
      typedef typename result_of<Fp()>::type result_type;
      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(NDNBOOST_THREAD_FWD_REF(Fp) f)
      : fp_(ndnboost::move(f))
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(ndnboost::move(f.fp_))
      {}
      result_type operator()()
      {
        return fp_();
      }
    };
    template <class R>
    class invoker<R(*)()>
    {
      typedef R(*Fp)();
      Fp fp_;
    public:
      NDNBOOST_THREAD_COPYABLE_AND_MOVABLE(invoker)
      typedef typename result_of<Fp()>::type result_type;
      NDNBOOST_SYMBOL_VISIBLE
      explicit invoker(Fp f)
      : fp_(f)
      {}

      NDNBOOST_SYMBOL_VISIBLE
      invoker(NDNBOOST_THREAD_FWD_REF(invoker) f)
      : fp_(f.fp_)
      {}
      result_type operator()()
      {
        return fp_();
      }
    };
#endif
#endif

  }
}

#include <ndnboost/thread/detail/variadic_footer.hpp>

#endif // header
