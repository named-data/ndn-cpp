//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_CONTAINER_ALLOCATOR_MEMORY_UTIL_HPP
#define NDNBOOST_CONTAINER_ALLOCATOR_MEMORY_UTIL_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

#include <ndnboost/container/detail/config_begin.hpp>
#include <ndnboost/container/detail/workaround.hpp>

#include <ndnboost/container/detail/preprocessor.hpp>
#include <ndnboost/intrusive/detail/memory_util.hpp>
#include <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>


#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME allocate
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (2, 2, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME destroy
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (1, 1, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME max_size
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (0, 0, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME select_on_container_copy_construction
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (0, 0, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME construct
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (1, NDNBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS+1, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME swap
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEGIN namespace ndnboost { namespace container { namespace container_detail {
#define NDNBOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define NDNBOOST_PP_ITERATION_PARAMS_1 (3, (1, 1, <ndnboost/intrusive/detail/has_member_function_callable_with.hpp>))
#include NDNBOOST_PP_ITERATE()

namespace ndnboost {
namespace container {
namespace container_detail {

NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(pointer)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(const_pointer)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(reference)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(const_reference)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(void_pointer)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(const_void_pointer)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(size_type)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(propagate_on_container_copy_assignment)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(propagate_on_container_move_assignment)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(propagate_on_container_swap)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(difference_type)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(value_compare)
NDNBOOST_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(wrapped_value_compare)

}  //namespace container_detail {
}  //namespace container {
}  //namespace ndnboost {

#include <ndnboost/container/detail/config_end.hpp>

#endif // ! defined(NDNBOOST_CONTAINER_ALLOCATOR_MEMORY_UTIL_HPP)
