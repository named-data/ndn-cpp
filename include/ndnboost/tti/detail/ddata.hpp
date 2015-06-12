
//  (C) Copyright Edward Diener 2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_DATA_HPP)
#define NDNBOOST_TTI_DETAIL_DATA_HPP

#include <ndnboost/mpl/or.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/dmem_data.hpp>
#include <ndnboost/tti/detail/dstatic_mem_data.hpp>

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_ET,class NDNBOOST_TTI_DETAIL_TP_DT> \
  struct NDNBOOST_PP_CAT(trait,_detail_hd) : \
    ndnboost::mpl::or_ \
    	< \
    	NDNBOOST_PP_CAT(trait,_detail_hmd_with_enclosing_class)<NDNBOOST_TTI_DETAIL_TP_ET,NDNBOOST_TTI_DETAIL_TP_DT>, \
    	NDNBOOST_PP_CAT(trait,_detail_hsd)<NDNBOOST_TTI_DETAIL_TP_ET,NDNBOOST_TTI_DETAIL_TP_DT> \
    	> \
    { \
    }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_DATA_HPP
