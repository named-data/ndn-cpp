
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_MEM_TYPE_HPP)
#define NDNBOOST_TTI_DETAIL_MEM_TYPE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_MEMBER_TYPE_OP(trait,name) \
  NDNBOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(NDNBOOST_PP_CAT(trait,_detail_mpl), name, false) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_op) : \
    NDNBOOST_PP_CAT(trait,_detail_mpl)<NDNBOOST_TTI_DETAIL_TP_T> \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_MEMBER_TYPE(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_MEMBER_TYPE_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail) \
    { \
    typedef typename \
    ndnboost::mpl::eval_if \
    	< \
    	ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
    	NDNBOOST_PP_CAT(trait,_detail_op)<NDNBOOST_TTI_DETAIL_TP_T>, \
    	ndnboost::mpl::false_ \
    	>::type type; \
    \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_MEMBER_TYPE(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_member_type) \
    { \
    typedef typename NDNBOOST_TTI_DETAIL_TP_T::name type; \
    }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_MEM_TYPE_HPP
