
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_HAS_MEMBER_DATA_HPP)
#define NDNBOOST_TTI_HAS_MEMBER_DATA_HPP

#include <ndnboost/config.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/detail/ddeftype.hpp>
#include <ndnboost/tti/detail/dmem_data.hpp>
#include <ndnboost/tti/gen/has_member_data_gen.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/// Expands to a metafunction which tests whether a member data with a particular name and type exists.
/**

    trait = the name of the metafunction.
    
    name  = the name of the inner member to introspect.

    generates a metafunction called "trait" where 'trait' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_ET,class NDNBOOST_TTI_TP_TYPE>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_ET   = the enclosing type in which to look for our 'name'
                                                         OR
                                    The type of the member data in the form of a pointer
                                    to member data.
                
                NDNBOOST_TTI_TP_TYPE = (optional) The type of the member data if the first
                                    parameter is the enclosing type.
                
                returns  = 'value' is true if the 'name' exists, with the correct data type,
                           otherwise 'value' is false.
                          
*/
#define NDNBOOST_TTI_TRAIT_HAS_MEMBER_DATA(trait,name) \
  NDNBOOST_TTI_DETAIL_TRAIT_HAS_MEMBER_DATA(trait,name) \
  template<class NDNBOOST_TTI_TP_ET,class NDNBOOST_TTI_TP_TYPE = NDNBOOST_TTI_NAMESPACE::detail::deftype> \
  struct trait \
    { \
    typedef typename \
    NDNBOOST_PP_CAT(trait,_detail_hmd) \
      	< \
      	NDNBOOST_TTI_TP_ET, \
      	NDNBOOST_TTI_TP_TYPE \
      	>::type type; \
    NDNBOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

/// Expands to a metafunction which tests whether a member data with a particular name and type exists.
/**

    name  = the name of the inner member.

    generates a metafunction called "has_member_data_name" where 'name' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_ET,class NDNBOOST_TTI_TP_TYPE>
              struct has_member_data_name
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                NDNBOOST_TTI_TP_ET   = the enclosing type in which to look for our 'name'
                                                         OR
                                    The type of the member data in the form of a pointer
                                    to member data.
                
                NDNBOOST_TTI_TP_TYPE = (optional) The type of the member data if the first
                                    parameter is the enclosing type.
                
                returns  = 'value' is true if the 'name' exists, with the correct data type,
                           otherwise 'value' is false.
                          
*/
#define NDNBOOST_TTI_HAS_MEMBER_DATA(name) \
  NDNBOOST_TTI_TRAIT_HAS_MEMBER_DATA \
  ( \
  NDNBOOST_TTI_HAS_MEMBER_DATA_GEN(name), \
  name \
  ) \
/**/

#endif // NDNBOOST_TTI_HAS_MEMBER_DATA_HPP
