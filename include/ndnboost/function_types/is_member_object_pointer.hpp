
// (C) Copyright Tobias Schwinger
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

//------------------------------------------------------------------------------

#ifndef NDNBOOST_FT_IS_MEMBER_OBJECT_POINTER_HPP_INCLUDED
#define NDNBOOST_FT_IS_MEMBER_OBJECT_POINTER_HPP_INCLUDED

#include <ndnboost/mpl/aux_/lambda_support.hpp>
#include <ndnboost/type_traits/detail/template_arity_spec.hpp>

#include <ndnboost/function_types/components.hpp>

namespace ndnboost 
{ 
  namespace function_types 
  {
    template< typename T > 
    struct is_member_object_pointer
      : function_types::detail::represents_impl
        < function_types::components<T>
        , detail::member_object_pointer_tag >
    { 
      NDNBOOST_MPL_AUX_LAMBDA_SUPPORT(1,is_member_object_pointer,(T))
    };
  }
  NDNBOOST_TT_AUX_TEMPLATE_ARITY_SPEC(1,function_types::is_member_object_pointer)
} 

#endif 

