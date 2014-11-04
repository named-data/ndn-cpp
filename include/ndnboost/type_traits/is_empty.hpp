
// (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef NDNBOOST_TT_IS_EMPTY_HPP_INCLUDED
#define NDNBOOST_TT_IS_EMPTY_HPP_INCLUDED

#include <ndnboost/type_traits/is_convertible.hpp>
#include <ndnboost/type_traits/detail/ice_or.hpp>
#include <ndnboost/type_traits/config.hpp>
#include <ndnboost/type_traits/intrinsics.hpp>

#   include <ndnboost/type_traits/remove_cv.hpp>
#   include <ndnboost/type_traits/is_class.hpp>
#   include <ndnboost/type_traits/add_reference.hpp>

// should be always the last #include directive
#include <ndnboost/type_traits/detail/bool_trait_def.hpp>

#ifndef NDNBOOST_INTERNAL_IS_EMPTY
#define NDNBOOST_INTERNAL_IS_EMPTY(T) false
#else
#define NDNBOOST_INTERNAL_IS_EMPTY(T) NDNBOOST_IS_EMPTY(T)
#endif

namespace ndnboost {

namespace detail {


#ifdef NDNBOOST_MSVC
#pragma warning(push)
#pragma warning(disable:4624) // destructor could not be generated
#endif

template <typename T>
struct empty_helper_t1 : public T
{
    empty_helper_t1();  // hh compiler bug workaround
    int i[256];
private:
   // suppress compiler warnings:
   empty_helper_t1(const empty_helper_t1&);
   empty_helper_t1& operator=(const empty_helper_t1&);
};

#ifdef NDNBOOST_MSVC
#pragma warning(pop)
#endif

struct empty_helper_t2 { int i[256]; };

#if !NDNBOOST_WORKAROUND(__BORLANDC__, < 0x600)

template <typename T, bool is_a_class = false>
struct empty_helper
{
    NDNBOOST_STATIC_CONSTANT(bool, value = false);
};

template <typename T>
struct empty_helper<T, true>
{
    NDNBOOST_STATIC_CONSTANT(
        bool, value = (sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2))
        );
};

template <typename T>
struct is_empty_impl
{
    typedef typename remove_cv<T>::type cvt;
    NDNBOOST_STATIC_CONSTANT(
        bool, value = (
            ::ndnboost::type_traits::ice_or<
              ::ndnboost::detail::empty_helper<cvt,::ndnboost::is_class<T>::value>::value
              , NDNBOOST_INTERNAL_IS_EMPTY(cvt)
            >::value
            ));
};

#else // __BORLANDC__

template <typename T, bool is_a_class, bool convertible_to_int>
struct empty_helper
{
    NDNBOOST_STATIC_CONSTANT(bool, value = false);
};

template <typename T>
struct empty_helper<T, true, false>
{
    NDNBOOST_STATIC_CONSTANT(bool, value = (
        sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2)
        ));
};

template <typename T>
struct is_empty_impl
{
   typedef typename remove_cv<T>::type cvt;
   typedef typename add_reference<T>::type r_type;

   NDNBOOST_STATIC_CONSTANT(
       bool, value = (
           ::ndnboost::type_traits::ice_or<
              ::ndnboost::detail::empty_helper<
                  cvt
                , ::ndnboost::is_class<T>::value
                , ::ndnboost::is_convertible< r_type,int>::value
              >::value
              , NDNBOOST_INTERNAL_IS_EMPTY(cvt)
           >::value));
};

#endif // __BORLANDC__


// these help when the compiler has no partial specialization support:
NDNBOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void,false)
#ifndef NDNBOOST_NO_CV_VOID_SPECIALIZATIONS
NDNBOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void const,false)
NDNBOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void volatile,false)
NDNBOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_empty,void const volatile,false)
#endif

} // namespace detail

NDNBOOST_TT_AUX_BOOL_TRAIT_DEF1(is_empty,T,::ndnboost::detail::is_empty_impl<T>::value)

} // namespace ndnboost

#include <ndnboost/type_traits/detail/bool_trait_undef.hpp>

#undef NDNBOOST_INTERNAL_IS_EMPTY

#endif // NDNBOOST_TT_IS_EMPTY_HPP_INCLUDED

