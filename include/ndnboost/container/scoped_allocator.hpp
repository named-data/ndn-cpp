//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Pablo Halpern 2009. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP
#define NDNBOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP

#if defined (_MSC_VER)
#  pragma once
#endif

#include <ndnboost/container/detail/config_begin.hpp>
#include <ndnboost/container/detail/workaround.hpp>
#include <ndnboost/container/scoped_allocator_fwd.hpp>
#include <ndnboost/type_traits/integral_constant.hpp>
#include <ndnboost/container/allocator_traits.hpp>
#include <ndnboost/container/detail/type_traits.hpp>
#include <ndnboost/container/detail/utilities.hpp>
#include <utility>
#include <ndnboost/container/detail/pair.hpp>
#include <ndnboost/move/utility.hpp>
#include <ndnboost/detail/no_exceptions_support.hpp>

namespace ndnboost { namespace container {

//! <b>Remark</b>: if a specialization is derived from true_type, indicates that T may be constructed
//! with an allocator as its last constructor argument.  Ideally, all constructors of T (including the
//! copy and move constructors) should have a variant that accepts a final argument of
//! allocator_type.
//!
//! <b>Requires</b>: if a specialization is derived from true_type, T must have a nested type,
//! allocator_type and at least one constructor for which allocator_type is the last
//! parameter.  If not all constructors of T can be called with a final allocator_type argument,
//! and if T is used in a context where a container must call such a constructor, then the program is
//! ill-formed.
//!
//! <code>
//!  template <class T, class Allocator = allocator<T> >
//!  class Z {
//!    public:
//!      typedef Allocator allocator_type;
//!
//!    // Default constructor with optional allocator suffix
//!    Z(const allocator_type& a = allocator_type());
//!
//!    // Copy constructor and allocator-extended copy constructor
//!    Z(const Z& zz);
//!    Z(const Z& zz, const allocator_type& a);
//! };
//!
//! // Specialize trait for class template Z
//! template <class T, class Allocator = allocator<T> >
//! struct constructible_with_allocator_suffix<Z<T,Allocator> >
//!      : ::ndnboost::true_type { };
//! </code>
//!
//! <b>Note</b>: This trait is a workaround inspired by "N2554: The Scoped Allocator Model (Rev 2)"
//! (Pablo Halpern, 2008-02-29) to backport the scoped allocator model to C++03, as
//! in C++03 there is no mechanism to detect if a type can be constructed from arbitrary arguments.
//! Applications aiming portability with several compilers should always define this trait.
//!
//! In conforming C++11 compilers or compilers supporting SFINAE expressions
//! (when NDNBOOST_NO_SFINAE_EXPR is NOT defined), this trait is ignored and C++11 rules will be used
//! to detect if a type should be constructed with suffix or prefix allocator arguments.
template <class T>
struct constructible_with_allocator_suffix
   : ::ndnboost::false_type
{};

//! <b>Remark</b>: if a specialization is derived from true_type, indicates that T may be constructed
//! with allocator_arg and T::allocator_type as its first two constructor arguments.
//! Ideally, all constructors of T (including the copy and move constructors) should have a variant
//! that accepts these two initial arguments.
//!
//! <b>Requires</b>: if a specialization is derived from true_type, T must have a nested type,
//! allocator_type and at least one constructor for which allocator_arg_t is the first
//! parameter and allocator_type is the second parameter.  If not all constructors of T can be
//! called with these initial arguments, and if T is used in a context where a container must call such
//! a constructor, then the program is ill-formed.
//!
//! <code>
//! template <class T, class Allocator = allocator<T> >
//! class Y {
//!    public:
//!       typedef Allocator allocator_type;
//!
//!       // Default constructor with and allocator-extended default constructor
//!       Y();
//!       Y(allocator_arg_t, const allocator_type& a);
//!
//!       // Copy constructor and allocator-extended copy constructor
//!       Y(const Y& yy);
//!       Y(allocator_arg_t, const allocator_type& a, const Y& yy);
//!
//!       // Variadic constructor and allocator-extended variadic constructor
//!       template<class ...Args> Y(Args&& args...);
//!       template<class ...Args>
//!       Y(allocator_arg_t, const allocator_type& a, Args&&... args);
//! };
//!
//! // Specialize trait for class template Y
//! template <class T, class Allocator = allocator<T> >
//! struct constructible_with_allocator_prefix<Y<T,Allocator> >
//!       : ::ndnboost::true_type { };
//!
//! </code>
//!
//! <b>Note</b>: This trait is a workaround inspired by "N2554: The Scoped Allocator Model (Rev 2)"
//! (Pablo Halpern, 2008-02-29) to backport the scoped allocator model to C++03, as
//! in C++03 there is no mechanism to detect if a type can be constructed from arbitrary arguments.
//! Applications aiming portability with several compilers should always define this trait.
//!
//! In conforming C++11 compilers or compilers supporting SFINAE expressions
//! (when NDNBOOST_NO_SFINAE_EXPR is NOT defined), this trait is ignored and C++11 rules will be used
//! to detect if a type should be constructed with suffix or prefix allocator arguments.
template <class T>
struct constructible_with_allocator_prefix
    : ::ndnboost::false_type
{};

#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

namespace container_detail {

template<typename T, typename Alloc>
struct uses_allocator_imp
{
   // Use SFINAE (Substitution Failure Is Not An Error) to detect the
   // presence of an 'allocator_type' nested type convertilble from Alloc.

   private:
   // Match this function if TypeT::allocator_type exists and is
   // implicitly convertible from Alloc
   template <typename U>
   static char test(int, typename U::allocator_type);

   // Match this function if TypeT::allocator_type does not exist or is
   // not convertible from Alloc.
   template <typename U>
   static int test(LowPriorityConversion<int>, LowPriorityConversion<Alloc>);

   static Alloc alloc;  // Declared but not defined

   public:
   enum { value = sizeof(test<T>(0, alloc)) == sizeof(char) };
};

}  //namespace container_detail {

#endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

//! <b>Remark</b>: Automatically detects if T has a nested allocator_type that is convertible from
//! Alloc. Meets the BinaryTypeTrait requirements ([meta.rqmts] 20.4.1). A program may
//! specialize this type to derive from true_type for a T of user-defined type if T does not
//! have a nested allocator_type but is nonetheless constructible using the specified Alloc.
//!
//! <b>Result</b>: derived from true_type if Convertible<Alloc,T::allocator_type> and
//! derived from false_type otherwise.
template <typename T, typename Alloc>
struct uses_allocator
   : ndnboost::integral_constant<bool, container_detail::uses_allocator_imp<T, Alloc>::value>
{};

#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

namespace container_detail {

template <typename Alloc>
struct is_scoped_allocator_imp
{
   template <typename T>
   static char test(int, typename T::outer_allocator_type*);

   template <typename T>
   static int test(LowPriorityConversion<int>, void*);

   static const bool value = (sizeof(char) == sizeof(test<Alloc>(0, 0)));
};

template<class MaybeScopedAlloc, bool = is_scoped_allocator_imp<MaybeScopedAlloc>::value >
struct outermost_allocator_type_impl
{
   typedef typename MaybeScopedAlloc::outer_allocator_type outer_type;
   typedef typename outermost_allocator_type_impl<outer_type>::type type;
};

template<class MaybeScopedAlloc>
struct outermost_allocator_type_impl<MaybeScopedAlloc, false>
{
   typedef MaybeScopedAlloc type;
};

template<class MaybeScopedAlloc, bool = is_scoped_allocator_imp<MaybeScopedAlloc>::value >
struct outermost_allocator_imp
{
   typedef MaybeScopedAlloc type;

   static type &get(MaybeScopedAlloc &a)
   {  return a;  }

   static const type &get(const MaybeScopedAlloc &a)
   {  return a;  }
};

template<class MaybeScopedAlloc>
struct outermost_allocator_imp<MaybeScopedAlloc, true>
{
   typedef typename MaybeScopedAlloc::outer_allocator_type outer_type;
   typedef typename outermost_allocator_type_impl<outer_type>::type type;

   static type &get(MaybeScopedAlloc &a)
   {  return outermost_allocator_imp<outer_type>::get(a.outer_allocator());  }

   static const type &get(const MaybeScopedAlloc &a)
   {  return outermost_allocator_imp<outer_type>::get(a.outer_allocator());  }
};

}  //namespace container_detail {

template <typename Alloc>
struct is_scoped_allocator
   : ndnboost::integral_constant<bool, container_detail::is_scoped_allocator_imp<Alloc>::value>
{};

template <typename Alloc>
struct outermost_allocator
   : container_detail::outermost_allocator_imp<Alloc>
{};

template <typename Alloc>
typename container_detail::outermost_allocator_imp<Alloc>::type &
   get_outermost_allocator(Alloc &a)
{  return container_detail::outermost_allocator_imp<Alloc>::get(a);   }

template <typename Alloc>
const typename container_detail::outermost_allocator_imp<Alloc>::type &
   get_outermost_allocator(const Alloc &a)
{  return container_detail::outermost_allocator_imp<Alloc>::get(a);   }

namespace container_detail {

// Check if we can detect is_convertible using advanced SFINAE expressions
#if !defined(NDNBOOST_NO_SFINAE_EXPR)

   //! Code inspired by Mathias Gaunard's is_convertible.cpp found in the Boost mailing list
   //! http://boost.2283326.n4.nabble.com/type-traits-is-constructible-when-decltype-is-supported-td3575452.html
   //! Thanks Mathias!

   //With variadic templates, we need a single class to implement the trait
   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template<class T, class ...Args>
   struct is_constructible_impl
   {
      typedef char yes_type;
      struct no_type
      { char padding[2]; };

      template<std::size_t N>
      struct dummy;

      template<class X>
      static yes_type test(dummy<sizeof(X(ndnboost::move_detail::declval<Args>()...))>*);

      template<class X>
      static no_type test(...);

      static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
   };

   template<class T, class ...Args>
   struct is_constructible
      : ndnboost::integral_constant<bool, is_constructible_impl<T, Args...>::value>
   {};

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : is_constructible<T, allocator_arg_t, InnerAlloc, Args...>
   {};

   #else // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

   //Without variadic templates, we need to use de preprocessor to generate
   //some specializations.

   #define NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS \
      NDNBOOST_PP_ADD(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, 3)
   //!

   //Generate N+1 template parameters so that we can specialize N
   template<class T
            NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_PP_ADD(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, 1)
                                 , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                 , void)
         >
   struct is_constructible_impl;

   //Generate N specializations, from 0 to
   //NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS parameters
   #define NDNBOOST_PP_LOCAL_MACRO(n)                                                                 \
   template<class T NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)>                                     \
   struct is_constructible_impl                                                                    \
      <T NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, P)                                                       \
         NDNBOOST_PP_ENUM_TRAILING                                                                    \
            ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, n)                     \
            , NDNBOOST_CONTAINER_PP_IDENTITY, void)                                                   \
      , void>                                                                                      \
   {                                                                                               \
      typedef char yes_type;                                                                       \
      struct no_type                                                                               \
      { char padding[2]; };                                                                        \
                                                                                                   \
      template<std::size_t N>                                                                      \
      struct dummy;                                                                                \
                                                                                                   \
      template<class X>                                                                            \
      static yes_type test(dummy<sizeof(X(NDNBOOST_PP_ENUM(n, NDNBOOST_CONTAINER_PP_DECLVAL, ~)))>*);    \
                                                                                                   \
      template<class X>                                                                            \
      static no_type test(...);                                                                    \
                                                                                                   \
      static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);                            \
   };                                                                                              \
   //!

   #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS)
   #include NDNBOOST_PP_LOCAL_ITERATE()

   //Finally just inherit from the implementation to define he trait
   template< class T
           NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS
                                 , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                 , void)
           >
   struct is_constructible
      : ndnboost::integral_constant
         < bool
         , is_constructible_impl
            < T
            NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, P)
            , void>::value
         >
   {};

   //Finally just inherit from the implementation to define he trait
   template <class T
            ,class InnerAlloc
            NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, 2)
                                 , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                 , void)
            >
   struct is_constructible_with_allocator_prefix
      : is_constructible
         < T, allocator_arg_t, InnerAlloc
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, 2), P)
         >
   {};
/*
   template <class T
            ,class InnerAlloc
            NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, 1)
                                 , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                 , void)
            >
   struct is_constructible_with_allocator_suffix
      : is_constructible
         < T
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_IS_CONSTRUCTIBLE_PARAMETERS, 1), P)
         , InnerAlloc
         >
   {};*/

   #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

#else    // #if !defined(NDNBOOST_NO_SFINAE_EXPR)

   //Without advanced SFINAE expressions, we can't use is_constructible
   //so backup to constructible_with_allocator_xxx

   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template < class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};
/*
   template < class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};*/

   #else    // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template < class T
            , class InnerAlloc
            NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS
                                  , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                  , void)
            >
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};
/*
   template < class T
            , class InnerAlloc
            NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS
                                  , NDNBOOST_CONTAINER_PP_TEMPLATE_PARAM_WITH_DEFAULT
                                  , void)
            >
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};*/

   #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

#endif   // #if !defined(NDNBOOST_NO_SFINAE_EXPR)

#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_allocator_prefix_suffix
   ( ndnboost::true_type  use_alloc_prefix, OutermostAlloc& outermost_alloc
   , InnerAlloc& inner_alloc, T* p, NDNBOOST_FWD_REF(Args) ...args)
{
   (void)use_alloc_prefix;
   allocator_traits<OutermostAlloc>::construct
      ( outermost_alloc, p, allocator_arg, inner_alloc, ::ndnboost::forward<Args>(args)...);
}

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_allocator_prefix_suffix
   ( ndnboost::false_type use_alloc_prefix, OutermostAlloc& outermost_alloc
   , InnerAlloc &inner_alloc, T* p, NDNBOOST_FWD_REF(Args)...args)
{
   (void)use_alloc_prefix;
   allocator_traits<OutermostAlloc>::construct
      (outermost_alloc, p, ::ndnboost::forward<Args>(args)..., inner_alloc);
}

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_uses_allocator
   ( ndnboost::true_type uses_allocator, OutermostAlloc& outermost_alloc
   , InnerAlloc& inner_alloc, T* p, NDNBOOST_FWD_REF(Args)...args)
{
   (void)uses_allocator;
   //NDNBOOST_STATIC_ASSERT((is_constructible_with_allocator_prefix<T, InnerAlloc, Args...>::value ||
   //                     is_constructible_with_allocator_suffix<T, InnerAlloc, Args...>::value ));
   dispatch_allocator_prefix_suffix
      ( is_constructible_with_allocator_prefix<T, InnerAlloc, Args...>()
      , outermost_alloc, inner_alloc, p, ::ndnboost::forward<Args>(args)...);
}

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_uses_allocator
   ( ndnboost::false_type uses_allocator, OutermostAlloc & outermost_alloc
   , InnerAlloc & inner_alloc
   ,T* p, NDNBOOST_FWD_REF(Args)...args)
{
   (void)uses_allocator; (void)inner_alloc;
   allocator_traits<OutermostAlloc>::construct
      (outermost_alloc, p, ::ndnboost::forward<Args>(args)...);
}

#else    //#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

#define NDNBOOST_PP_LOCAL_MACRO(n)                                                              \
template < typename OutermostAlloc                                                           \
         , typename InnerAlloc                                                               \
         , typename T                                                                        \
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)                                           \
         >                                                                                   \
inline void dispatch_allocator_prefix_suffix(                                                \
                                       ndnboost::true_type  use_alloc_prefix,                   \
                                       OutermostAlloc& outermost_alloc,                      \
                                       InnerAlloc&    inner_alloc,                           \
                                       T* p                                                  \
                              NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))   \
{                                                                                            \
   (void)use_alloc_prefix,                                                                   \
   allocator_traits<OutermostAlloc>::construct                                               \
      (outermost_alloc, p, allocator_arg, inner_alloc                                        \
       NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _));                      \
}                                                                                            \
                                                                                             \
template < typename OutermostAlloc                                                           \
         , typename InnerAlloc                                                               \
         , typename T                                                                        \
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)                                           \
         >                                                                                   \
inline void dispatch_allocator_prefix_suffix(                                                \
                           ndnboost::false_type use_alloc_prefix,                               \
                           OutermostAlloc& outermost_alloc,                                  \
                           InnerAlloc&    inner_alloc,                                       \
                           T* p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _)) \
{                                                                                            \
   (void)use_alloc_prefix;                                                                   \
   allocator_traits<OutermostAlloc>::construct                                               \
      (outermost_alloc, p                                                                    \
       NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _)                        \
      , inner_alloc);                                                                        \
}                                                                                            \
                                                                                             \
template < typename OutermostAlloc                                                           \
         , typename InnerAlloc                                                               \
         , typename T                                                                        \
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)                                           \
         >                                                                                   \
inline void dispatch_uses_allocator(ndnboost::true_type uses_allocator,                         \
                        OutermostAlloc& outermost_alloc,                                     \
                        InnerAlloc&    inner_alloc,                                          \
                        T* p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))    \
{                                                                                            \
   (void)uses_allocator;                                                                     \
   dispatch_allocator_prefix_suffix                                                          \
      (is_constructible_with_allocator_prefix                                                \
         < T, InnerAlloc NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, P)>()                              \
         , outermost_alloc, inner_alloc, p                                                   \
         NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _));                    \
}                                                                                            \
                                                                                             \
template < typename OutermostAlloc                                                           \
         , typename InnerAlloc                                                               \
         , typename T                                                                        \
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)                                           \
         >                                                                                   \
inline void dispatch_uses_allocator(ndnboost::false_type uses_allocator                         \
                        ,OutermostAlloc &    outermost_alloc                                 \
                        ,InnerAlloc &    inner_alloc                                         \
                        ,T* p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))   \
{                                                                                            \
   (void)uses_allocator; (void)inner_alloc;                                                  \
   allocator_traits<OutermostAlloc>::construct                                               \
      (outermost_alloc, p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _));   \
}                                                                                            \
//!
#define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
#include NDNBOOST_PP_LOCAL_ITERATE()

#endif   //#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

template <typename OuterAlloc, class ...InnerAllocs>
class scoped_allocator_adaptor_base
   : public OuterAlloc
{
   typedef allocator_traits<OuterAlloc> outer_traits_type;
   NDNBOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)

   public:
   template <class OuterA2>
   struct rebind_base
   {
      typedef scoped_allocator_adaptor_base<OuterA2, InnerAllocs...> other;
   };

   typedef OuterAlloc outer_allocator_type;
   typedef scoped_allocator_adaptor<InnerAllocs...>   inner_allocator_type;
   typedef allocator_traits<inner_allocator_type>     inner_traits_type;
   typedef scoped_allocator_adaptor
      <OuterAlloc, InnerAllocs...>                    scoped_allocator_type;
   typedef ndnboost::integral_constant<
      bool,
      outer_traits_type::propagate_on_container_copy_assignment::value ||
      inner_allocator_type::propagate_on_container_copy_assignment::value
      > propagate_on_container_copy_assignment;
   typedef ndnboost::integral_constant<
      bool,
      outer_traits_type::propagate_on_container_move_assignment::value ||
      inner_allocator_type::propagate_on_container_move_assignment::value
      > propagate_on_container_move_assignment;
   typedef ndnboost::integral_constant<
      bool,
      outer_traits_type::propagate_on_container_swap::value ||
      inner_allocator_type::propagate_on_container_swap::value
      > propagate_on_container_swap;

   scoped_allocator_adaptor_base()
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base(NDNBOOST_FWD_REF(OuterA2) outerAlloc, const InnerAllocs &...args)
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))
      , m_inner(args...)
      {}

   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   scoped_allocator_adaptor_base(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)
      : outer_allocator_type(::ndnboost::move(other.outer_allocator()))
      , m_inner(::ndnboost::move(other.inner_allocator()))
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (const scoped_allocator_adaptor_base<OuterA2, InnerAllocs...>& other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (NDNBOOST_RV_REF_BEG scoped_allocator_adaptor_base
         <OuterA2, InnerAllocs...> NDNBOOST_RV_REF_END other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   public:
   struct internal_type_t{};

   template <class OuterA2>
   scoped_allocator_adaptor_base
      ( internal_type_t
      , NDNBOOST_FWD_REF(OuterA2) outerAlloc
      , const inner_allocator_type &inner)
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))
      , m_inner(inner)
   {}

   public:

   scoped_allocator_adaptor_base &operator=
      (NDNBOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(other.outer_allocator());
      m_inner = other.inner_allocator();
      return *this;
   }

   scoped_allocator_adaptor_base &operator=(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(ndnboost::move(other.outer_allocator()));
      m_inner = ::ndnboost::move(other.inner_allocator());
      return *this;
   }

   void swap(scoped_allocator_adaptor_base &r)
   {
      ndnboost::container::swap_dispatch(this->outer_allocator(), r.outer_allocator());
      ndnboost::container::swap_dispatch(this->m_inner, r.inner_allocator());
   }

   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)
   {  l.swap(r);  }

   inner_allocator_type&       inner_allocator() NDNBOOST_CONTAINER_NOEXCEPT
      { return m_inner; }

   inner_allocator_type const& inner_allocator() const NDNBOOST_CONTAINER_NOEXCEPT
      { return m_inner; }

   outer_allocator_type      & outer_allocator() NDNBOOST_CONTAINER_NOEXCEPT
      { return static_cast<outer_allocator_type&>(*this); }

   const outer_allocator_type &outer_allocator() const NDNBOOST_CONTAINER_NOEXCEPT
      { return static_cast<const outer_allocator_type&>(*this); }

   scoped_allocator_type select_on_container_copy_construction() const
   {
      return scoped_allocator_type
         (internal_type_t()
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())
         ,inner_traits_type::select_on_container_copy_construction(this->inner_allocator())
         );
   }

   private:
   inner_allocator_type m_inner;
};

#else //#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

//Let's add a dummy first template parameter to allow creating
//specializations up to maximum InnerAlloc count
template <
   typename OuterAlloc
   , bool Dummy
   NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, class Q)
   >
class scoped_allocator_adaptor_base;

//Specializations for the adaptor with InnerAlloc allocators

#define NDNBOOST_PP_LOCAL_MACRO(n)                                                                 \
template <typename OuterAlloc                                                                   \
NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class Q)                                                       \
>                                                                                               \
class scoped_allocator_adaptor_base<OuterAlloc, true                                            \
   NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, Q)                                                          \
   NDNBOOST_PP_ENUM_TRAILING( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)          \
                         , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                    \
   >                                                                                            \
   : public OuterAlloc                                                                          \
{                                                                                               \
   typedef allocator_traits<OuterAlloc> outer_traits_type;                                      \
   NDNBOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)                                    \
                                                                                                \
   public:                                                                                      \
   template <class OuterA2>                                                                     \
   struct rebind_base                                                                           \
   {                                                                                            \
      typedef scoped_allocator_adaptor_base<OuterA2, true NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, Q)   \
         NDNBOOST_PP_ENUM_TRAILING                                                                 \
            ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)                       \
            , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                                 \
         > other;                                                                               \
   };                                                                                           \
                                                                                                \
   typedef OuterAlloc outer_allocator_type;                                                     \
   typedef scoped_allocator_adaptor<NDNBOOST_PP_ENUM_PARAMS(n, Q)                                  \
      NDNBOOST_PP_ENUM_TRAILING                                                                    \
         ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)                          \
         , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                                    \
      > inner_allocator_type;                                                                   \
   typedef scoped_allocator_adaptor<OuterAlloc, NDNBOOST_PP_ENUM_PARAMS(n, Q)                      \
      NDNBOOST_PP_ENUM_TRAILING                                                                    \
         ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)                          \
         , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                                    \
      > scoped_allocator_type;                                                                  \
   typedef allocator_traits<inner_allocator_type>   inner_traits_type;                          \
   typedef ndnboost::integral_constant<                                                            \
      bool,                                                                                     \
      outer_traits_type::propagate_on_container_copy_assignment::value ||                       \
      inner_allocator_type::propagate_on_container_copy_assignment::value                       \
      > propagate_on_container_copy_assignment;                                                 \
   typedef ndnboost::integral_constant<                                                            \
      bool,                                                                                     \
      outer_traits_type::propagate_on_container_move_assignment::value ||                       \
      inner_allocator_type::propagate_on_container_move_assignment::value                       \
      > propagate_on_container_move_assignment;                                                 \
   typedef ndnboost::integral_constant<                                                            \
      bool,                                                                                     \
      outer_traits_type::propagate_on_container_swap::value ||                                  \
      inner_allocator_type::propagate_on_container_swap::value                                  \
      > propagate_on_container_swap;                                                            \
                                                                                                \
   scoped_allocator_adaptor_base()                                                              \
      {}                                                                                        \
                                                                                                \
   template <class OuterA2>                                                                     \
   scoped_allocator_adaptor_base(NDNBOOST_FWD_REF(OuterA2) outerAlloc                              \
      NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_CONST_REF_PARAM_LIST_Q, _))                  \
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))                             \
      , m_inner(NDNBOOST_PP_ENUM_PARAMS(n, q))                                                     \
      {}                                                                                        \
                                                                                                \
   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)                    \
      : outer_allocator_type(other.outer_allocator())                                           \
      , m_inner(other.inner_allocator())                                                        \
      {}                                                                                        \
                                                                                                \
   scoped_allocator_adaptor_base(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)             \
      : outer_allocator_type(::ndnboost::move(other.outer_allocator()))                            \
      , m_inner(::ndnboost::move(other.inner_allocator()))                                         \
      {}                                                                                        \
                                                                                                \
   template <class OuterA2>                                                                     \
   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base<OuterA2, true              \
          NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, Q)                                                   \
          NDNBOOST_PP_ENUM_TRAILING                                                                \
            ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)                       \
            , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                                 \
         >& other)                                                                              \
      : outer_allocator_type(other.outer_allocator())                                           \
      , m_inner(other.inner_allocator())                                                        \
      {}                                                                                        \
                                                                                                \
   template <class OuterA2>                                                                     \
   scoped_allocator_adaptor_base                                                                \
      (NDNBOOST_RV_REF_BEG scoped_allocator_adaptor_base<OuterA2, true                             \
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, Q)                                                    \
         NDNBOOST_PP_ENUM_TRAILING                                                                 \
            ( NDNBOOST_PP_SUB(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, n)                       \
            , NDNBOOST_CONTAINER_PP_IDENTITY, nat)                                                 \
         > NDNBOOST_RV_REF_END other)                                                              \
      : outer_allocator_type(other.outer_allocator())                                           \
      , m_inner(other.inner_allocator())                                                        \
      {}                                                                                        \
                                                                                                \
   public:                                                                                      \
   struct internal_type_t{};                                                                    \
                                                                                                \
   template <class OuterA2>                                                                     \
   scoped_allocator_adaptor_base                                                                \
      ( internal_type_t                                                                         \
      , NDNBOOST_FWD_REF(OuterA2) outerAlloc                                                       \
      , const inner_allocator_type &inner)                                                      \
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))                             \
      , m_inner(inner)                                                                          \
   {}                                                                                           \
                                                                                                \
   public:                                                                                      \
   scoped_allocator_adaptor_base &operator=                                                     \
      (NDNBOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)                              \
   {                                                                                            \
      outer_allocator_type::operator=(other.outer_allocator());                                 \
      m_inner = other.inner_allocator();                                                        \
      return *this;                                                                             \
   }                                                                                            \
                                                                                                \
   scoped_allocator_adaptor_base &operator=(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)  \
   {                                                                                            \
      outer_allocator_type::operator=(ndnboost::move(other.outer_allocator()));                    \
      m_inner = ::ndnboost::move(other.inner_allocator());                                         \
      return *this;                                                                             \
   }                                                                                            \
                                                                                                \
   void swap(scoped_allocator_adaptor_base &r)                                                  \
   {                                                                                            \
      ndnboost::container::swap_dispatch(this->outer_allocator(), r.outer_allocator());            \
      ndnboost::container::swap_dispatch(this->m_inner, r.inner_allocator());                      \
   }                                                                                            \
                                                                                                \
   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)         \
   {  l.swap(r);  }                                                                             \
                                                                                                \
   inner_allocator_type&       inner_allocator()                                                \
      { return m_inner; }                                                                       \
                                                                                                \
   inner_allocator_type const& inner_allocator() const                                          \
      { return m_inner; }                                                                       \
                                                                                                \
   outer_allocator_type      & outer_allocator()                                                \
      { return static_cast<outer_allocator_type&>(*this); }                                     \
                                                                                                \
   const outer_allocator_type &outer_allocator() const                                          \
      { return static_cast<const outer_allocator_type&>(*this); }                               \
                                                                                                \
   scoped_allocator_type select_on_container_copy_construction() const                          \
   {                                                                                            \
      return scoped_allocator_type                                                              \
         (internal_type_t()                                                                     \
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())     \
         ,inner_traits_type::select_on_container_copy_construction(this->inner_allocator())     \
         );                                                                                     \
   }                                                                                            \
   private:                                                                                     \
   inner_allocator_type m_inner;                                                                \
};                                                                                              \
//!
#define NDNBOOST_PP_LOCAL_LIMITS (1, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
#include NDNBOOST_PP_LOCAL_ITERATE()

#endif   //#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

//Specialization for adaptor without any InnerAlloc
template <typename OuterAlloc>
class scoped_allocator_adaptor_base
   < OuterAlloc
   #if defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
      , true
      NDNBOOST_PP_ENUM_TRAILING(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, NDNBOOST_CONTAINER_PP_IDENTITY, nat)
   #endif
   >
   : public OuterAlloc
{
   NDNBOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)
   public:

   template <class U>
   struct rebind_base
   {
      typedef scoped_allocator_adaptor_base
         <typename allocator_traits<OuterAlloc>::template portable_rebind_alloc<U>::type
         #if defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         , true
         NDNBOOST_PP_ENUM_TRAILING(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, NDNBOOST_CONTAINER_PP_IDENTITY, container_detail::nat)
         #endif
         > other;
   };

   typedef OuterAlloc                           outer_allocator_type;
   typedef allocator_traits<OuterAlloc>         outer_traits_type;
   typedef scoped_allocator_adaptor<OuterAlloc> inner_allocator_type;
   typedef inner_allocator_type                 scoped_allocator_type;
   typedef allocator_traits<inner_allocator_type>   inner_traits_type;
   typedef typename outer_traits_type::
      propagate_on_container_copy_assignment    propagate_on_container_copy_assignment;
   typedef typename outer_traits_type::
      propagate_on_container_move_assignment    propagate_on_container_move_assignment;
   typedef typename outer_traits_type::
      propagate_on_container_swap               propagate_on_container_swap;

   scoped_allocator_adaptor_base()
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base(NDNBOOST_FWD_REF(OuterA2) outerAlloc)
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))
      {}

   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)
      : outer_allocator_type(other.outer_allocator())
      {}

   scoped_allocator_adaptor_base(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)
      : outer_allocator_type(::ndnboost::move(other.outer_allocator()))
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (const scoped_allocator_adaptor_base<
         OuterA2
         #if defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         , true
         NDNBOOST_PP_ENUM_TRAILING(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, NDNBOOST_CONTAINER_PP_IDENTITY, container_detail::nat)
         #endif
         >& other)
      : outer_allocator_type(other.outer_allocator())
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (NDNBOOST_RV_REF_BEG scoped_allocator_adaptor_base<
         OuterA2
         #if defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         , true
         NDNBOOST_PP_ENUM_TRAILING(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, NDNBOOST_CONTAINER_PP_IDENTITY, container_detail::nat)
         #endif
         > NDNBOOST_RV_REF_END other)
      : outer_allocator_type(other.outer_allocator())
      {}

   public:
   struct internal_type_t{};

   template <class OuterA2>
   scoped_allocator_adaptor_base(internal_type_t, NDNBOOST_FWD_REF(OuterA2) outerAlloc, const inner_allocator_type &)
      : outer_allocator_type(::ndnboost::forward<OuterA2>(outerAlloc))
      {}

   public:
   scoped_allocator_adaptor_base &operator=(NDNBOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(other.outer_allocator());
      return *this;
   }

   scoped_allocator_adaptor_base &operator=(NDNBOOST_RV_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(ndnboost::move(other.outer_allocator()));
      return *this;
   }

   void swap(scoped_allocator_adaptor_base &r)
   {
      ndnboost::container::swap_dispatch(this->outer_allocator(), r.outer_allocator());
   }

   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)
   {  l.swap(r);  }

   inner_allocator_type&       inner_allocator()
      { return static_cast<inner_allocator_type&>(*this); }

   inner_allocator_type const& inner_allocator() const
      { return static_cast<const inner_allocator_type&>(*this); }

   outer_allocator_type      & outer_allocator()
      { return static_cast<outer_allocator_type&>(*this); }

   const outer_allocator_type &outer_allocator() const
      { return static_cast<const outer_allocator_type&>(*this); }

   scoped_allocator_type select_on_container_copy_construction() const
   {
      return scoped_allocator_type
         (internal_type_t()
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())
         //Don't use inner_traits_type::select_on_container_copy_construction(this->inner_allocator())
         //as inner_allocator() is equal to *this and that would trigger an infinite loop
         , this->inner_allocator()
         );
   }
};

}  //namespace container_detail {

#endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

//Scoped allocator
#if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   #if !defined(NDNBOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

   //! This class is a C++03-compatible implementation of std::scoped_allocator_adaptor.
   //! The class template scoped_allocator_adaptor is an allocator template that specifies
   //! the memory resource (the outer allocator) to be used by a container (as any other
   //! allocator does) and also specifies an inner allocator resource to be passed to
   //! the constructor of every element within the container.
   //!
   //! This adaptor is
   //! instantiated with one outer and zero or more inner allocator types. If
   //! instantiated with only one allocator type, the inner allocator becomes the
   //! scoped_allocator_adaptor itself, thus using the same allocator resource for the
   //! container and every element within the container and, if the elements themselves
   //! are containers, each of their elements recursively. If instantiated with more than
   //! one allocator, the first allocator is the outer allocator for use by the container,
   //! the second allocator is passed to the constructors of the container's elements,
   //! and, if the elements themselves are containers, the third allocator is passed to
   //! the elements' elements, and so on. If containers are nested to a depth greater
   //! than the number of allocators, the last allocator is used repeatedly, as in the
   //! single-allocator case, for any remaining recursions.
   //!
   //! [<b>Note</b>: The
   //! scoped_allocator_adaptor is derived from the outer allocator type so it can be
   //! substituted for the outer allocator type in most expressions. -end note]
   //!
   //! In the construct member functions, <code>OUTERMOST(x)</code> is x if x does not have
   //! an <code>outer_allocator()</code> member function and
   //! <code>OUTERMOST(x.outer_allocator())</code> otherwise; <code>OUTERMOST_ALLOC_TRAITS(x)</code> is
   //! <code>allocator_traits<decltype(OUTERMOST(x))></code>.
   //!
   //! [<b>Note</b>: <code>OUTERMOST(x)</code> and
   //! <code>OUTERMOST_ALLOC_TRAITS(x)</code> are recursive operations. It is incumbent upon
   //! the definition of <code>outer_allocator()</code> to ensure that the recursion terminates.
   //! It will terminate for all instantiations of scoped_allocator_adaptor. -end note]
   template <typename OuterAlloc, typename ...InnerAllocs>
   class scoped_allocator_adaptor

   #else // #if !defined(NDNBOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

   template <typename OuterAlloc, typename ...InnerAllocs>
   class scoped_allocator_adaptor<OuterAlloc, InnerAllocs...>

   #endif   // #if !defined(NDNBOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

#else // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

template <typename OuterAlloc
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, class Q)
         >
class scoped_allocator_adaptor
#endif
   : public container_detail::scoped_allocator_adaptor_base
         <OuterAlloc
         #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
         , InnerAllocs...
         #else
         , true NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
         #endif
         >
{
   NDNBOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor)

   public:
   #ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   typedef container_detail::scoped_allocator_adaptor_base
      <OuterAlloc
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      , InnerAllocs...
      #else
      , true NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
      >                       base_type;
   typedef typename base_type::internal_type_t              internal_type_t;
   #endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   typedef OuterAlloc                                       outer_allocator_type;
   //! Type: For exposition only
   //!
   typedef allocator_traits<OuterAlloc>                     outer_traits_type;
   //! Type: <code>scoped_allocator_adaptor<OuterAlloc></code> if <code>sizeof...(InnerAllocs)</code> is zero; otherwise,
   //! <code>scoped_allocator_adaptor<InnerAllocs...></code>.
   typedef typename base_type::inner_allocator_type         inner_allocator_type;
   typedef allocator_traits<inner_allocator_type>           inner_traits_type;
   typedef typename outer_traits_type::value_type           value_type;
   typedef typename outer_traits_type::size_type            size_type;
   typedef typename outer_traits_type::difference_type      difference_type;
   typedef typename outer_traits_type::pointer              pointer;
   typedef typename outer_traits_type::const_pointer        const_pointer;
   typedef typename outer_traits_type::void_pointer         void_pointer;
   typedef typename outer_traits_type::const_void_pointer   const_void_pointer;
   //! Type: <code>true_type</code> if <code>allocator_traits<Allocator>::propagate_on_container_copy_assignment::value</code> is
   //! true for any <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>; otherwise, false_type.
   typedef typename base_type::
      propagate_on_container_copy_assignment                propagate_on_container_copy_assignment;
   //! Type: <code>true_type</code> if <code>allocator_traits<Allocator>::propagate_on_container_move_assignment::value</code> is
   //! true for any <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>; otherwise, false_type.
   typedef typename base_type::
      propagate_on_container_move_assignment                propagate_on_container_move_assignment;
   //! Type: <code>true_type</code> if <code>allocator_traits<Allocator>::propagate_on_container_swap::value</code> is true for any
   //! <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>; otherwise, false_type.
   typedef typename base_type::
      propagate_on_container_swap                           propagate_on_container_swap;

   //! Type: Rebinds scoped allocator to
   //!    <code>typedef scoped_allocator_adaptor
   //!      < typename outer_traits_type::template portable_rebind_alloc<U>::type
   //!      , InnerAllocs... ></code>
   template <class U>
   struct rebind
   {
      typedef scoped_allocator_adaptor
         < typename outer_traits_type::template portable_rebind_alloc<U>::type
         #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
         , InnerAllocs...
         #else
         NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
         #endif
         > other;
   };

   //! <b>Effects</b>: value-initializes the OuterAlloc base class
   //! and the inner allocator object.
   scoped_allocator_adaptor()
      {}

   ~scoped_allocator_adaptor()
      {}

   //! <b>Effects</b>: initializes each allocator within the adaptor with
   //! the corresponding allocator from other.
   scoped_allocator_adaptor(const scoped_allocator_adaptor& other)
      : base_type(other.base())
      {}

   //! <b>Effects</b>: move constructs each allocator within the adaptor with
   //! the corresponding allocator from other.
   scoped_allocator_adaptor(NDNBOOST_RV_REF(scoped_allocator_adaptor) other)
      : base_type(::ndnboost::move(other.base()))
      {}

   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes the OuterAlloc base class with ndnboost::forward<OuterA2>(outerAlloc) and inner
   //! with innerAllocs...(hence recursively initializing each allocator within the adaptor with the
   //! corresponding allocator from the argument list).
   template <class OuterA2>
   scoped_allocator_adaptor(NDNBOOST_FWD_REF(OuterA2) outerAlloc, const InnerAllocs & ...innerAllocs)
      : base_type(::ndnboost::forward<OuterA2>(outerAlloc), innerAllocs...)
      {}
   #else // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   #define NDNBOOST_PP_LOCAL_MACRO(n)                                                              \
   template <class OuterA2>                                                                     \
   scoped_allocator_adaptor(NDNBOOST_FWD_REF(OuterA2) outerAlloc                                   \
                     NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_CONST_REF_PARAM_LIST_Q, _))   \
      : base_type(::ndnboost::forward<OuterA2>(outerAlloc)                                         \
                  NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, q)                                           \
                  )                                                                             \
      {}                                                                                        \
   //!
   #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
   #include NDNBOOST_PP_LOCAL_ITERATE()

   #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes each allocator within the adaptor with the corresponding allocator from other.
   template <class OuterA2>
   scoped_allocator_adaptor(const scoped_allocator_adaptor<OuterA2
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      , InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
      > &other)
      : base_type(other.base())
      {}

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes each allocator within the adaptor with the corresponding allocator
   //! rvalue from other.
   template <class OuterA2>
   scoped_allocator_adaptor(NDNBOOST_RV_REF_BEG scoped_allocator_adaptor<OuterA2
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      , InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
      > NDNBOOST_RV_REF_END other)
      : base_type(::ndnboost::move(other.base()))
      {}

   scoped_allocator_adaptor &operator=(NDNBOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor) other)
   {  return static_cast<scoped_allocator_adaptor&>(base_type::operator=(static_cast<const base_type &>(other))); }

   scoped_allocator_adaptor &operator=(NDNBOOST_RV_REF(scoped_allocator_adaptor) other)
   {  return static_cast<scoped_allocator_adaptor&>(base_type::operator=(ndnboost::move(static_cast<base_type&>(other)))); }

   #ifdef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   //! <b>Effects</b>: swaps *this with r.
   //!
   void swap(scoped_allocator_adaptor &r);

   //! <b>Effects</b>: swaps *this with r.
   //!
   friend void swap(scoped_allocator_adaptor &l, scoped_allocator_adaptor &r);

   //! <b>Returns</b>:
   //!   <code>static_cast<OuterAlloc&>(*this)</code>.
   outer_allocator_type      & outer_allocator() NDNBOOST_CONTAINER_NOEXCEPT;

   //! <b>Returns</b>:
   //!   <code>static_cast<const OuterAlloc&>(*this)</code>.
   const outer_allocator_type &outer_allocator() const NDNBOOST_CONTAINER_NOEXCEPT;

   //! <b>Returns</b>:
   //!   *this if <code>sizeof...(InnerAllocs)</code> is zero; otherwise, inner.
   inner_allocator_type&       inner_allocator() NDNBOOST_CONTAINER_NOEXCEPT;

   //! <b>Returns</b>:
   //!   *this if <code>sizeof...(InnerAllocs)</code> is zero; otherwise, inner.
   inner_allocator_type const& inner_allocator() const NDNBOOST_CONTAINER_NOEXCEPT;

   #endif   //NDNBOOST_CONTAINER_DOXYGEN_INVOKED

   //! <b>Returns</b>:
   //!   <code>allocator_traits<OuterAlloc>::max_size(outer_allocator())</code>.
   size_type max_size() const NDNBOOST_CONTAINER_NOEXCEPT
   {
      return outer_traits_type::max_size(this->outer_allocator());
   }

   //! <b>Effects</b>:
   //!   calls <code>OUTERMOST_ALLOC_TRAITS(*this)::destroy(OUTERMOST(*this), p)</code>.
   template <class T>
   void destroy(T* p) NDNBOOST_CONTAINER_NOEXCEPT
   {
      allocator_traits<typename outermost_allocator<OuterAlloc>::type>
         ::destroy(get_outermost_allocator(this->outer_allocator()), p);
   }

   //! <b>Returns</b>:
   //! <code>allocator_traits<OuterAlloc>::allocate(outer_allocator(), n)</code>.
   pointer allocate(size_type n)
   {
      return outer_traits_type::allocate(this->outer_allocator(), n);
   }

   //! <b>Returns</b>:
   //! <code>allocator_traits<OuterAlloc>::allocate(outer_allocator(), n, hint)</code>.
   pointer allocate(size_type n, const_void_pointer hint)
   {
      return outer_traits_type::allocate(this->outer_allocator(), n, hint);
   }

   //! <b>Effects</b>:
   //! <code>allocator_traits<OuterAlloc>::deallocate(outer_allocator(), p, n)</code>.
   void deallocate(pointer p, size_type n)
   {
      outer_traits_type::deallocate(this->outer_allocator(), p, n);
   }

   #ifdef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   //! <b>Returns</b>: Allocator new scoped_allocator_adaptor object where each allocator
   //! A in the adaptor is initialized from the result of calling
   //! <code>allocator_traits<Allocator>::select_on_container_copy_construction()</code> on
   //! the corresponding allocator in *this.
   scoped_allocator_adaptor select_on_container_copy_construction() const;
   #endif   //NDNBOOST_CONTAINER_DOXYGEN_INVOKED

   #ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   base_type &base()             { return *this; }

   const base_type &base() const { return *this; }
   #endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Effects</b>:
   //! 1) If <code>uses_allocator<T, inner_allocator_type>::value</code> is false calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct
   //!       (OUTERMOST(*this), p, std::forward<Args>(args)...)</code>.
   //!
   //! 2) Otherwise, if <code>uses_allocator<T, inner_allocator_type>::value</code> is true and
   //!    <code>is_constructible<T, allocator_arg_t, inner_allocator_type, Args...>::value</code> is true, calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST(*this), p, allocator_arg,
   //!    inner_allocator(), std::forward<Args>(args)...)</code>.
   //!
   //! [<b>Note</b>: In compilers without advanced decltype SFINAE support, <code>is_constructible</code> can't
   //! be implemented so that condition will be replaced by
   //! constructible_with_allocator_prefix<T>::value. -end note]
   //!
   //! 3) Otherwise, if uses_allocator<T, inner_allocator_type>::value is true and
   //!    <code>is_constructible<T, Args..., inner_allocator_type>::value</code> is true, calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST(*this), p,
   //!    std::forward<Args>(args)..., inner_allocator())</code>.
   //!
   //! [<b>Note</b>: In compilers without advanced decltype SFINAE support, <code>is_constructible</code> can't be
   //! implemented so that condition will be replaced by
   //! <code>constructible_with_allocator_suffix<T>::value</code>. -end note]
   //!
   //! 4) Otherwise, the program is ill-formed.
   //!
   //! [<b>Note</b>: An error will result if <code>uses_allocator</code> evaluates
   //! to true but the specific constructor does not take an allocator. This definition prevents a silent
   //! failure to pass an inner allocator to a contained element. -end note]
   template < typename T, class ...Args>
   #if defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
   void
   #else
   typename container_detail::enable_if_c<!container_detail::is_pair<T>::value, void>::type
   #endif
   construct(T* p, NDNBOOST_FWD_REF(Args)...args)
   {
      container_detail::dispatch_uses_allocator
         ( uses_allocator<T, inner_allocator_type>()
         , get_outermost_allocator(this->outer_allocator())
         , this->inner_allocator()
         , p, ::ndnboost::forward<Args>(args)...);
   }

   #else // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   //Disable this overload if the first argument is pair as some compilers have
   //overload selection problems when the first parameter is a pair.
   #define NDNBOOST_PP_LOCAL_MACRO(n)                                                              \
   template < typename T                                                                        \
            NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P)                                           \
            >                                                                                   \
   typename container_detail::enable_if_c<!container_detail::is_pair<T>::value, void>::type     \
      construct(T* p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))               \
   {                                                                                            \
      container_detail::dispatch_uses_allocator                                                 \
         ( uses_allocator<T, inner_allocator_type>()                                            \
         , get_outermost_allocator(this->outer_allocator())                                     \
         , this->inner_allocator()                                                              \
         , p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _));                   \
   }                                                                                            \
   //!
   #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
   #include NDNBOOST_PP_LOCAL_ITERATE()

   #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   template <class T1, class T2>
   void construct(std::pair<T1,T2>* p)
   {  this->construct_pair(p);  }

   template <class T1, class T2>
   void construct(container_detail::pair<T1,T2>* p)
   {  this->construct_pair(p);  }

   template <class T1, class T2, class U, class V>
   void construct(std::pair<T1, T2>* p, NDNBOOST_FWD_REF(U) x, NDNBOOST_FWD_REF(V) y)
   {  this->construct_pair(p, ::ndnboost::forward<U>(x), ::ndnboost::forward<V>(y));   }

   template <class T1, class T2, class U, class V>
   void construct(container_detail::pair<T1, T2>* p, NDNBOOST_FWD_REF(U) x, NDNBOOST_FWD_REF(V) y)
   {  this->construct_pair(p, ::ndnboost::forward<U>(x), ::ndnboost::forward<V>(y));   }

   template <class T1, class T2, class U, class V>
   void construct(std::pair<T1, T2>* p, const std::pair<U, V>& x)
   {  this->construct_pair(p, x);   }

   template <class T1, class T2, class U, class V>
   void construct( container_detail::pair<T1, T2>* p
                 , const container_detail::pair<U, V>& x)
   {  this->construct_pair(p, x);   }

   template <class T1, class T2, class U, class V>
   void construct( std::pair<T1, T2>* p
                 , NDNBOOST_RV_REF_BEG std::pair<U, V> NDNBOOST_RV_REF_END x)
   {  this->construct_pair(p, x);   }

   template <class T1, class T2, class U, class V>
   void construct( container_detail::pair<T1, T2>* p
                 , NDNBOOST_RV_REF_BEG container_detail::pair<U, V> NDNBOOST_RV_REF_END x)
   {  this->construct_pair(p, x);   }

   #ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   private:
   template <class Pair>
   void construct_pair(Pair* p)
   {
      this->construct(container_detail::addressof(p->first));
      NDNBOOST_TRY{
         this->construct(container_detail::addressof(p->second));
      }
      NDNBOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
   }

   template <class Pair, class U, class V>
   void construct_pair(Pair* p, NDNBOOST_FWD_REF(U) x, NDNBOOST_FWD_REF(V) y)
   {
      this->construct(container_detail::addressof(p->first), ::ndnboost::forward<U>(x));
      NDNBOOST_TRY{
         this->construct(container_detail::addressof(p->second), ::ndnboost::forward<V>(y));
      }
      NDNBOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
   }

   template <class Pair, class Pair2>
   void construct_pair(Pair* p, const Pair2& pr)
   {
      this->construct(container_detail::addressof(p->first), pr.first);
      NDNBOOST_TRY{
         this->construct(container_detail::addressof(p->second), pr.second);
      }
      NDNBOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
   }

   template <class Pair, class Pair2>
   void construct_pair(Pair* p, NDNBOOST_RV_REF(Pair2) pr)
   {
      this->construct(container_detail::addressof(p->first), ::ndnboost::move(pr.first));
      NDNBOOST_TRY{
         this->construct(container_detail::addressof(p->second), ::ndnboost::move(pr.second));
      }
      NDNBOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         NDNBOOST_RETHROW
      }
      NDNBOOST_CATCH_END
   }

   //template <class T1, class T2, class... Args1, class... Args2>
   //void construct(pair<T1, T2>* p, piecewise_construct_t, tuple<Args1...> x, tuple<Args2...> y);

   public:
   //Internal function
   template <class OuterA2>
   scoped_allocator_adaptor(internal_type_t, NDNBOOST_FWD_REF(OuterA2) outer, const inner_allocator_type& inner)
      : base_type(internal_type_t(), ::ndnboost::forward<OuterA2>(outer), inner)
   {}

   #endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
};

template <typename OuterA1, typename OuterA2
   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
   , typename... InnerAllocs
   #else
   NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, class Q)
   #endif
   >
inline bool operator==(
   const scoped_allocator_adaptor<OuterA1
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      ,InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
      >& a,
   const scoped_allocator_adaptor<OuterA2
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      ,InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
   >& b)
{
   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
   const bool has_zero_inner = sizeof...(InnerAllocs) == 0u;
   #else
   const bool has_zero_inner =
      ndnboost::container::container_detail::is_same
         <Q0, container_detail::nat>::value;
   #endif

    return a.outer_allocator() == b.outer_allocator()
        && (has_zero_inner || a.inner_allocator() == b.inner_allocator());
}

template <typename OuterA1, typename OuterA2
   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
   , typename... InnerAllocs
   #else
   NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, class Q)
   #endif
   >
inline bool operator!=(
   const scoped_allocator_adaptor<OuterA1
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      ,InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
      >& a,
   const scoped_allocator_adaptor<OuterA2
      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      ,InnerAllocs...
      #else
      NDNBOOST_PP_ENUM_TRAILING_PARAMS(NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS, Q)
      #endif
   >& b)
{
    return ! (a == b);
}

}} // namespace ndnboost { namespace container {

#include <ndnboost/container/detail/config_end.hpp>

#endif //  NDNBOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP
