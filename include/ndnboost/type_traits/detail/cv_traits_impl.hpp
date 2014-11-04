
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef NDNBOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
#define NDNBOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED

#include <cstddef>
#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>


// implementation helper:


namespace ndnboost {
namespace detail {

#if NDNBOOST_WORKAROUND(NDNBOOST_MSVC, == 1700)
#define NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(X) X
   template <typename T>
   struct cv_traits_imp
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type;
   };

   template <typename T>
   struct cv_traits_imp<T[]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<const T[]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<volatile T[]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<const volatile T[]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<T[N]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<const T[N]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<volatile T[N]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<const volatile T[N]>
   {
      NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
      NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[N];
   };

#else
#define NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(X) X *
template <typename T> struct cv_traits_imp {};

template <typename T>
struct cv_traits_imp<T*>
{
    NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
    NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
    typedef T unqualified_type;
};
#endif

template <typename T>
struct cv_traits_imp<NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(const T)>
{
    NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
    NDNBOOST_STATIC_CONSTANT(bool, is_volatile = false);
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(volatile T)>
{
    NDNBOOST_STATIC_CONSTANT(bool, is_const = false);
    NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(const volatile T)>
{
    NDNBOOST_STATIC_CONSTANT(bool, is_const = true);
    NDNBOOST_STATIC_CONSTANT(bool, is_volatile = true);
    typedef T unqualified_type;
};

} // namespace detail
} // namespace ndnboost 


#endif // NDNBOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
