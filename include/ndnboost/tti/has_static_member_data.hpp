
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_HAS_STATIC_MEMBER_DATA_HPP)
#define NDNBOOST_TTI_HAS_STATIC_MEMBER_DATA_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/gen/has_static_member_data_gen.hpp>
#include <ndnboost/tti/detail/dstatic_mem_data.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/// Expands to a metafunction which tests whether a static member data with a particular name and type exists.
/**

    trait = the name of the metafunction within the tti namespace.
    
    name  = the name of the inner member.

    generates a metafunction called "trait" where 'trait' is the macro parameter.
    
              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T    = the enclosing type.
                
                NDNBOOST_TTI_TP_TYPE = the static member data type,
                           in the form of a data type,
                           in which to look for our 'name'.
                       
                returns = 'value' is true if the 'name' exists,
                          with the NDNBOOST_TTI_TP_TYPE type,
                          within the enclosing NDNBOOST_TTI_TP_T type,
                          otherwise 'value' is false.
                          
*/
#define NDNBOOST_TTI_TRAIT_HAS_STATIC_MEMBER_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_STATIC_MEMBER_DATA(trait,name) \
  template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_TYPE> \
  struct trait \
    { \
    typedef typename \
    NDNBOOST_PP_CAT(trait,_detail_hsd)<NDNBOOST_TTI_TP_T,NDNBOOST_TTI_TP_TYPE>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

/// Expands to a metafunction which tests whether a static member data with a particular name and type exists.
/**

    name  = the name of the inner member.

    generates a metafunction called "has_static_member_data_name" where 'name' is the macro parameter.
    
              The metafunction types and return:
    
                NDNBOOST_TTI_TP_T    = the enclosing type.
                
                NDNBOOST_TTI_TP_TYPE = the static member data type,
                           in the form of a data type,
                           in which to look for our 'name'.
                       
                returns = 'value' is true if the 'name' exists,
                          with the appropriate NDNBOOST_TTI_TP_TYPE type,
                          within the enclosing NDNBOOST_TTI_TP_T type,
                          otherwise 'value' is false.
                          
*/
#define NDNBOOST_TTI_HAS_STATIC_MEMBER_DATA(name) \
  NDNBOOST_TTI_TRAIT_HAS_STATIC_MEMBER_DATA \
  ( \
  NDNBOOST_TTI_HAS_STATIC_MEMBER_DATA_GEN(name), \
  name \
  ) \
/**/

#endif // NDNBOOST_TTI_HAS_STATIC_MEMBER_DATA_HPP
