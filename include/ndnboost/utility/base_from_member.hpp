//  boost utility/base_from_member.hpp header file  --------------------------//

//  Copyright 2001, 2003, 2004, 2012 Daryle Walker.  Use, modification, and
//  distribution are subject to the Boost Software License, Version 1.0.  (See
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/utility/> for the library's home page.

#ifndef NDNBOOST_UTILITY_BASE_FROM_MEMBER_HPP
#define NDNBOOST_UTILITY_BASE_FROM_MEMBER_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/preprocessor/arithmetic/inc.hpp>
#include <ndnboost/preprocessor/repetition/enum_binary_params.hpp>
#include <ndnboost/preprocessor/repetition/enum_params.hpp>
#include <ndnboost/preprocessor/repetition/repeat_from_to.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/remove_cv.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>
#include <ndnboost/utility/enable_if.hpp>


//  Base-from-member arity configuration macro  ------------------------------//

// The following macro determines how many arguments will be in the largest
// constructor template of base_from_member.  Constructor templates will be
// generated from one argument to this maximum.  Code from other files can read
// this number if they need to always match the exact maximum base_from_member
// uses.  The maximum constructor length can be changed by overriding the
// #defined constant.  Make sure to apply the override, if any, for all source
// files during project compiling for consistency.

// Contributed by Jonathan Turkanis

#ifndef NDNBOOST_BASE_FROM_MEMBER_MAX_ARITY
#define NDNBOOST_BASE_FROM_MEMBER_MAX_ARITY  10
#endif


//  An iteration of a constructor template for base_from_member  -------------//

// A macro that should expand to:
//     template < typename T1, ..., typename Tn >
//     base_from_member( T1 x1, ..., Tn xn )
//         : member( x1, ..., xn )
//         {}
// This macro should only persist within this file.

#define NDNBOOST_PRIVATE_CTR_DEF( z, n, data )                            \
    template < NDNBOOST_PP_ENUM_PARAMS(n, typename T) >                   \
    explicit base_from_member( NDNBOOST_PP_ENUM_BINARY_PARAMS(n, T, x) )  \
        : member( NDNBOOST_PP_ENUM_PARAMS(n, x) )                         \
        {}                                                             \
    /**/


namespace ndnboost
{

namespace detail
{

//  Type-unmarking class template  -------------------------------------------//

// Type-trait to get the raw type, i.e. the type without top-level reference nor
// cv-qualification, from a type expression.  Mainly for function arguments, any
// reference part is stripped first.

// Contributed by Daryle Walker

template < typename T >
struct remove_cv_ref
{
    typedef typename ::ndnboost::remove_cv<typename
     ::ndnboost::remove_reference<T>::type>::type  type;

};  // ndnboost::detail::remove_cv_ref

//  Unmarked-type comparison class template  ---------------------------------//

// Type-trait to check if two type expressions have the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

template < typename T, typename U >
struct is_related
    : public ::ndnboost::is_same<
     typename ::ndnboost::detail::remove_cv_ref<T>::type,
     typename ::ndnboost::detail::remove_cv_ref<U>::type >
{};

//  Enable-if-on-unidentical-unmarked-type class template  -------------------//

// Enable-if on the first two type expressions NOT having the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

#ifndef NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES
template<typename ...T>
struct enable_if_unrelated
    : public ::ndnboost::enable_if_c<true>
{};

template<typename T, typename U, typename ...U2>
struct enable_if_unrelated<T, U, U2...>
    : public ::ndnboost::disable_if< ::ndnboost::detail::is_related<T, U> >
{};
#endif

}  // namespace ndnboost::detail


//  Base-from-member class template  -----------------------------------------//

// Helper to initialize a base object so a derived class can use this
// object in the initialization of another base class.  Used by
// Dietmar Kuehl from ideas by Ron Klatcho to solve the problem of a
// base class needing to be initialized by a member.

// Contributed by Daryle Walker

template < typename MemberType, int UniqueID = 0 >
class base_from_member
{
protected:
    MemberType  member;

#if !defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && \
    !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    !defined(NDNBOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS) && \
    !(defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 4))
    template <typename ...T, typename EnableIf = typename
     ::ndnboost::detail::enable_if_unrelated<base_from_member, T...>::type>
    explicit NDNBOOST_CONSTEXPR base_from_member( T&& ...x )
        NDNBOOST_NOEXCEPT_IF( NDNBOOST_NOEXCEPT_EXPR(::new ((void*) 0) MemberType(
         static_cast<T&&>(x)... )) )  // no std::is_nothrow_constructible...
        : member( static_cast<T&&>(x)... )     // ...nor std::forward needed
        {}
#else
    base_from_member()
        : member()
        {}

    NDNBOOST_PP_REPEAT_FROM_TO( 1, NDNBOOST_PP_INC(NDNBOOST_BASE_FROM_MEMBER_MAX_ARITY),
     NDNBOOST_PRIVATE_CTR_DEF, _ )
#endif

};  // ndnboost::base_from_member

template < typename MemberType, int UniqueID >
class base_from_member<MemberType&, UniqueID>
{
protected:
    MemberType& member;

    explicit NDNBOOST_CONSTEXPR base_from_member( MemberType& x )
        NDNBOOST_NOEXCEPT
        : member( x )
        {}

};  // ndnboost::base_from_member

}  // namespace ndnboost


// Undo any private macros
#undef NDNBOOST_PRIVATE_CTR_DEF


#endif  // NDNBOOST_UTILITY_BASE_FROM_MEMBER_HPP
