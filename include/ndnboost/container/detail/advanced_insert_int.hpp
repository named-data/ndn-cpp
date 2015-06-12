//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2008-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_CONTAINER_ADVANCED_INSERT_INT_HPP
#define NDNBOOST_CONTAINER_ADVANCED_INSERT_INT_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

#include <ndnboost/container/detail/config_begin.hpp>
#include <ndnboost/container/detail/workaround.hpp>

#include <ndnboost/container/allocator_traits.hpp>
#include <ndnboost/container/detail/destroyers.hpp>
#include <ndnboost/aligned_storage.hpp>
#include <ndnboost/move/utility.hpp>
#include <iterator>  //std::iterator_traits
#include <ndnboost/assert.hpp>
#include <ndnboost/detail/no_exceptions_support.hpp>

namespace ndnboost { namespace container { namespace container_detail {

template<class A, class FwdIt, class Iterator>
struct move_insert_range_proxy
{
   typedef typename allocator_traits<A>::size_type size_type;
   typedef typename allocator_traits<A>::value_type value_type;

   explicit move_insert_range_proxy(FwdIt first)
      :  first_(first)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n)
   {
      this->first_ = ::ndnboost::container::uninitialized_move_alloc_n_source
         (a, this->first_, n, p);
   }

   void copy_n_and_update(A &, Iterator p, size_type n)
   {
      this->first_ = ::ndnboost::container::move_n_source(this->first_, n, p);
   }

   FwdIt first_;
};


template<class A, class FwdIt, class Iterator>
struct insert_range_proxy
{
   typedef typename allocator_traits<A>::size_type size_type;
   typedef typename allocator_traits<A>::value_type value_type;

   explicit insert_range_proxy(FwdIt first)
      :  first_(first)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n)
   {
      this->first_ = ::ndnboost::container::uninitialized_copy_alloc_n_source(a, this->first_, n, p);
   }

   void copy_n_and_update(A &, Iterator p, size_type n)
   {
      this->first_ = ::ndnboost::container::copy_n_source(this->first_, n, p);
   }

   FwdIt first_;
};


template<class A, class Iterator>
struct insert_n_copies_proxy
{
   typedef typename allocator_traits<A>::size_type size_type;
   typedef typename allocator_traits<A>::value_type value_type;

   explicit insert_n_copies_proxy(const value_type &v)
      :  v_(v)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n) const
   {  ndnboost::container::uninitialized_fill_alloc_n(a, v_, n, p);  }

   void copy_n_and_update(A &, Iterator p, size_type n) const
   {  std::fill_n(p, n, v_);  }

   const value_type &v_;
};

template<class A, class Iterator>
struct insert_value_initialized_n_proxy
{
   typedef ::ndnboost::container::allocator_traits<A> alloc_traits;
   typedef typename allocator_traits<A>::size_type size_type;
   typedef typename allocator_traits<A>::value_type value_type;

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n) const
   {  ndnboost::container::uninitialized_value_init_alloc_n(a, n, p);  }

   void copy_n_and_update(A &, Iterator, size_type) const
   {  NDNBOOST_ASSERT(false); }
};

template<class A, class Iterator>
struct insert_default_initialized_n_proxy
{
   typedef ::ndnboost::container::allocator_traits<A> alloc_traits;
   typedef typename allocator_traits<A>::size_type size_type;
   typedef typename allocator_traits<A>::value_type value_type;

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n) const
   {  ndnboost::container::uninitialized_default_init_alloc_n(a, n, p);  }

   void copy_n_and_update(A &, Iterator, size_type) const
   {  NDNBOOST_ASSERT(false); }
};

template<class A, class Iterator>
struct insert_copy_proxy
{
   typedef ndnboost::container::allocator_traits<A> alloc_traits;
   typedef typename alloc_traits::size_type size_type;
   typedef typename alloc_traits::value_type value_type;

   explicit insert_copy_proxy(const value_type &v)
      :  v_(v)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n) const
   {
      NDNBOOST_ASSERT(n == 1);  (void)n;
      alloc_traits::construct( a, iterator_to_raw_pointer(p), v_);
   }

   void copy_n_and_update(A &, Iterator p, size_type n) const
   {
      NDNBOOST_ASSERT(n == 1);  (void)n;
      *p =v_;
   }

   const value_type &v_;
};


template<class A, class Iterator>
struct insert_move_proxy
{
   typedef ndnboost::container::allocator_traits<A> alloc_traits;
   typedef typename alloc_traits::size_type size_type;
   typedef typename alloc_traits::value_type value_type;

   explicit insert_move_proxy(value_type &v)
      :  v_(v)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n) const
   {
      NDNBOOST_ASSERT(n == 1);  (void)n;
      alloc_traits::construct( a, iterator_to_raw_pointer(p), ::ndnboost::move(v_) );
   }

   void copy_n_and_update(A &, Iterator p, size_type n) const
   {
      NDNBOOST_ASSERT(n == 1);  (void)n;
      *p = ::ndnboost::move(v_);
   }

   value_type &v_;
};

template<class It, class A>
insert_move_proxy<A, It> get_insert_value_proxy(NDNBOOST_RV_REF(typename std::iterator_traits<It>::value_type) v)
{
   return insert_move_proxy<A, It>(v);
}

template<class It, class A>
insert_copy_proxy<A, It> get_insert_value_proxy(const typename std::iterator_traits<It>::value_type &v)
{
   return insert_copy_proxy<A, It>(v);
}

}}}   //namespace ndnboost { namespace container { namespace container_detail {

#ifdef NDNBOOST_CONTAINER_PERFECT_FORWARDING

#include <ndnboost/container/detail/variadic_templates_tools.hpp>
#include <ndnboost/move/utility.hpp>
#include <typeinfo>
//#include <iostream> //For debugging purposes

namespace ndnboost {
namespace container {
namespace container_detail {

template<class A, class Iterator, class ...Args>
struct insert_non_movable_emplace_proxy
{
   typedef ndnboost::container::allocator_traits<A>   alloc_traits;
   typedef typename alloc_traits::size_type        size_type;
   typedef typename alloc_traits::value_type       value_type;

   typedef typename build_number_seq<sizeof...(Args)>::type index_tuple_t;

   explicit insert_non_movable_emplace_proxy(Args&&... args)
      : args_(args...)
   {}

   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n)
   {  this->priv_uninitialized_copy_some_and_update(a, index_tuple_t(), p, n);  }

   private:
   template<int ...IdxPack>
   void priv_uninitialized_copy_some_and_update(A &a, const index_tuple<IdxPack...>&, Iterator p, size_type n)
   {
      NDNBOOST_ASSERT(n == 1); (void)n;
      alloc_traits::construct( a, iterator_to_raw_pointer(p), ::ndnboost::forward<Args>(get<IdxPack>(this->args_))... );
   }

   protected:
   tuple<Args&...> args_;
};

template<class A, class Iterator, class ...Args>
struct insert_emplace_proxy
   :  public insert_non_movable_emplace_proxy<A, Iterator, Args...>
{
   typedef insert_non_movable_emplace_proxy<A, Iterator, Args...> base_t;
   typedef ndnboost::container::allocator_traits<A>   alloc_traits;
   typedef typename base_t::value_type             value_type;
   typedef typename base_t::size_type              size_type;
   typedef typename base_t::index_tuple_t          index_tuple_t;

   explicit insert_emplace_proxy(Args&&... args)
      : base_t(::ndnboost::forward<Args>(args)...)
   {}

   void copy_n_and_update(A &a, Iterator p, size_type n)
   {  this->priv_copy_some_and_update(a, index_tuple_t(), p, n);  }

   private:

   template<int ...IdxPack>
   void priv_copy_some_and_update(A &a, const index_tuple<IdxPack...>&, Iterator p, size_type n)
   {
      NDNBOOST_ASSERT(n ==1); (void)n;
      aligned_storage<sizeof(value_type), alignment_of<value_type>::value> v;
      value_type *vp = static_cast<value_type *>(static_cast<void *>(&v));
      alloc_traits::construct(a, vp,
         ::ndnboost::forward<Args>(get<IdxPack>(this->args_))...);
      NDNBOOST_TRY{
         *p = ::ndnboost::move(*vp);
      }
      NDNBOOST_CATCH(...){
         alloc_traits::destroy(a, vp);
         NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
      alloc_traits::destroy(a, vp);
   }
};

}}}   //namespace ndnboost { namespace container { namespace container_detail {

#else //#ifdef NDNBOOST_CONTAINER_PERFECT_FORWARDING

#include <ndnboost/container/detail/preprocessor.hpp>
#include <ndnboost/container/detail/value_init.hpp>

namespace ndnboost {
namespace container {
namespace container_detail {

#define NDNBOOST_PP_LOCAL_MACRO(N)                                                     \
template<class A, class Iterator NDNBOOST_PP_ENUM_TRAILING_PARAMS(N, class P) >        \
struct NDNBOOST_PP_CAT(insert_non_movable_emplace_proxy_arg, N)                        \
{                                                                                   \
   typedef ndnboost::container::allocator_traits<A> alloc_traits;                      \
   typedef typename alloc_traits::size_type size_type;                              \
   typedef typename alloc_traits::value_type value_type;                            \
                                                                                    \
   explicit NDNBOOST_PP_CAT(insert_non_movable_emplace_proxy_arg, N)                   \
      ( NDNBOOST_PP_ENUM(N, NDNBOOST_CONTAINER_PP_PARAM_LIST, _) )                        \
      NDNBOOST_PP_EXPR_IF(N, :) NDNBOOST_PP_ENUM(N, NDNBOOST_CONTAINER_PP_PARAM_INIT, _)     \
   {}                                                                               \
                                                                                    \
   void uninitialized_copy_n_and_update(A &a, Iterator p, size_type n)              \
   {                                                                                \
      NDNBOOST_ASSERT(n == 1); (void)n;                                                \
      alloc_traits::construct                                                       \
         ( a, iterator_to_raw_pointer(p)                                            \
         NDNBOOST_PP_ENUM_TRAILING(N, NDNBOOST_CONTAINER_PP_MEMBER_FORWARD, _)            \
         );                                                                         \
   }                                                                                \
                                                                                    \
   void copy_n_and_update(A &, Iterator, size_type)                                 \
   {  NDNBOOST_ASSERT(false);   }                                                      \
                                                                                    \
   protected:                                                                       \
   NDNBOOST_PP_REPEAT(N, NDNBOOST_CONTAINER_PP_PARAM_DEFINE, _)                           \
};                                                                                  \
                                                                                    \
template<class A, class Iterator NDNBOOST_PP_ENUM_TRAILING_PARAMS(N, class P) >        \
struct NDNBOOST_PP_CAT(insert_emplace_proxy_arg, N)                                    \
   : NDNBOOST_PP_CAT(insert_non_movable_emplace_proxy_arg, N)                          \
         < A, Iterator NDNBOOST_PP_ENUM_TRAILING_PARAMS(N, P) >                        \
{                                                                                   \
   typedef NDNBOOST_PP_CAT(insert_non_movable_emplace_proxy_arg, N)                    \
         <A, Iterator NDNBOOST_PP_ENUM_TRAILING_PARAMS(N, P) > base_t;                 \
   typedef typename base_t::value_type       value_type;                            \
   typedef typename base_t::size_type  size_type;                                   \
   typedef ndnboost::container::allocator_traits<A> alloc_traits;                      \
                                                                                    \
   explicit NDNBOOST_PP_CAT(insert_emplace_proxy_arg, N)                               \
      ( NDNBOOST_PP_ENUM(N, NDNBOOST_CONTAINER_PP_PARAM_LIST, _) )                        \
      : base_t(NDNBOOST_PP_ENUM(N, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _) )              \
   {}                                                                               \
                                                                                    \
   void copy_n_and_update(A &a, Iterator p, size_type n)                            \
   {                                                                                \
      NDNBOOST_ASSERT(n == 1); (void)n;                                                \
      aligned_storage<sizeof(value_type), alignment_of<value_type>::value> v;       \
      value_type *vp = static_cast<value_type *>(static_cast<void *>(&v));          \
      alloc_traits::construct(a, vp                                                 \
         NDNBOOST_PP_ENUM_TRAILING(N, NDNBOOST_CONTAINER_PP_MEMBER_FORWARD, _));          \
      NDNBOOST_TRY{                                                                    \
         *p = ::ndnboost::move(*vp);                                                   \
      }                                                                             \
      NDNBOOST_CATCH(...){                                                             \
         alloc_traits::destroy(a, vp);                                              \
         NDNBOOST_RETHROW                                                              \
      }                                                                             \
      NDNBOOST_CATCH_END                                                               \
      alloc_traits::destroy(a, vp);                                                 \
   }                                                                                \
};                                                                                  \
//!
#define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
#include NDNBOOST_PP_LOCAL_ITERATE()

}}}   //namespace ndnboost { namespace container { namespace container_detail {

#endif   //#ifdef NDNBOOST_CONTAINER_PERFECT_FORWARDING

#include <ndnboost/container/detail/config_end.hpp>

#endif //#ifndef NDNBOOST_CONTAINER_ADVANCED_INSERT_INT_HPP
