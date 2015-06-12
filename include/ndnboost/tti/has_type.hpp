
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_HAS_TYPE_HPP)
#define NDNBOOST_TTI_HAS_TYPE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/gen/has_type_gen.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/tti/detail/dtype.hpp>
#include <ndnboost/tti/detail/ddeftype.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/**

    NDNBOOST_TTI_TRAIT_HAS_TYPE is a macro which expands to a metafunction.
    The metafunction tests whether an inner type with a particular name exists
    and, optionally, whether a lambda expression invoked with the inner type 
    is true or not.
    
    trait = the name of the metafunction within the tti namespace.
    
    name  = the name of the inner type.

    generates a metafunction called "trait" where 'trait' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_U>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                NDNBOOST_TTI_TP_U = (optional) An optional template parameter, defaulting to a marker type.
                                   If specified it is an MPL lambda expression which is invoked 
                                   with the inner type found and must return a constant boolean 
                                   value.
                                   
                returns = 'value' depends on whether or not the optional NDNBOOST_TTI_TP_U is specified.
                
                          If NDNBOOST_TTI_TP_U is not specified, then 'value' is true if the 'name' type 
                          exists within the enclosing type NDNBOOST_TTI_TP_T; otherwise 'value' is false.
                          
                          If NDNBOOST_TTI_TP_U is specified , then 'value' is true if the 'name' type exists 
                          within the enclosing type NDNBOOST_TTI_TP_T and the lambda expression as specified 
                          by NDNBOOST_TTI_TP_U, invoked by passing the actual inner type of 'name', returns 
                          a 'value' of true; otherwise 'value' is false.
                             
                          The action taken with NDNBOOST_TTI_TP_U occurs only when the 'name' type exists 
                          within the enclosing type NDNBOOST_TTI_TP_T.
                             
  Example usage:
  
  NDNBOOST_TTI_TRAIT_HAS_TYPE(LookFor,MyType) generates the metafunction LookFor in the current scope
  to look for an inner type called MyType.
  
  LookFor<EnclosingType>::value is true if MyType is an inner type of EnclosingType, otherwise false.
  
  LookFor<EnclosingType,ALambdaExpression>::value is true if MyType is an inner type of EnclosingType
    and invoking ALambdaExpression with the inner type returns a value of true, otherwise false.
    
  A popular use of the optional MPL lambda expression is to check whether the type found is the same  
  as another type, when the type found is a typedef. In that case our example would be:
  
  LookFor<EnclosingType,ndnboost::is_same<_,SomeOtherType> >::value is true if MyType is an inner type
    of EnclosingType and is the same type as SomeOtherType.
  
*/
#define NDNBOOST_TTI_TRAIT_HAS_TYPE(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE(trait,name) \
  template \
    < \
    class NDNBOOST_TTI_TP_T, \
    class NDNBOOST_TTI_TP_U = NDNBOOST_TTI_NAMESPACE::detail::deftype \
    > \
  struct trait \
    { \
    typedef typename \
    NDNBOOST_PP_CAT(trait,_detail_type)<NDNBOOST_TTI_TP_T,NDNBOOST_TTI_TP_U>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

/**

    NDNBOOST_TTI_HAS_TYPE is a macro which expands to a metafunction.
    The metafunction tests whether an inner type with a particular name exists
    and, optionally, whether a lambda expression invoked with the inner type 
    is true or not.
    
    name  = the name of the inner type.

    generates a metafunction called "has_type_'name'" where 'name' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_U>
              struct has_type_'name'
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                NDNBOOST_TTI_TP_U = (optional) An optional template parameter, defaulting to a marker type.
                                   If specified it is an MPL lambda expression which is invoked 
                                   with the inner type found and must return a constant boolean 
                                   value.
                                   
                returns = 'value' depends on whether or not the optional NDNBOOST_TTI_TP_U is specified.
                
                          If NDNBOOST_TTI_TP_U is not specified, then 'value' is true if the 'name' type 
                          exists within the enclosing type NDNBOOST_TTI_TP_T; otherwise 'value' is false.
                          
                          If NDNBOOST_TTI_TP_U is specified , then 'value' is true if the 'name' type exists 
                          within the enclosing type NDNBOOST_TTI_TP_T and the lambda expression as specified 
                          by NDNBOOST_TTI_TP_U, invoked by passing the actual inner type of 'name', returns 
                          a 'value' of true; otherwise 'value' is false.
                             
                          The action taken with NDNBOOST_TTI_TP_U occurs only when the 'name' type exists 
                          within the enclosing type NDNBOOST_TTI_TP_T.
                             
  Example usage:
  
  NDNBOOST_TTI_HAS_TYPE(MyType) generates the metafunction has_type_MyType in the current scope
  to look for an inner type called MyType.
  
  has_type_MyType<EnclosingType>::value is true if MyType is an inner type of EnclosingType, otherwise false.
  
  has_type_MyType<EnclosingType,ALambdaExpression>::value is true if MyType is an inner type of EnclosingType
    and invoking ALambdaExpression with the inner type returns a value of true, otherwise false.
  
  A popular use of the optional MPL lambda expression is to check whether the type found is the same  
  as another type, when the type found is a typedef. In that case our example would be:
  
  has_type_MyType<EnclosingType,ndnboost::is_same<_,SomeOtherType> >::value is true if MyType is an inner type
    of EnclosingType and is the same type as SomeOtherType.
  
*/
#define NDNBOOST_TTI_HAS_TYPE(name) \
  NDNBOOST_TTI_TRAIT_HAS_TYPE \
  ( \
  NDNBOOST_TTI_HAS_TYPE_GEN(name), \
  name \
  ) \
/**/

#endif // NDNBOOST_TTI_HAS_TYPE_HPP
