
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, 
//      Howard Hinnant and John Maddock 2000. 
//  (C) Copyright Mat Marcus, Jesse Jones and Adobe Systems Inc 2001

//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

//    Fixed is_pointer, is_reference, is_const, is_volatile, is_same, 
//    is_member_pointer based on the Simulated Partial Specialization work 
//    of Mat Marcus and Jesse Jones. See  http://opensource.adobe.com or 
//    http://groups.yahoo.com/group/boost/message/5441 
//    Some workarounds in here use ideas suggested from "Generic<Programming>: 
//    Mappings between Types and Values" 
//    by Andrei Alexandrescu (see http://www.cuj.com/experts/1810/alexandr.html).


#ifndef NDNBOOST_TT_IS_VOLATILE_HPP_INCLUDED
#define NDNBOOST_TT_IS_VOLATILE_HPP_INCLUDED

#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>

#   include <ndnboost/type_traits/detail/cv_traits_impl.hpp>
#   if NDNBOOST_WORKAROUND(NDNBOOST_MSVC, < 1400)
#       include <ndnboost/type_traits/remove_bounds.hpp>
#   endif

// should be the last #include
#include <ndnboost/type_traits/detail/bool_trait_def.hpp>

namespace ndnboost {

namespace detail{
template <class T>
struct is_volatile_rval_filter
{
#if NDNBOOST_WORKAROUND(NDNBOOST_MSVC, < 1400)
   NDNBOOST_STATIC_CONSTANT(bool, value = ::ndnboost::detail::cv_traits_imp<typename ndnboost::remove_bounds<T>::type*>::is_volatile);
#else
   NDNBOOST_STATIC_CONSTANT(bool, value = ::ndnboost::detail::cv_traits_imp<NDNBOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(T)>::is_volatile);
#endif
};
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
//
// We can't filter out rvalue_references at the same level as
// references or we get ambiguities from msvc:
//
template <class T>
struct is_volatile_rval_filter<T&&>
{
   NDNBOOST_STATIC_CONSTANT(bool, value = false);
};
#endif
}

#if defined( __CODEGEARC__ )
NDNBOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,__is_volatile(T))
#else

//* is a type T declared volatile - is_volatile<T>
NDNBOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,::ndnboost::detail::is_volatile_rval_filter<T>::value)
NDNBOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T&,false)

#if  defined(NDNBOOST_ILLEGAL_CV_REFERENCES)
// these are illegal specialisations; cv-qualifies applied to
// references have no effect according to [8.3.2p1],
// C++ Builder requires them though as it treats cv-qualified
// references as distinct types...
NDNBOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& const,false)
NDNBOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& volatile,false)
NDNBOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& const volatile,false)
#endif

#endif

} // namespace ndnboost

#include <ndnboost/type_traits/detail/bool_trait_undef.hpp>

#endif // NDNBOOST_TT_IS_VOLATILE_HPP_INCLUDED
