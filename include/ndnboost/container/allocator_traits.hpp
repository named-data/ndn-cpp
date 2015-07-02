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

#ifndef NDNBOOST_CONTAINER_ALLOCATOR_ALLOCATOR_TRAITS_HPP
#define NDNBOOST_CONTAINER_ALLOCATOR_ALLOCATOR_TRAITS_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

#include <ndnboost/container/detail/config_begin.hpp>
#include <ndnboost/container/detail/workaround.hpp>
#include <ndnboost/container/container_fwd.hpp>
#include <ndnboost/intrusive/pointer_traits.hpp>
#include <ndnboost/intrusive/detail/memory_util.hpp>
#include <ndnboost/container/detail/memory_util.hpp>
#include <ndnboost/type_traits/integral_constant.hpp>
#include <ndnboost/container/detail/mpl.hpp>
#include <ndnboost/move/utility.hpp>
#include <limits> //numeric_limits<>::max()
#include <new>    //placement new
#include <memory> //std::allocator

#if defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <ndnboost/container/detail/preprocessor.hpp>
#endif

namespace ndnboost {
namespace container {

#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

namespace container_detail {

//workaround needed for C++03 compilers with no construct()
//supporting rvalue references
template<class A>
struct is_std_allocator
{  static const bool value = false; };

template<class T>
struct is_std_allocator< std::allocator<T> >
{  static const bool value = true; };

}  //namespace container_detail {

#endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED

//! The class template allocator_traits supplies a uniform interface to all allocator types.
//! This class is a C++03-compatible implementation of std::allocator_traits
template <typename Alloc>
struct allocator_traits
{
   //allocator_type
   typedef Alloc allocator_type;
   //value_type
   typedef typename Alloc::value_type         value_type;

   #if defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      //! Alloc::pointer if such a type exists; otherwise, value_type*
      //!
      typedef unspecified pointer;
      //! Alloc::const_pointer if such a type exists ; otherwise, pointer_traits<pointer>::rebind<const
      //!
      typedef see_documentation const_pointer;
      //! Non-standard extension
      //! Alloc::reference if such a type exists; otherwise, value_type&
      typedef see_documentation reference;
      //! Non-standard extension
      //! Alloc::const_reference if such a type exists ; otherwise, const value_type&
      typedef see_documentation const_reference;
      //! Alloc::void_pointer if such a type exists ; otherwise, pointer_traits<pointer>::rebind<void>.
      //!
      typedef see_documentation void_pointer;
      //! Alloc::const_void_pointer if such a type exists ; otherwis e, pointer_traits<pointer>::rebind<const
      //!
      typedef see_documentation const_void_pointer;
      //! Alloc::difference_type if such a type exists ; otherwise, pointer_traits<pointer>::difference_type.
      //!
      typedef see_documentation difference_type;
      //! Alloc::size_type if such a type exists ; otherwise, make_unsigned<difference_type>::type
      //!
      typedef see_documentation size_type;
      //! Alloc::propagate_on_container_copy_assignment if such a type exists, otherwise an integral_constant
      //! type with internal constant static member <code>value</code> == false.
      typedef see_documentation propagate_on_container_copy_assignment;
      //! Alloc::propagate_on_container_move_assignment if such a type exists, otherwise an integral_constant
      //! type with internal constant static member <code>value</code> == false.
      typedef see_documentation propagate_on_container_move_assignment;
      //! Alloc::propagate_on_container_swap if such a type exists, otherwise an integral_constant
      //! type with internal constant static member <code>value</code> == false.
      typedef see_documentation propagate_on_container_swap;
      //! Defines an allocator: Alloc::rebind<T>::other if such a type exists; otherwise, Alloc<T, Args>
      //! if Alloc is a class template instantiation of the form Alloc<U, Args>, where Args is zero or
      //! more type arguments ; otherwise, the instantiation of rebind_alloc is ill-formed.
      //!
      //! In C++03 compilers <code>rebind_alloc</code> is a struct derived from an allocator
      //! deduced by previously detailed rules.
      template <class T> using rebind_alloc = see_documentation;

      //! In C++03 compilers <code>rebind_traits</code> is a struct derived from
      //! <code>allocator_traits<OtherAlloc></code>, where <code>OtherAlloc</code> is
      //! the allocator deduced by rules explained in <code>rebind_alloc</code>.
      template <class T> using rebind_traits = allocator_traits<rebind_alloc<T> >;

      //! Non-standard extension: Portable allocator rebind for C++03 and C++11 compilers.
      //! <code>type</code> is an allocator related to Alloc deduced deduced by rules explained in <code>rebind_alloc</code>.
      template <class T>
      struct portable_rebind_alloc
      {  typedef see_documentation type;  };
   #else
      //pointer
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         pointer, value_type*)
            pointer;
      //const_pointer
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_EVAL_DEFAULT(ndnboost::container::container_detail::, Alloc,
         const_pointer, typename ndnboost::intrusive::pointer_traits<pointer>::template
            rebind_pointer<const value_type>)
               const_pointer;
      //reference
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         reference, typename container_detail::unvoid<value_type>::type&)
            reference;
      //const_reference
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         const_reference, const typename container_detail::unvoid<value_type>::type&)
               const_reference;
      //void_pointer
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_EVAL_DEFAULT(ndnboost::container::container_detail::, Alloc,
         void_pointer, typename ndnboost::intrusive::pointer_traits<pointer>::template
            rebind_pointer<void>)
               void_pointer;
      //const_void_pointer
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_EVAL_DEFAULT(ndnboost::container::container_detail::, Alloc,
         const_void_pointer, typename ndnboost::intrusive::pointer_traits<pointer>::template
            rebind_pointer<const void>)
               const_void_pointer;
      //difference_type
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         difference_type, std::ptrdiff_t)
            difference_type;
      //size_type
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         size_type, std::size_t)
            size_type;
      //propagate_on_container_copy_assignment
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         propagate_on_container_copy_assignment, ndnboost::false_type)
            propagate_on_container_copy_assignment;
      //propagate_on_container_move_assignment
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         propagate_on_container_move_assignment, ndnboost::false_type)
            propagate_on_container_move_assignment;
      //propagate_on_container_swap
      typedef NDNBOOST_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(ndnboost::container::container_detail::, Alloc,
         propagate_on_container_swap, ndnboost::false_type)
            propagate_on_container_swap;

      #if !defined(NDNBOOST_NO_CXX11_TEMPLATE_ALIASES)
         //C++11
         template <typename T> using rebind_alloc  = typename ndnboost::intrusive::detail::type_rebinder<Alloc, T>::type;
         template <typename T> using rebind_traits = allocator_traits< rebind_alloc<T> >;
      #else    // #if !defined(NDNBOOST_NO_CXX11_TEMPLATE_ALIASES)
         //Some workaround for C++03 or C++11 compilers with no template aliases
         template <typename T>
         struct rebind_alloc : ndnboost::intrusive::detail::type_rebinder<Alloc,T>::type
         {
            typedef typename ndnboost::intrusive::detail::type_rebinder<Alloc,T>::type Base;
            #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
            template <typename... Args>
            rebind_alloc(NDNBOOST_FWD_REF(Args)... args)
               : Base(ndnboost::forward<Args>(args)...)
            {}
            #else    // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
            #define NDNBOOST_PP_LOCAL_MACRO(n)                                                        \
            NDNBOOST_PP_EXPR_IF(n, template<) NDNBOOST_PP_ENUM_PARAMS(n, class P) NDNBOOST_PP_EXPR_IF(n, >) \
            rebind_alloc(NDNBOOST_PP_ENUM(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))                       \
               : Base(NDNBOOST_PP_ENUM(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _))                       \
            {}                                                                                     \
            //
            #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
            #include NDNBOOST_PP_LOCAL_ITERATE()
            #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         };

         template <typename T>
         struct rebind_traits
            : allocator_traits<typename ndnboost::intrusive::detail::type_rebinder<Alloc, T>::type>
         {};
      #endif   // #if !defined(NDNBOOST_NO_CXX11_TEMPLATE_ALIASES)
      template <class T>
      struct portable_rebind_alloc
      {  typedef typename ndnboost::intrusive::detail::type_rebinder<Alloc, T>::type type;  };
   #endif   //NDNBOOST_CONTAINER_DOXYGEN_INVOKED

   //! <b>Returns</b>: <code>a.allocate(n)</code>
   //!
   static pointer allocate(Alloc &a, size_type n)
   {  return a.allocate(n);  }

   //! <b>Returns</b>: <code>a.deallocate(p, n)</code>
   //!
   //! <b>Throws</b>: Nothing
   static void deallocate(Alloc &a, pointer p, size_type n)
   {  a.deallocate(p, n);  }

   //! <b>Effects</b>: calls <code>a.allocate(n, p)</code> if that call is well-formed;
   //! otherwise, invokes <code>a.allocate(n)</code>
   static pointer allocate(Alloc &a, size_type n, const_void_pointer p)
   {
      const bool value = ndnboost::container::container_detail::
         has_member_function_callable_with_allocate
            <Alloc, const size_type, const const_void_pointer>::value;
      ::ndnboost::integral_constant<bool, value> flag;
      return allocator_traits::priv_allocate(flag, a, n, p);
   }

   //! <b>Effects</b>: calls <code>a.destroy(p)</code> if that call is well-formed;
   //! otherwise, invokes <code>p->~T()</code>.
   template<class T>
   static void destroy(Alloc &a, T*p) NDNBOOST_CONTAINER_NOEXCEPT
   {
      typedef T* destroy_pointer;
      const bool value = ndnboost::container::container_detail::
         has_member_function_callable_with_destroy
            <Alloc, const destroy_pointer>::value;
      ::ndnboost::integral_constant<bool, value> flag;
      allocator_traits::priv_destroy(flag, a, p);
   }

   //! <b>Returns</b>: <code>a.max_size()</code> if that expression is well-formed; otherwise,
   //! <code>numeric_limits<size_type>::max()</code>.
   static size_type max_size(const Alloc &a) NDNBOOST_CONTAINER_NOEXCEPT
   {
      const bool value = ndnboost::container::container_detail::
         has_member_function_callable_with_max_size
            <const Alloc>::value;
      ::ndnboost::integral_constant<bool, value> flag;
      return allocator_traits::priv_max_size(flag, a);
   }

   //! <b>Returns</b>: <code>a.select_on_container_copy_construction()</code> if that expression is well-formed;
   //! otherwise, a.
   static
   #if !defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
   typename container_detail::if_c
      <  ndnboost::container::container_detail::
                  has_member_function_callable_with_select_on_container_copy_construction
                     <const Alloc>::value
      , Alloc
      , const Alloc &
      >::type
   #else
   Alloc
   #endif
   select_on_container_copy_construction(const Alloc &a)
   {
      const bool value = ndnboost::container::container_detail::
         has_member_function_callable_with_select_on_container_copy_construction
            <const Alloc>::value;
      ::ndnboost::integral_constant<bool, value> flag;
      return allocator_traits::priv_select_on_container_copy_construction(flag, a);
   }

   #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      //! <b>Effects</b>: calls <code>a.construct(p, std::forward<Args>(args)...)</code> if that call is well-formed;
      //! otherwise, invokes <code>::new (static_cast<void*>(p)) T(std::forward<Args>(args)...)</code>
      template <class T, class ...Args>
      static void construct(Alloc & a, T* p, NDNBOOST_FWD_REF(Args)... args)
      {
         ::ndnboost::integral_constant<bool, container_detail::is_std_allocator<Alloc>::value> flag;
         allocator_traits::priv_construct(flag, a, p, ::ndnboost::forward<Args>(args)...);
      }
   #endif
   #ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
   #if !defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)
      private:
      static pointer priv_allocate(ndnboost::true_type, Alloc &a, size_type n, const_void_pointer p)
      {  return a.allocate(n, p);  }

      static pointer priv_allocate(ndnboost::false_type, Alloc &a, size_type n, const_void_pointer)
      {  return allocator_traits::allocate(a, n);  }

      template<class T>
      static void priv_destroy(ndnboost::true_type, Alloc &a, T* p) NDNBOOST_CONTAINER_NOEXCEPT
      {  a.destroy(p);  }

      template<class T>
      static void priv_destroy(ndnboost::false_type, Alloc &, T* p) NDNBOOST_CONTAINER_NOEXCEPT
      {  p->~T(); (void)p;  }

      static size_type priv_max_size(ndnboost::true_type, const Alloc &a) NDNBOOST_CONTAINER_NOEXCEPT
      {  return a.max_size();  }

      static size_type priv_max_size(ndnboost::false_type, const Alloc &) NDNBOOST_CONTAINER_NOEXCEPT
      {  return (std::numeric_limits<size_type>::max)();  }

      static Alloc priv_select_on_container_copy_construction(ndnboost::true_type, const Alloc &a)
      {  return a.select_on_container_copy_construction();  }

      static const Alloc &priv_select_on_container_copy_construction(ndnboost::false_type, const Alloc &a) NDNBOOST_CONTAINER_NOEXCEPT
      {  return a;  }

      #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         template<class T, class ...Args>
         static void priv_construct(ndnboost::false_type, Alloc &a, T *p, NDNBOOST_FWD_REF(Args) ...args)
         {
            const bool value = ndnboost::container::container_detail::
                  has_member_function_callable_with_construct
                     < Alloc, T*, Args... >::value;
            ::ndnboost::integral_constant<bool, value> flag;
            priv_construct_dispatch2(flag, a, p, ::ndnboost::forward<Args>(args)...);
         }

         template<class T, class ...Args>
         static void priv_construct(ndnboost::true_type, Alloc &a, T *p, NDNBOOST_FWD_REF(Args) ...args)
         {
            priv_construct_dispatch2(ndnboost::false_type(), a, p, ::ndnboost::forward<Args>(args)...);
         }

         template<class T, class ...Args>
         static void priv_construct_dispatch2(ndnboost::true_type, Alloc &a, T *p, NDNBOOST_FWD_REF(Args) ...args)
         {  a.construct( p, ::ndnboost::forward<Args>(args)...);  }

         template<class T, class ...Args>
         static void priv_construct_dispatch2(ndnboost::false_type, Alloc &, T *p, NDNBOOST_FWD_REF(Args) ...args)
         {  ::new((void*)p) T(::ndnboost::forward<Args>(args)...); }
      #else // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
         public:
         #define NDNBOOST_PP_LOCAL_MACRO(n)                                                              \
         template<class T NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P) >                                 \
         static void construct(Alloc &a, T *p                                                         \
                              NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _))            \
         {                                                                                            \
            ::ndnboost::integral_constant<bool, container_detail::is_std_allocator<Alloc>::value> flag;  \
            allocator_traits::priv_construct(flag, a, p                                               \
               NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _));                       \
         }                                                                                            \
         //
         #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
         #include NDNBOOST_PP_LOCAL_ITERATE()

         private:
         #define NDNBOOST_PP_LOCAL_MACRO(n)                                                                    \
         template<class T  NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P) >                                      \
         static void priv_construct(ndnboost::false_type, Alloc &a, T *p                                       \
                        NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST,_))                         \
         {                                                                                                  \
            const bool value =                                                                              \
               ndnboost::container::container_detail::has_member_function_callable_with_construct              \
                     < Alloc, T* NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_FWD_TYPE, _) >::value;        \
            ::ndnboost::integral_constant<bool, value> flag;                                                   \
            priv_construct_dispatch2(flag, a, p                                                             \
               NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _) );                            \
         }                                                                                                  \
                                                                                                            \
         template<class T  NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P) >                                      \
         static void priv_construct(ndnboost::true_type, Alloc &a, T *p                                        \
                        NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST,_))                         \
         {                                                                                                  \
            priv_construct_dispatch2(ndnboost::false_type(), a, p                                              \
               NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _) );                            \
         }                                                                                                  \
                                                                                                            \
         template<class T  NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P) >                                      \
         static void priv_construct_dispatch2(ndnboost::true_type, Alloc &a, T *p                              \
                        NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST,_))                         \
         {  a.construct( p NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _) );  }             \
                                                                                                            \
         template<class T  NDNBOOST_PP_ENUM_TRAILING_PARAMS(n, class P) >                                      \
         static void priv_construct_dispatch2(ndnboost::false_type, Alloc &, T *p                              \
                        NDNBOOST_PP_ENUM_TRAILING(n, NDNBOOST_CONTAINER_PP_PARAM_LIST, _) )                       \
         {  ::new((void*)p) T(NDNBOOST_PP_ENUM(n, NDNBOOST_CONTAINER_PP_PARAM_FORWARD, _)); }                     \
         //
         #define NDNBOOST_PP_LOCAL_LIMITS (0, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
         #include NDNBOOST_PP_LOCAL_ITERATE()
      #endif   // #if !defined(NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)

      template<class T>
      static void priv_construct_dispatch2(ndnboost::false_type, Alloc &, T *p, ::ndnboost::container::default_init_t)
      {  ::new((void*)p) T; }
   #endif   //#if defined(NDNBOOST_CONTAINER_DOXYGEN_INVOKED)

   #endif   //#ifndef NDNBOOST_CONTAINER_DOXYGEN_INVOKED
};

}  //namespace container {
}  //namespace ndnboost {

#include <ndnboost/container/detail/config_end.hpp>

#endif // ! defined(NDNBOOST_CONTAINER_ALLOCATOR_ALLOCATOR_TRAITS_HPP)
