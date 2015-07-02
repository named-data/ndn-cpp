
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_VM_TEMPLATE_PARAMS_HPP)
#define NDNBOOST_TTI_DETAIL_VM_TEMPLATE_PARAMS_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/preprocessor/config/config.hpp>

#if NDNBOOST_PP_VARIADICS

#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/mpl/identity.hpp>
#include <ndnboost/preprocessor/arithmetic/add.hpp>
#include <ndnboost/preprocessor/variadic/size.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/preprocessor/comparison/equal.hpp>
#include <ndnboost/preprocessor/control/iif.hpp>
#include <ndnboost/preprocessor/detail/is_binary.hpp>
#include <ndnboost/preprocessor/facilities/is_empty.hpp>
#include <ndnboost/preprocessor/seq/enum.hpp>
#include <ndnboost/preprocessor/seq/seq.hpp>
#include <ndnboost/preprocessor/variadic/elem.hpp>
#include <ndnboost/preprocessor/variadic/to_seq.hpp>
#include <ndnboost/tti/detail/dtemplate.hpp>
#include <ndnboost/tti/detail/dtemplate_params.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#if !defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)
#if !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(trait,name,...) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_FUNCTION_SFINAE \
    (  \
      ( NDNBOOST_PP_ADD(NDNBOOST_PP_VARIADIC_SIZE(__VA_ARGS__),4), ( trait, name, 1, false, __VA_ARGS__ ) )  \
    )  \
/**/

#else // !!NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(trait,name,...) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_TEMPLATE_SFINAE \
    ( \
      ( NDNBOOST_PP_ADD(NDNBOOST_PP_VARIADIC_SIZE(__VA_ARGS__),4), ( trait, name, 1, false, __VA_ARGS__ ) )  \
    ) \
/**/

#endif // !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)
#else // defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(trait,name,...) \
  NDNBOOST_TTI_DETAIL_SAME(trait,name) \
/**/

#endif // !defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)

#define NDNBOOST_TTI_DETAIL_VM_CHECK_MORE_THAN_TWO(trait,...) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_PP_EQUAL \
      ( \
      NDNBOOST_PP_VARIADIC_SIZE(__VA_ARGS__), \
      2 \
      ), \
    NDNBOOST_TTI_DETAIL_VM_TRAIT_CHOOSE_FROM_TWO, \
    NDNBOOST_TTI_DETAIL_VM_TRAIT_EXPAND_ARGUMENTS \
    ) \
    (trait,__VA_ARGS__) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_CHOOSE_FROM_TWO(trait,...) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_PP_IS_BINARY \
      ( \
      NDNBOOST_PP_VARIADIC_ELEM(1,__VA_ARGS__) \
      ), \
    NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS, \
    NDNBOOST_TTI_DETAIL_VM_TRAIT_CHOOSE_IF_NIL \
    ) \
  ( \
  trait, \
  NDNBOOST_PP_VARIADIC_ELEM(0,__VA_ARGS__), \
  NDNBOOST_PP_VARIADIC_ELEM(1,__VA_ARGS__) \
  ) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_IS_NIL(param) \
  NDNBOOST_PP_IS_EMPTY \
    ( \
    NDNBOOST_PP_CAT(NDNBOOST_TTI_DETAIL_IS_HELPER_,param) \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_CHOOSE_IF_NIL(trait,name,param) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_TTI_DETAIL_VM_IS_NIL(param), \
    NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE, \
    NDNBOOST_TTI_DETAIL_VM_CALL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS \
    ) \
  (trait,name,param) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_VARIADIC_TAIL(...) \
  NDNBOOST_PP_SEQ_ENUM \
    ( \
    NDNBOOST_PP_SEQ_TAIL \
      ( \
      NDNBOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
      ) \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_EXPAND_ARGUMENTS(trait,...) \
  NDNBOOST_TTI_DETAIL_VM_CALL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS \
    ( \
    trait, \
    NDNBOOST_PP_VARIADIC_ELEM(0,__VA_ARGS__), \
    NDNBOOST_TTI_DETAIL_VM_VARIADIC_TAIL(__VA_ARGS__) \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE(trait,...) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE \
    ( \
    trait, \
    NDNBOOST_PP_VARIADIC_ELEM(0,__VA_ARGS__), \
    NDNBOOST_PP_NIL \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_VM_CT_INVOKE(trait,name,...) \
  NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(NDNBOOST_PP_CAT(trait,_detail),name,__VA_ARGS__) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_vm_ct_invoke) : \
  	NDNBOOST_PP_CAT(trait,_detail)<NDNBOOST_TTI_DETAIL_TP_T> \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_VM_CALL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(trait,name,...) \
  NDNBOOST_TTI_DETAIL_VM_CT_INVOKE(trait,name,__VA_ARGS__) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct trait \
    { \
    typedef typename \
  	ndnboost::mpl::eval_if \
  		< \
  		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
  		NDNBOOST_PP_CAT(trait,_detail_vm_ct_invoke)<NDNBOOST_TTI_DETAIL_TP_T>, \
  		ndnboost::mpl::false_ \
  		>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

#endif // NDNBOOST_PP_VARIADICS

#endif // NDNBOOST_TTI_DETAIL_VM_TEMPLATE_PARAMS_HPP
