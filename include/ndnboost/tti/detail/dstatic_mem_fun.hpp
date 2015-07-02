
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_STATIC_MEM_FUN_HPP)
#define NDNBOOST_TTI_DETAIL_STATIC_MEM_FUN_HPP

#include <ndnboost/function_types/is_function.hpp>
#include <ndnboost/function_types/property_tags.hpp>
#include <ndnboost/mpl/and.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/identity.hpp>
#include <ndnboost/mpl/vector.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/dnullptr.hpp>
#include <ndnboost/tti/detail/dtfunction.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/detail/yes_no_type.hpp>

#if defined(__SUNPRO_CC)

#define NDNBOOST_TTI_DETAIL_TRAIT_IMPL_HAS_STATIC_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_ihsmf) \
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
    typedef ndnboost::mpl::bool_<sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_T>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#else

#define NDNBOOST_TTI_DETAIL_TRAIT_IMPL_HAS_STATIC_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_ihsmf) \
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
    typedef ndnboost::mpl::bool_<sizeof(chkt<NDNBOOST_TTI_DETAIL_TP_T>(NDNBOOST_TTI_DETAIL_NULLPTR))==sizeof(::ndnboost::type_traits::yes_type)> type; \
    }; \
/**/

#endif

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_FUNCTION_OP(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_IMPL_HAS_STATIC_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsmf_op) : \
    NDNBOOST_PP_CAT(trait,_detail_ihsmf) \
      < \
      NDNBOOST_TTI_DETAIL_TP_T, \
      typename \
      ndnboost::mpl::eval_if \
        < \
        ndnboost::mpl::and_ \
          < \
          ndnboost::function_types::is_function<NDNBOOST_TTI_DETAIL_TP_R>, \
          ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_FS,ndnboost::mpl::vector<> >, \
          ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_TAG,ndnboost::function_types::null_tag> \
          >, \
        ndnboost::mpl::identity<NDNBOOST_TTI_DETAIL_TP_R>, \
        NDNBOOST_TTI_NAMESPACE::detail::tfunction_seq<NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG> \
        >::type \
      > \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_FUNCTION(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_FUNCTION_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_FS,class NDNBOOST_TTI_DETAIL_TP_TAG> \
  struct NDNBOOST_PP_CAT(trait,_detail_hsmf) : \
	ndnboost::mpl::eval_if \
		< \
 		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
 		NDNBOOST_PP_CAT(trait,_detail_hsmf_op)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_R,NDNBOOST_TTI_DETAIL_TP_FS,NDNBOOST_TTI_DETAIL_TP_TAG>, \
 		ndnboost::mpl::false_ \
		> \
    { \
    }; \
/**/

#endif // NDNBOOST_TTI_DETAIL_STATIC_MEM_FUN_HPP
