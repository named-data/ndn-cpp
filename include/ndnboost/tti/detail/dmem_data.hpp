
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_MEM_DATA_HPP)
#define NDNBOOST_TTI_DETAIL_MEM_DATA_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/function_types/components.hpp>
#include <ndnboost/function_types/is_member_object_pointer.hpp>
#include <ndnboost/mpl/assert.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/identity.hpp>
#include <ndnboost/mpl/or.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/ddeftype.hpp>
#include <ndnboost/tti/detail/dftclass.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/type_traits/detail/yes_no_type.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/remove_const.hpp>

#if defined(NDNBOOST_MSVC) || (NDNBOOST_WORKAROUND(NDNBOOST_GCC, >= 40400) && NDNBOOST_WORKAROUND(NDNBOOST_GCC, < 40600))

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_OP(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_C> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd_op) \
    { \
    template<class> \
    struct return_of; \
    \
    template<class NDNBOOST_TTI_DETAIL_TP_R,class NDNBOOST_TTI_DETAIL_TP_IC> \
    struct return_of<NDNBOOST_TTI_DETAIL_TP_R NDNBOOST_TTI_DETAIL_TP_IC::*> \
      { \
      typedef NDNBOOST_TTI_DETAIL_TP_R type; \
      }; \
    \
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
    static ::ndnboost::type_traits::yes_type check2(NDNBOOST_TTI_DETAIL_TP_V NDNBOOST_TTI_DETAIL_TP_U::*); \
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
    struct ttc_md \
      { \
      typedef ndnboost::mpl::bool_<sizeof(has_matching_member<NDNBOOST_TTI_DETAIL_TP_V,typename return_of<NDNBOOST_TTI_DETAIL_TP_U>::type>(0))==sizeof(::ndnboost::type_traits::yes_type)> type; \
      }; \
    \
    typedef typename ttc_md<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_C>::type type; \
    \
    }; \
/**/

#else // !defined(NDNBOOST_MSVC)

#include <ndnboost/tti/detail/dmem_fun.hpp>

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_OP(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPES_MEMBER_FUNCTION(trait,name) \
  template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_C> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd_op) : \
    NDNBOOST_PP_CAT(trait,_detail_types)<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_C> \
    { \
    }; \
/**/

#endif // defined(NDNBOOST_MSVC)

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_INVOKE_ENCLOSING_CLASS(trait) \
  template<class NDNBOOST_TTI_DETAIL_TP_ET,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd_invoke_enclosing_class) : \
  	NDNBOOST_PP_CAT(trait,_detail_hmd_op) \
		< \
		typename NDNBOOST_TTI_NAMESPACE::detail::ptmd<NDNBOOST_TTI_DETAIL_TP_ET,NDNBOOST_TTI_DETAIL_TP_TYPE>::type, \
		typename ndnboost::remove_const<NDNBOOST_TTI_DETAIL_TP_ET>::type \
		> \
  	{ \
  	}; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_INVOKE_PT_MEMBER(trait) \
  template<class NDNBOOST_TTI_DETAIL_TP_ET,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd_invoke_pt_member) : \
	NDNBOOST_PP_CAT(trait,_detail_hmd_op) \
		< \
		typename NDNBOOST_TTI_NAMESPACE::detail::dmem_get_type<NDNBOOST_TTI_DETAIL_TP_ET,NDNBOOST_TTI_DETAIL_TP_TYPE>::type, \
		typename ndnboost::remove_const \
			< \
			typename NDNBOOST_TTI_NAMESPACE::detail::dmem_get_enclosing<NDNBOOST_TTI_DETAIL_TP_ET,NDNBOOST_TTI_DETAIL_TP_TYPE>::type \
			>::type \
		> \
  	{ \
  	}; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_WITH_ENCLOSING_CLASS(trait) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_INVOKE_ENCLOSING_CLASS(trait) \
  template<class NDNBOOST_TTI_DETAIL_TP_ET,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd_with_enclosing_class) : \
	ndnboost::mpl::eval_if \
		< \
 		ndnboost::is_class<NDNBOOST_TTI_DETAIL_TP_ET>, \
 		NDNBOOST_PP_CAT(trait,_detail_hmd_invoke_enclosing_class) \
 			< \
 			NDNBOOST_TTI_DETAIL_TP_ET, \
 			NDNBOOST_TTI_DETAIL_TP_TYPE \
 			>, \
 		ndnboost::mpl::false_ \
		> \
  	{ \
  	}; \
/**/

#define NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_OP(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_WITH_ENCLOSING_CLASS(trait) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA_INVOKE_PT_MEMBER(trait) \
  template<class NDNBOOST_TTI_DETAIL_TP_ET,class NDNBOOST_TTI_DETAIL_TP_TYPE> \
  struct NDNBOOST_PP_CAT(trait,_detail_hmd) : \
 	ndnboost::mpl::eval_if \
 		< \
		ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_TYPE,NDNBOOST_TTI_NAMESPACE::detail::deftype>, \
 		NDNBOOST_PP_CAT(trait,_detail_hmd_invoke_pt_member) \
 			< \
 			NDNBOOST_TTI_DETAIL_TP_ET, \
 			NDNBOOST_TTI_DETAIL_TP_TYPE \
 			>, \
 		NDNBOOST_PP_CAT(trait,_detail_hmd_with_enclosing_class) \
 			< \
 			NDNBOOST_TTI_DETAIL_TP_ET, \
 			NDNBOOST_TTI_DETAIL_TP_TYPE \
 			> \
 		> \
    { \
    }; \
/**/

namespace ndnboost
  {
  namespace tti
    {
    namespace detail
      {
      
      template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_R>
      struct ptmd
        {
        typedef NDNBOOST_TTI_DETAIL_TP_R NDNBOOST_TTI_DETAIL_TP_T::* type;
        };
        
      template<class NDNBOOST_TTI_DETAIL_TP_T>
      struct dmem_check_ptmd :
        ndnboost::mpl::identity<NDNBOOST_TTI_DETAIL_TP_T>
        {
        NDNBOOST_MPL_ASSERT((ndnboost::function_types::is_member_object_pointer<NDNBOOST_TTI_DETAIL_TP_T>));
        };
        
      template<class NDNBOOST_TTI_DETAIL_TP_T>
      struct dmem_check_ptec :
        NDNBOOST_TTI_NAMESPACE::detail::class_type<NDNBOOST_TTI_DETAIL_TP_T>
        {
        NDNBOOST_MPL_ASSERT((ndnboost::function_types::is_member_object_pointer<NDNBOOST_TTI_DETAIL_TP_T>));
        };
        
      template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_T2>
      struct dmem_get_type :
        ndnboost::mpl::eval_if
          <
          ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_T2,NDNBOOST_TTI_NAMESPACE::detail::deftype>,
          NDNBOOST_TTI_NAMESPACE::detail::dmem_check_ptmd<NDNBOOST_TTI_DETAIL_TP_T>,
          NDNBOOST_TTI_NAMESPACE::detail::ptmd<NDNBOOST_TTI_DETAIL_TP_T,NDNBOOST_TTI_DETAIL_TP_T2>
          >
        {
        };
        
      template<class NDNBOOST_TTI_DETAIL_TP_T,class NDNBOOST_TTI_DETAIL_TP_T2>
      struct dmem_get_enclosing :
        ndnboost::mpl::eval_if
          <
          ndnboost::is_same<NDNBOOST_TTI_DETAIL_TP_T2,NDNBOOST_TTI_NAMESPACE::detail::deftype>,
          NDNBOOST_TTI_NAMESPACE::detail::dmem_check_ptec<NDNBOOST_TTI_DETAIL_TP_T>,
          ndnboost::mpl::identity<NDNBOOST_TTI_DETAIL_TP_T>
          >
        {
        };
        
      }
    }
  }
  
#endif // NDNBOOST_TTI_DETAIL_MEM_DATA_HPP
