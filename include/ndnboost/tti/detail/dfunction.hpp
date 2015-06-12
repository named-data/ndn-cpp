
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_FUNCTION_HPP)
#define NDNBOOST_TTI_DETAIL_FUNCTION_HPP

#include <ndnboost/mpl/or.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/dmem_fun.hpp>
#include <ndnboost/tti/detail/dstatic_mem_fun.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_CALL_TYPES_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_IMPL_HAS_STATIC_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_hf) : \
    ndnboost::mpl::or_ \
    	< \
    	NDNBOOST_PP_CAT(trait,_detail_call_types)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG>, \
    	NDNBOOST_PP_CAT(trait,_detail_ihsmf) \
    		< \
    		NDNBOOST_TTI_DETAIL_TP_T, \
    		typename NDNBOOST_TTI_NAMESPACE::detail::tfunction_seq<NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG>::type \
    		> \
    	> \
    { \
    }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_FUNCTION_HPP
