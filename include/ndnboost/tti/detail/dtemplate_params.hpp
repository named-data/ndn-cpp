
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMS_HPP)
#define NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMS_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/preprocessor/arithmetic/add.hpp>
#include <ndnboost/preprocessor/arithmetic/sub.hpp>
#include <ndnboost/preprocessor/array/elem.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/preprocessor/punctuation/comma_if.hpp>
#include <ndnboost/preprocessor/repetition/repeat.hpp>
#include <ndnboost/preprocessor/repetition/enum.hpp>
#include <ndnboost/preprocessor/array/enum.hpp>
#include <ndnboost/preprocessor/array/size.hpp>
#include <ndnboost/type_traits/is_class.hpp>

#if !defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)

#define NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMETERS(z,n,args) \
NDNBOOST_PP_ARRAY_ELEM(NDNBOOST_PP_ADD(4,n),args) \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_IMPLEMENTATION(args,introspect_macro) \
   template \
     < \
     typename NDNBOOST_TTI_DETAIL_TP_T, \
     typename NDNBOOST_TTI_DETAIL_TP_FALLBACK_ \
       = ndnboost::mpl::bool_< NDNBOOST_PP_ARRAY_ELEM(3, args) > \
     > \
   struct NDNBOOST_PP_ARRAY_ELEM(0, args) \
     { \
     private: \
     introspect_macro(args) \
     public: \
       static const bool value \
         = NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_NAME(args)< NDNBOOST_TTI_DETAIL_TP_T >::value; \
       typedef typename NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) \
         < \
         NDNBOOST_TTI_DETAIL_TP_T \
         >::type type; \
     }; \
/**/

#if !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_MULTI_SUBSTITUTE(z,n,args) \
  template \
    < \
    template \
      < \
      NDNBOOST_PP_ENUM_ ## z \
        ( \
        NDNBOOST_PP_SUB \
          ( \
          NDNBOOST_PP_ARRAY_SIZE(args), \
          4 \
          ), \
        NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMETERS, \
        args \
        ) \
      > \
    class NDNBOOST_TTI_DETAIL_TM_V \
    > \
  struct NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME(args, n) \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_SUBSTITUTE(args) \
  NDNBOOST_PP_REPEAT \
    ( \
    NDNBOOST_PP_ARRAY_ELEM(2, args), \
    NDNBOOST_TTI_DETAIL_HAS_MEMBER_MULTI_SUBSTITUTE, \
    args \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_INTROSPECT(args) \
  template< typename U > \
  struct NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) \
    { \
    NDNBOOST_TTI_DETAIL_HAS_MEMBER_SUBSTITUTE(args) \
    NDNBOOST_MPL_HAS_MEMBER_REJECT(args, NDNBOOST_PP_NIL) \
    NDNBOOST_MPL_HAS_MEMBER_ACCEPT(args, NDNBOOST_PP_NIL) \
    NDNBOOST_STATIC_CONSTANT \
      ( \
      bool, value = NDNBOOST_MPL_HAS_MEMBER_TEST(args) \
      ); \
    typedef ndnboost::mpl::bool_< value > type; \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_FUNCTION_SFINAE(args) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_IMPLEMENTATION \
    ( \
    args, \
    NDNBOOST_TTI_DETAIL_HAS_MEMBER_INTROSPECT \
    ) \
/**/

#else // !!NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_MULTI_SUBSTITUTE_WITH_TEMPLATE_SFINAE(z,n,args) \
  template \
    < \
    template \
      < \
      NDNBOOST_PP_ENUM_ ## z \
        ( \
        NDNBOOST_PP_SUB \
          ( \
          NDNBOOST_PP_ARRAY_SIZE(args), \
          4 \
          ), \
        NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMETERS, \
        args \
        ) \
      > \
    class NDNBOOST_TTI_DETAIL_TM_U \
    > \
  struct NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_NAME_WITH_TEMPLATE_SFINAE \
    ( \
    args, \
    n \
    ) \
    { \
    typedef \
      NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME(args) \
      type; \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_SUBSTITUTE_WITH_TEMPLATE_SFINAE(args) \
  typedef void \
      NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_SUBSTITUTE_TAG_NAME(args); \
  NDNBOOST_PP_REPEAT \
    ( \
    NDNBOOST_PP_ARRAY_ELEM(2, args), \
    NDNBOOST_TTI_DETAIL_HAS_MEMBER_MULTI_SUBSTITUTE_WITH_TEMPLATE_SFINAE, \
    args \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_INTROSPECT_WITH_TEMPLATE_SFINAE(args) \
  NDNBOOST_MPL_HAS_MEMBER_REJECT_WITH_TEMPLATE_SFINAE(args,NDNBOOST_PP_NIL) \
  NDNBOOST_MPL_HAS_MEMBER_ACCEPT_WITH_TEMPLATE_SFINAE(args,NDNBOOST_PP_NIL) \
  template< typename NDNBOOST_TTI_DETAIL_TP_U > \
  struct NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_NAME(args) \
      : NDNBOOST_MPL_HAS_MEMBER_INTROSPECTION_TEST_NAME(args)< NDNBOOST_TTI_DETAIL_TP_U > { \
  }; \
/**/

#define NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_TEMPLATE_SFINAE(args) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_SUBSTITUTE_WITH_TEMPLATE_SFINAE \
    ( \
    args \
    ) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_IMPLEMENTATION \
    ( \
    args, \
    NDNBOOST_TTI_DETAIL_HAS_MEMBER_INTROSPECT_WITH_TEMPLATE_SFINAE \
    ) \
/**/

#endif // !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#else // defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)

#define NDNBOOST_TTI_DETAIL_SAME(trait,name) \
  NDNBOOST_MPL_HAS_XXX_TEMPLATE_NAMED_DEF \
    ( \
    trait, \
    name, \
    false \
    ) \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_CALL_HAS_TEMPLATE_CHECK_PARAMS(trait,name,tp) \
  NDNBOOST_TTI_DETAIL_SAME(trait,name) \
/**/

#endif // !NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS_OP(trait,name,tpArray) \
  NDNBOOST_TTI_DETAIL_TRAIT_CALL_HAS_TEMPLATE_CHECK_PARAMS(NDNBOOST_PP_CAT(trait,_detail),name,tpArray) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct NDNBOOST_PP_CAT(trait,_detail_cp_op) : \
    NDNBOOST_PP_CAT(trait,_detail)<NDNBOOST_TTI_DETAIL_TP_T> \
    { \
    }; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS(trait,name,tpArray) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS_OP(trait,name,tpArray) \
  template<class NDNBOOST_TTI_DETAIL_TP_T> \
  struct trait \
    { \
    typedef typename \
  	ndnboost::mpl::eval_if \
  		< \
  		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_T>, \
  		NDNBOOST_PP_CAT(trait,_detail_cp_op)<NDNBOOST_TTI_DETAIL_TP_T>, \
  		ndnboost::mpl::false_ \
  		>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

#if !defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)
#if !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_TRAIT_CALL_HAS_TEMPLATE_CHECK_PARAMS(trait,name,tpArray) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_FUNCTION_SFINAE \
    (  \
      ( NDNBOOST_PP_ADD(NDNBOOST_PP_ARRAY_SIZE(tpArray),4), ( trait, name, 1, false, NDNBOOST_PP_ARRAY_ENUM(tpArray) ) )  \
    )  \
/**/

#else // NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)

#define NDNBOOST_TTI_DETAIL_TRAIT_CALL_HAS_TEMPLATE_CHECK_PARAMS(trait,name,tpArray) \
  NDNBOOST_TTI_DETAIL_HAS_MEMBER_WITH_TEMPLATE_SFINAE \
    ( \
      ( NDNBOOST_PP_ADD(NDNBOOST_PP_ARRAY_SIZE(tpArray),4), ( trait, name, 1, false, NDNBOOST_PP_ARRAY_ENUM(tpArray) ) )  \
    ) \
/**/

#endif // !NDNBOOST_WORKAROUND(NDNBOOST_MSVC, <= 1400)
#endif // !defined(NDNBOOST_MPL_CFG_NO_HAS_XXX_TEMPLATE)

#endif // NDNBOOST_TTI_DETAIL_TEMPLATE_PARAMS_HPP
