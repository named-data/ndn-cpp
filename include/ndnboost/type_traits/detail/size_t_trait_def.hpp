
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// $Source$
// $Date$
// $Revision$

#include <ndnboost/type_traits/detail/template_arity_spec.hpp>
#include <ndnboost/type_traits/integral_constant.hpp>
#include <ndnboost/mpl/aux_/lambda_support.hpp>
#include <ndnboost/mpl/size_t.hpp>

#include <cstddef>

// Obsolete. Remove.
#define NDNBOOST_TT_AUX_SIZE_T_BASE(C) public ::ndnboost::integral_constant<std::size_t,C>
#define NDNBOOST_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) /**/


#define NDNBOOST_TT_AUX_SIZE_T_TRAIT_DEF1(trait,T,C) \
template< typename T > struct trait \
    : public ::ndnboost::integral_constant<std::size_t,C> \
{ \
public:\
    NDNBOOST_MPL_AUX_LAMBDA_SUPPORT(1,trait,(T)) \
}; \
\
NDNBOOST_TT_AUX_TEMPLATE_ARITY_SPEC(1,trait) \
/**/

#define NDNBOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(trait,spec,C) \
template<> struct trait<spec> \
    : public ::ndnboost::integral_constant<std::size_t,C> \
{ \
public:\
    NDNBOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,trait,(spec)) \
}; \
/**/

#define NDNBOOST_TT_AUX_SIZE_T_TRAIT_PARTIAL_SPEC1_1(param,trait,spec,C) \
template< param > struct trait<spec> \
    : public ::ndnboost::integral_constant<std::size_t,C> \
{ \
}; \
/**/
