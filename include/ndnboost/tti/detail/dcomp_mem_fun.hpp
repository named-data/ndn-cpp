
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_COMP_MEM_FUN_HPP)
#define NDNBOOST_TTI_DETAIL_COMP_MEM_FUN_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/dftclass.hpp>
#include <ndnboost/tti/detail/dnullptr.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/type_traits/remove_const.hpp>
#include <ndnboost/type_traits/detail/yes_no_type.hpp>

#if defined(__SUNPRO_CC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_COMP_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_hcmf) \
    { \
    template<class NDNBOOST_TTI_DETAIL_TP_F> \
    struct cl_type : \
      ndnboost::remove_const \
        < \
        typename NDNBOOST_TTI_NAMESPACE::detail::class_type<NDNBOOST_TTI_DETAIL_TP_F>::type \
        > \
      { \
      }; \
    \
    template<NDNBOOST_TTI_DETAIL_TP_T> \
    struct helper {}; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_U::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<sizeof(chkt<typename cl_type<NDNBOOST_TTI_DETAIL_TP_T>::type>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#else

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_COMP_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_hcmf) \
    { \
    template<class NDNBOOST_TTI_DETAIL_TP_F> \
    struct cl_type : \
      ndnboost::remove_const \
        < \
        typename NDNBOOST_TTI_NAMESPACE::detail::class_type<NDNBOOST_TTI_DETAIL_TP_F>::type \
        > \
      { \
      }; \
    \
    template<NDNBOOST_TTI_DETAIL_TP_T> \
    struct helper; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::yes_type chkt(helper<&NDNBOOST_TTI_DETAIL_TP_U::name> *); \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_U> \
    static ::ndnboost::type_traits::no_type chkt(...); \
    \
    typedef ndnboost::mpl::bool_<sizeof(chkt<typename cl_type<NDNBOOST_TTI_DETAIL_TP_T>::type>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#endif


#endif // NDNBOOST_TTI_DETAIL_COMP_MEM_FUN_HPP
