/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_INTRUSIVE_FWD_HPP
#define NDNBOOST_INTRUSIVE_FWD_HPP

//! \file
//! This header file forward declares most Intrusive classes.
//!
//! It forward declares the following containers and hooks:
//!   - ndnboost::intrusive::slist / ndnboost::intrusive::slist_base_hook / ndnboost::intrusive::slist_member_hook
//!   - ndnboost::intrusive::list / ndnboost::intrusive::list_base_hook / ndnboost::intrusive::list_member_hook
//!   - ndnboost::intrusive::bstree / ndnboost::intrusive::bs_set / ndnboost::intrusive::bs_multiset /
//!      ndnboost::intrusive::bs_set_base_hook / ndnboost::intrusive::bs_set_member_hook
//!   - ndnboost::intrusive::rbtree / ndnboost::intrusive::set / ndnboost::intrusive::multiset / 
//!      ndnboost::intrusive::set_base_hook / ndnboost::intrusive::set_member_hook
//!   - ndnboost::intrusive::avltree / ndnboost::intrusive::avl_set / ndnboost::intrusive::avl_multiset /
//!      ndnboost::intrusive::avl_set_base_hook / ndnboost::intrusive::avl_set_member_hook
//!   - ndnboost::intrusive::splaytree / ndnboost::intrusive::splay_set / ndnboost::intrusive::splay_multiset 
//!   - ndnboost::intrusive::sgtree / ndnboost::intrusive::sg_set / ndnboost::intrusive::sg_multiset 
//!   - ndnboost::intrusive::treap / ndnboost::intrusive::treap_set / ndnboost::intrusive::treap_multiset
//!   - ndnboost::intrusive::hashtable / ndnboost::intrusive::unordered_set / ndnboost::intrusive::unordered_multiset /
//!      ndnboost::intrusive::unordered_set_base_hook / ndnboost::intrusive::unordered_set_member_hook / 
//!   - ndnboost::intrusive::any_base_hook / ndnboost::intrusive::any_member_hook
//!
//! It forward declares the following container or hook options:
//!   - ndnboost::intrusive::constant_time_size / ndnboost::intrusive::size_type / ndnboost::intrusive::compare / ndnboost::intrusive::equal
//!   - ndnboost::intrusive::floating_point / ndnboost::intrusive::priority / ndnboost::intrusive::hash
//!   - ndnboost::intrusive::value_traits / ndnboost::intrusive::member_hook / ndnboost::intrusive::function_hook / ndnboost::intrusive::base_hook
//!   - ndnboost::intrusive::void_pointer / ndnboost::intrusive::tag / ndnboost::intrusive::link_mode
//!   - ndnboost::intrusive::optimize_size / ndnboost::intrusive::linear / ndnboost::intrusive::cache_last
//!   - ndnboost::intrusive::bucket_traits / ndnboost::intrusive::store_hash / ndnboost::intrusive::optimize_multikey
//!   - ndnboost::intrusive::power_2_buckets / ndnboost::intrusive::cache_begin / ndnboost::intrusive::compare_hash / ndnboost::intrusive::incremental
//!
//! It forward declares the following value traits utilities:
//!   - ndnboost::intrusive::value_traits / ndnboost::intrusive::derivation_value_traits /
//!      ndnboost::intrusive::trivial_value_traits
//!
//! Finally it forward declares the following general purpose utilities:
//!   - ndnboost::intrusive::pointer_plus_bits / ndnboost::intrusive::priority_compare. 

#if !defined(NDNBOOST_INTRUSIVE_DOXYGEN_INVOKED)

#include <cstddef>
#include <ndnboost/intrusive/link_mode.hpp>
#include <ndnboost/intrusive/detail/workaround.hpp>

namespace ndnboost {
namespace intrusive {

////////////////////////////
//     Node algorithms
////////////////////////////

//Algorithms predeclarations
template<class NodeTraits>
class circular_list_algorithms;

template<class NodeTraits>
class circular_slist_algorithms;

template<class NodeTraits>
class linear_slist_algorithms;

template<class NodeTraits>
class bstree_algorithms;

template<class NodeTraits>
class rbtree_algorithms;

template<class NodeTraits>
class avltree_algorithms;

template<class NodeTraits>
class sgtree_algorithms;

template<class NodeTraits>
class splaytree_algorithms;

template<class NodeTraits>
class treap_algorithms;

////////////////////////////
//       Containers
////////////////////////////

//slist
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class slist;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class slist_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class slist_member_hook;

//list
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class T, class ...Options>
#endif
class list;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class list_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class list_member_hook;

//rbtree/set/multiset
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class rbtree;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class multiset;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class set_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class set_member_hook;

//splaytree/splay_set/splay_multiset
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class splaytree;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class splay_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class splay_multiset;

//avltree/avl_set/avl_multiset
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class avltree;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class avl_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class avl_multiset;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class avl_set_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class avl_set_member_hook;


//treap/treap_set/treap_multiset
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class treap;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class treap_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class treap_multiset;

//sgtree/sg_set/sg_multiset
#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class sgtree;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class sg_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class sg_multiset;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class bstree;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class bs_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   >
#else
template<class T, class ...Options>
#endif
class bs_multiset;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class bs_set_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class bs_set_member_hook;

//hashtable/unordered_set/unordered_multiset

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   >
#else
template<class T, class ...Options>
#endif
class hashtable;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   >
#else
template<class T, class ...Options>
#endif
class unordered_set;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   >
#else
template<class T, class ...Options>
#endif
class unordered_multiset;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class unordered_set_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class unordered_set_member_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class any_base_hook;

#if !defined(NDNBOOST_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class any_member_hook;

//Options

template<bool Enabled>
struct constant_time_size;

template<typename SizeType>
struct size_type;

template<typename Compare>
struct compare;

template<bool Enabled>
struct floating_point;

template<typename Equal>
struct equal;

template<typename Priority>
struct priority;

template<typename Hash>
struct hash;

template<typename ValueTraits> struct value_traits;

template< typename Parent
        , typename MemberHook 
        , MemberHook Parent::* PtrToMember>
struct member_hook;

template<typename Functor>
struct function_hook;

template<typename BaseHook>
struct base_hook;

template<typename VoidPointer>
struct void_pointer;

template<typename Tag>
struct tag;

template<link_mode_type LinkType>
struct link_mode;

template<bool Enabled> struct
optimize_size;

template<bool Enabled>
struct linear;

template<bool Enabled>
struct cache_last;

template<typename BucketTraits>
struct bucket_traits;

template<bool Enabled>
struct store_hash;

template<bool Enabled>
struct optimize_multikey;

template<bool Enabled>
struct power_2_buckets;

template<bool Enabled>
struct cache_begin;

template<bool Enabled>
struct compare_hash;

template<bool Enabled>
struct incremental;

//Value traits

template<typename ValueTraits>
struct value_traits;

template< typename Parent
        , typename MemberHook
        , MemberHook Parent::* PtrToMember>
struct member_hook;

template< typename Functor>
struct function_hook;

template<typename BaseHook>
struct base_hook;

template<class T, class NodeTraits, link_mode_type LinkMode = safe_link>
struct derivation_value_traits;

template<class NodeTraits, link_mode_type LinkMode = normal_link>
struct trivial_value_traits;

//Additional utilities

template<typename VoidPointer, std::size_t Alignment> 
struct max_pointer_plus_bits;

template<std::size_t Alignment> 
struct max_pointer_plus_bits<void *, Alignment>;

template<typename Pointer, std::size_t NumBits>
struct pointer_plus_bits;

template<typename T, std::size_t NumBits> 
struct pointer_plus_bits<T *, NumBits>;

template<typename Ptr>
struct pointer_traits;

template<typename T>
struct pointer_traits<T *>;

}  //namespace intrusive {
}  //namespace ndnboost {

#endif   //#if !defined(NDNBOOST_INTRUSIVE_DOXYGEN_INVOKED)

#endif   //#ifndef NDNBOOST_INTRUSIVE_FWD_HPP
