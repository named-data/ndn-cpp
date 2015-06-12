
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_TEMPLATE_HPP)
#define NDNBOOST_TTI_DETAIL_TEMPLATE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/preprocessor/debug/assert.hpp>
#include <ndnboost/preprocessor/facilities/is_empty.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#define NDNBOOST_TTI_DETAIL_IS_HELPER_NDNBOOST_PP_NIL

#define NDNBOOST_TTI_DETAIL_IS_NIL(param) \
  NDNBOOST_PP_IS_EMPTY \
    ( \
    NDNBOOST_PP_CAT(NDNBOOST_TTI_DETAIL_IS_HELPER_,param) \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_ASSERT_NOT_NIL(trait,name,params) \
  NDNBOOST_PP_ASSERT_MSG(0, "The parameter must be NDNBOOST_PP_NIL") \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_CHECK_IS_NIL(trait,name,params) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_TTI_DETAIL_IS_NIL(params), \
    NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE, \
    NDNBOOST_TTI_DETAIL_TRAIT_ASSERT_NOT_NIL \
    ) \
    (trait,name,params) \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_THT(trait,name) \
  NDNBOOST_MPL_HAS_XXX_TEMPLATE_NAMED_DEF(NDNBOOST_PP_CAT(trait,_detail_mpl), name, false) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_tht) : \
    NDNBOOST_PP_CAT(trait,_detail_mpl)<NDNBOOST_TTI_DETAIL_TP_T> \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE(trait,name,params) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_THT(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct trait \
    { \
    typedef typename \
  	ndnboost::mpl::eval_if \
  		< \
  		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
  		NDNBOOST_PP_CAT(trait,_tht)<NDNBOOST_TTI_DETAIL_TP_T>, \
  		ndnboost::mpl::false_ \
  		>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

#endif // !NDNBOOST_TTI_DETAIL_TEMPLATE_HPP
