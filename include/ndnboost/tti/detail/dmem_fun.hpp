
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_MEM_FUN_HPP)
#define NDNBOOST_TTI_DETAIL_MEM_FUN_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/function_types/is_member_function_pointer.hpp>
#include <ndnboost/function_types/property_tags.hpp>
#include <ndnboost/mpl/and.hpp>
#include <ndnboost/mpl/logical.hpp>
#include <ndnboost/mpl/assert.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/vector.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/type_traits/detail/yes_no_type.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/remove_const.hpp>
#include <ndnboost/tti/detail/dcomp_mem_fun.hpp>
#include <ndnboost/tti/detail/ddeftype.hpp>
#include <ndnboost/tti/detail/dnullptr.hpp>
#include <ndnboost/tti/detail/dptmf.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>

#if defined(__SUNPRO_CC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPES_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_PMEMF,class NDNBOOST_TTI_DETAIL_TP_C> \
  struct NDNBOOST_PP_CAT(trait,_detail_types) \
    { \
    template<NDNBOOST_TTI_DETAIL_TP_PMEMF> \
    struct helper {}; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_EC> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_EC::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_EC> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_C>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#else

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPES_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_PMEMF,class NDNBOOST_TTI_DETAIL_TP_C> \
  struct NDNBOOST_PP_CAT(trait,_detail_types) \
    { \
    template<NDNBOOST_TTI_DETAIL_TP_PMEMF> \
    struct helper; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_EC> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_EC::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_EC> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_C>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#endif

#define NDNBOOST_TTI_DETAIL_TRAIT_CTMF_INVOKE(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPES_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_ctmf_invoke) : \
    NDNBOOST_PP_CAT(trait,_detail_types) \
      < \
      typename NDNBOOST_TTI_NAMESPACE::detail::ptmf_seq<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG>::type, \
      NDNBOOST_TTI_DETAIL_TP_T \
      > \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_CALL_TYPES_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_CTMF_INVOKE(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_call_types) : \
	ndnboost::mpl::eval_if \
		< \
 		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
 		NDNBOOST_PP_CAT(trait,_detail_ctmf_invoke) \
 			< \
 			NDNBOOST_TTI_DETAIL_TP_T, \
 			NDNBOOST_TTI_DETAIL_TP_R, \
 			NDNBOOST_TTI_DETAIL_TP_FS, \
 			NDNBOOST_TTI_DETAIL_TP_TAG \
 			>, \
 		ndnboost::mpl::false_ \
		> \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_CHECK_HAS_COMP_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_COMP_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_check_comp) : \
    NDNBOOST_PP_CAT(trait,_detail_hcmf)<NDNBOOST_TTI_DETAIL_TP_T> \
    { \
    NDNBOOST_MPL_ASSERT((ndnboost::function_types::is_member_function_pointer<NDNBOOST_TTI_DETAIL_TP_T>)); \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_CALL_TYPES_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_CHECK_HAS_COMP_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmf) : \
    ndnboost::mpl::eval_if \
      < \
      ndnboost::mpl::and_ \
        < \
        ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_NAMESPACE::detail::deftype>, \
        ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_FS,ndnboost::mpl::vector<> >, \
        ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_TAG,ndnboost::function_types::null_tag> \
        >, \
      NDNBOOST_PP_CAT(trait,_detail_check_comp)<NDNBOOST_TTI_DETAIL_TP_T>, \
      NDNBOOST_PP_CAT(trait,_detail_call_types)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG> \
      > \
    { \
    }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_MEM_FUN_HPP
