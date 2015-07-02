
// (C) Copyright Tobias Schwinger
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

//------------------------------------------------------------------------------

#ifndef NDNBOOST_FT_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
#define NDNBOOST_FT_MEMBER_FUNCTION_POINTER_HPP_INCLUDED

#include <ndnboost/mpl/aux_/lambda_support.hpp>
#include <ndnboost/type_traits/detail/template_arity_spec.hpp>

#include <ndnboost/function_types/detail/synthesize.hpp>
#include <ndnboost/function_types/detail/to_sequence.hpp>

namespace ndnboost 
{ 
  namespace function_types 
  {
    template<typename Types, typename Tag = null_tag> 
    struct member_function_pointer
      : detail::synthesize_mfp< typename detail::to_sequence<Types>::type, Tag >
    { 
      NDNBOOST_MPL_AUX_LAMBDA_SUPPORT(2,member_function_pointer,(Types,Tag))
    };
  } 
  NDNBOOST_TT_AUX_TEMPLATE_ARITY_SPEC(2,function_types::member_function_pointer)
} 

#endif

