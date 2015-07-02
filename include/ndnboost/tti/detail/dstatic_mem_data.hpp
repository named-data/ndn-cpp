
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_STATIC_MEM_DATA_HPP)
#define NDNBOOST_TTI_DETAIL_STATIC_MEM_DATA_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/function_types/is_function.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/type_traits/detail/yes_no_type.hpp>
#include <ndnboost/tti/detail/dnullptr.hpp>

#if defined(NDNBOOST_MSVC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsd_op) \
    { \
    template<bool,typename NDNBOOST_TTI_DETAIL_TP_U> \
    struct menable_if; \
    \
    template<typename NDNBOOST_TTI_DETAIL_TP_U> \
    struct menable_if<true,NDNBOOST_TTI_DETAIL_TP_U> \
      { \
      typedef NDNBOOST_TTI_DETAIL_TP_U type; \
      }; \
    \
    template<typename NDNBOOST_TTI_DETAIL_TP_U,typename NDNBOOST_TTI_DETAIL_TP_V> \
    static ::ndnboost::type_traits::yes_type check2(NDNBOOST_TTI_DETAIL_TP_V *); \
    \
    template<typename NDNBOOST_TTI_DETAIL_TP_U,typename NDNBOOST_TTI_DETAIL_TP_V> \
    static ::ndnboost::type_traits::no_type check2(NDNBOOST_TTI_DETAIL_TP_U); \
    \
    template<typename NDNBOOST_TTI_DETAIL_TP_U,typename NDNBOOST_TTI_DETAIL_TP_V> \
    static typename \
      menable_if \
        < \
        sizeof(check2<NDNBOOST_TTI_DETAIL_TP_U,NDNBOOST_TTI_DETAIL_TP_V>(&NDNBOOST_TTI_DETAIL_TP_U::name))==sizeof(::ndnboost::type_traits::yes_type), \
        ::ndnboost::type_traits::yes_type \
        > \
      ::type \
    has_matching_member(int); \
    \
    template<typename NDNBOOST_TTI_DETAIL_TP_U,typename NDNBOOST_TTI_DETAIL_TP_V> \
    static ::ndnboost::type_traits::no_type has_matching_member(...); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U,class NDNBOOST_TTI_DETAIL_TP_V> \
    struct ttc_sd \
      { \
      typedef ndnboost::mpl::bool_<sizeof(has_matching_member<NDNBOOST_TTI_DETAIL_TP_V,NDNBOOST_TTI_DETAIL_TP_U>(0))==sizeof(::ndnboost::type_traits::yes_type)> type; \
      }; \
    \
    typedef typename ttc_sd<NDNBOOST_TTI_DETAIL_TP_TYPE,NDNBOOST_TTI_DETAIL_TP_T>::type type; \
    }; \
/**/

#elif defined(__SUNPRO_CC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsd_op) \
    { \
    template<NDNBOOST_TTI_DETAIL_TP_TYPE *> \
    struct helper {}; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_U::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<(!ndnboost::function_types::is_function<NDNBOOST_TTI_DETAIL_TP_TYPE>::value) && (sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_T>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type))> type; \
    }; \
/**/

#else

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsd_op) \
    { \
    template<NDNBOOST_TTI_DETAIL_TP_TYPE *> \
    struct helper; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_U::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<(!ndnboost::function_types::is_function<NDNBOOST_TTI_DETAIL_TP_TYPE>::value) && (sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_T>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type))> type; \
    }; \
/**/

#endif // defined(NDNBOOST_MSVC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsd) : \
	ndnboost::mpl::eval_if \
		< \
 		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
 		NDNBOOST_PP_CAT(trait,_detail_hsd_op)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_TYPE>, \
 		ndnboost::mpl::false_ \
		> \
    { \
    }; \
/**/
    
#endif // NDNBOOST_TTI_DETAIL_STATIC_MEM_DATA_HPP
