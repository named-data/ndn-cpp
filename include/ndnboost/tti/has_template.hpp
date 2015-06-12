
//  (C) Copyright Edward Diener 2011,2012
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

#if !defined(NDNBOOST_TTI_HAS_TEMPLATE_HPP)
#define NDNBOOST_TTI_HAS_TEMPLATE_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/tti/gen/has_template_gen.hpp>
#include <ndnboost/preprocessor/config/config.hpp>
#include <ndnboost/preprocessor/control/iif.hpp>

#if NDNBOOST_PP_VARIADICS

#include <ndnboost/preprocessor/comparison/equal.hpp>
#include <ndnboost/preprocessor/variadic/elem.hpp>
#include <ndnboost/preprocessor/variadic/size.hpp>
#include <ndnboost/tti/detail/dvm_template_params.hpp>

/// Expands to a metafunction which tests whether an inner class template with a particular name exists.
/**

    trait = the name of the metafunction.
    ...   = variadic parameters.
    
            The first variadic parameter is the inner class template name.
            
            Following variadic parameters are optional.
            
            If no following variadic parameters exist, then the inner class template 
            being introspected must be all template type parameters ( template parameters 
            starting with `class` or `typename` ) and any number of template type parameters
            can occur.
            
            If the second variadic parameter is NDNBOOST_PP_NIL and no other variadic 
            parameter is given, then just as in the previous case the inner class template 
            being introspected must be all template type parameters ( template parameters 
            starting with `class` or `typename` ) and any number of template type parameters
            can occur. This form is allowed in order to be consistent with using the 
            non-variadic form of this macro.
            
            If the second variadic parameter is a Boost preprocessor library array and no other 
            variadic parameter is given, then the inner class template must have its template 
            parameters matching the sequence in the tuple portion of the Boost PP array. This 
            form is allowed in order to be consistent with using the non-variadic form of this 
            macro.
            
            Otherwise the inner class template must have its template parameters matching the 
            sequence of the optional variadic parameters.
    
    generates a metafunction called "trait" where 'trait' is the first macro parameter.
    
              template<class NDNBOOST_TTI_TP_T>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                returns = 'value' is true if the 'name' template exists within the enclosing type,
                          otherwise 'value' is false.
                          
    Examples:
    
    1) Search for an inner class template called 'MyTemplate', with all template type parameters,
       nested within the class 'MyClass' using a metafunction name of 'MyMeta'.
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate)
    
       or
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate,NDNBOOST_PP_NIL) // Non-variadic macro form
    
       MyMeta<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
    2) Search for an inner class template called 'MyTemplate', with template parameters 
       of 'class T,int x,template<class> class U', nested within the class 'MyClass' 
       using a metafunction name of 'MyMeta'.
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate,class,int,template<class> class)
    
       or
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate,(3,(class,int,template<class> class))) // Non-variadic macro form
    
       MyMeta<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
*/
#define NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(trait,...) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_PP_EQUAL \
      ( \
      NDNBOOST_PP_VARIADIC_SIZE(__VA_ARGS__), \
      1 \
      ), \
    NDNBOOST_TTI_DETAIL_VM_TRAIT_HAS_TEMPLATE, \
    NDNBOOST_TTI_DETAIL_VM_CHECK_MORE_THAN_TWO \
    ) \
    (trait,__VA_ARGS__) \
/**/

/// Expands to a metafunction which tests whether an inner class template with a particular name exists.
/**

    ...   = variadic parameters.
    
            The first variadic parameter is the inner class template name.
            
            Following variadic parameters are optional.
            
            If no following variadic parameters exist, then the inner class template 
            being introspected must be all template type parameters ( template parameters 
            starting with `class` or `typename` ) and any number of template type parameters
            can occur.
            
            If the second variadic parameter is NDNBOOST_PP_NIL and no other variadic 
            parameter is given, then just as in the previous case the inner class template 
            being introspected must be all template type parameters ( template parameters 
            starting with `class` or `typename` ) and any number of template type parameters
            can occur. This form is allowed in order to be consistent with using the 
            non-variadic form of this macro.
            
            If the second variadic parameter is a Boost preprocessor library array and no other 
            variadic parameter is given, then the inner class template must have its template 
            parameters matching the sequence in the tuple portion of the Boost PP array. This 
            form is allowed in order to be consistent with using the non-variadic form of this 
            macro.
            
            Otherwise the inner class template must have its template parameters matching the 
            sequence of the optional variadic parameters.
    
    generates a metafunction called "has_template_'name'" where 'name' is the first variadic parameter.
    
              template<class NDNBOOST_TTI_TP_T>
              struct has_template_'name'
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                returns = 'value' is true if the 'name' template exists within the enclosing type,
                          otherwise 'value' is false.
                          
    Examples:
    
    1) Search for an inner class template called 'MyTemplate', with all template type parameters,
       nested within the class 'MyClass'.
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate)
    
       or
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate,NDNBOOST_PP_NIL) // Non-variadic macro form
    
       has_template_MyTemplate<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
    2) Search for an inner class template called 'MyTemplate' with template parameters 
       of 'class T,int x,template<class> class U' nested within the class 'MyClass'.
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate,class,int,template<class> class)
    
       or
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate,(3,(class,int,template<class> class))) // Non-variadic macro form
    
       has_template_MyTemplate<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
*/
#define NDNBOOST_TTI_HAS_TEMPLATE(...) \
  NDNBOOST_TTI_TRAIT_HAS_TEMPLATE \
    ( \
    NDNBOOST_TTI_HAS_TEMPLATE_GEN \
      ( \
      NDNBOOST_PP_VARIADIC_ELEM(0,__VA_ARGS__) \
      ), \
    __VA_ARGS__ \
    ) \
/**/

#else // !NDNBOOST_PP_VARIADICS

#include <ndnboost/preprocessor/detail/is_binary.hpp>
#include <ndnboost/tti/detail/dtemplate.hpp>
#include <ndnboost/tti/detail/dtemplate_params.hpp>

/// Expands to a metafunction which tests whether an inner class template with a particular name exists.
/**

    trait  = the name of the metafunction.
    name   = the inner class template name.
    params = If the  parameter is NDNBOOST_PP_NIL the inner class template 
             being introspected must be all template type parameters ( template parameters 
             starting with `class` or `typename` ) and any number of template type parameters
             can occur.
            
             If the parameter is a Boost preprocessor library array, then the inner class 
             template must have its template parameters matching the sequence in the tuple portion 
             of the Boost PP array.
            
             Otherwise a compiler error occurs.
    
    generates a metafunction called "trait" where 'trait' is the first macro parameter.
    
              template<class NDNBOOST_TTI_TP_T>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                returns = 'value' is true if the 'name' template exists within the enclosing type,
                          otherwise 'value' is false.
                          
    Examples:
    
    1) Search for an inner class template called 'MyTemplate', with all template type parameters,
       nested within the class 'MyClass' using a metafunction name of 'MyMeta'.
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate,NDNBOOST_PP_NIL)
    
       MyMeta<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
    2) Search for an inner class template called 'MyTemplate', with template parameters 
       of 'class T,int x,template<class> class U', nested within the class 'MyClass' 
       using a metafunction name of 'MyMeta'.
    
       NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(MyMeta,MyTemplate,(3,(class,int,template<class> class)))
    
       MyMeta<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
*/
#define NDNBOOST_TTI_TRAIT_HAS_TEMPLATE(trait,name,params) \
  NDNBOOST_PP_IIF \
    ( \
    NDNBOOST_PP_IS_BINARY(params), \
    NDNBOOST_TTI_DETAIL_TRAIT_HAS_TEMPLATE_CHECK_PARAMS, \
    NDNBOOST_TTI_DETAIL_TRAIT_CHECK_IS_NIL \
    ) \
    (trait,name,params) \
/**/
  
/// Expands to a metafunction which tests whether an inner class template with a particular name exists.
/**

    name   = the inner class template name.
    params = If the  parameter is NDNBOOST_PP_NIL the inner class template 
             being introspected must be all template type parameters ( template parameters 
             starting with `class` or `typename` ) and any number of template type parameters
             can occur.
            
             If the parameter is a Boost preprocessor library array, then the inner class 
             template must have its template parameters matching the sequence in the tuple portion 
             of the Boost PP array.
            
             Otherwise a compiler error occurs.
    
    generates a metafunction called "has_template_'name'" where 'name' is the first macro parameter.
    
              template<class NDNBOOST_TTI_TP_T>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                returns = 'value' is true if the 'name' template exists within the enclosing type,
                          otherwise 'value' is false.
                          
    Examples:
    
    1) Search for an inner class template called 'MyTemplate', with all template type parameters,
       nested within the class 'MyClass'.
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate,NDNBOOST_PP_NIL)
    
       has_template_MyTemplate<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
    
    2) Search for an inner class template called 'MyTemplate' with template parameters 
       of 'class T,int x,template<class> class U' nested within the class 'MyClass'.
    
       NDNBOOST_TTI_HAS_TEMPLATE(MyTemplate,(3,(class,int,template<class> class)))
    
       has_template_MyTemplate<MyClass>::value
    
       is a compile time boolean constant which is either 'true' or 'false'
       if the nested template exists.
       
*/
#define NDNBOOST_TTI_HAS_TEMPLATE(name,params) \
  NDNBOOST_TTI_TRAIT_HAS_TEMPLATE \
  ( \
  NDNBOOST_TTI_HAS_TEMPLATE_GEN(name), \
  name, \
  params \
  ) \
/**/

#endif // NDNBOOST_PP_VARIADICS
#endif // NDNBOOST_TTI_HAS_TEMPLATE_HPP
