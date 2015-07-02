
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_TYPE_HPP)
#define NDNBOOST_TTI_DETAIL_TYPE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/apply.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/ddeftype.hpp>
#include <ndnboost/tti/detail/dlambda.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#define NDNBOOST_TTI_DETAIL_TRAIT_INVOKE_HAS_TYPE(trait,name) \
template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_MFC> \
struct NDNBOOST_PP_CAT(trait,_detail_type_invoke) : \
  ndnboost::mpl::apply<NDNBOOST_TTI_DETAIL_TP_MFC,typename NDNBOOST_TTI_DETAIL_TP_T::name> \
  { \
  }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_OP_CHOOSE(trait,name) \
NDNBOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(NDNBOOST_PP_CAT(trait,_detail_type_mpl), name, false) \
NDNBOOST_TTI_DETAIL_TRAIT_INVOKE_HAS_TYPE(trait,name) \
template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_U,class NDNBOOST_TTI_DETAIL_TP_B> \
struct NDNBOOST_PP_CAT(trait,_detail_type_op_choose) \
  { \
  NDNBOOST_MPL_ASSERT((NDNBOOST_TTI_NAMESPACE::detail::is_lambda_expression<NDNBOOST_TTI_DETAIL_TP_U>)); \
  typedef typename NDNBOOST_PP_CAT(trait,_detail_type_invoke)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_U>::type type; \
  }; \
\
template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_U> \
struct NDNBOOST_PP_CAT(trait,_detail_type_op_choose)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_U,ndnboost::mpl::false_::type> : \
  ndnboost::mpl::false_ \
  { \
  }; \
\
template<class NDNBOOST_TTI_DETAIL_TP_T> \
struct NDNBOOST_PP_CAT(trait,_detail_type_op_choose)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_NAMESPACE::detail::deftype,ndnboost::mpl::true_::type> : \
  ndnboost::mpl::true_ \
  { \
  }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_OP(trait,name) \
NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_OP_CHOOSE(trait,name) \
template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_U> \
struct NDNBOOST_PP_CAT(trait,_detail_type_op) : \
  NDNBOOST_PP_CAT(trait,_detail_type_op_choose) \
    < \
    NDNBOOST_TTI_DETAIL_TP_T, \
    NDNBOOST_TTI_DETAIL_TP_U, \
    typename NDNBOOST_PP_CAT(trait,_detail_type_mpl)<NDNBOOST_TTI_DETAIL_TP_T>::type \
    > \
  { \
  }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE(trait,name) \
NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_OP(trait,name) \
template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_U> \
struct NDNBOOST_PP_CAT(trait,_detail_type) : \
	ndnboost::mpl::eval_if \
		< \
 		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
 		NDNBOOST_PP_CAT(trait,_detail_type_op)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_U>, \
 		ndnboost::mpl::false_ \
		> \
  { \
  }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_TYPE_HPP
