
//  (C) Copyright Edward Diener 2011,2012,2013
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_MEMBER_TYPE_HPP)
#define NDNBOOST_TTI_MEMBER_TYPE_HPP
  
#include <ndnboost/config.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/identity.hpp>
#include <ndnboost/mpl/not.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/preprocessor/cat.hpp>
#include <ndnboost/tti/gen/member_type_gen.hpp>
#include <ndnboost/tti/gen/namespace_gen.hpp>
#include <ndnboost/tti/detail/dmem_type.hpp>
#include <ndnboost/tti/detail/dnotype.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/// Expands to a metafunction whose typedef 'type' is either the named type or a marker type.
/**

    trait = the name of the metafunction within the tti namespace.
    
    name  = the name of the inner type.

    generates a metafunction called "trait" where 'trait' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_MARKER_TYPE = ndnboost::tti::detail::notype>
              struct trait
                {
                typedef unspecified type;
                
                typedef NDNBOOST_TTI_TP_MARKER_TYPE boost_tti_marker_type;
                };

              The metafunction types and return:
              
                NDNBOOST_TTI_TP_T           = the enclosing type.
                NDNBOOST_TTI_TP_MARKER_TYPE = (optional) a type to use as the marker type.
                                  defaults to the internal ndnboost::tti::detail::notype.
                
                returns         = 'type' is the inner type of 'name' if the inner type exists
                                  within the enclosing type, else 'type' is a marker type.
                                  if the end-user does not specify a marker type then
                                  an internal ndnboost::tti::detail::notype marker type is used.
                          
                The metafunction also encapsulates the type of the marker type as
                a nested 'boost_tti_marker_type'.
                          
    The purpose of this macro is to encapsulate the 'name' type as the typedef 'type'
    of a metafunction, but only if it exists within the enclosing type. This allows for
    an evaluation of inner type existence, without generating a compiler error,
    which can be used by other metafunctions in this library.
    
*/
#define NDNBOOST_TTI_TRAIT_MEMBER_TYPE(trait,name) \
    NDNBOOST_TTI_DETAIL_TRAIT_HAS_TYPE_MEMBER_TYPE(trait,name) \
    NDNBOOST_TTI_DETAIL_TRAIT_MEMBER_TYPE(trait,name) \
    template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_MARKER_TYPE = NDNBOOST_TTI_NAMESPACE::detail::notype> \
    struct trait : \
      ndnboost::mpl::eval_if \
        < \
        NDNBOOST_PP_CAT(trait,_detail)<NDNBOOST_TTI_TP_T>, \
        NDNBOOST_PP_CAT(trait,_detail_member_type)<NDNBOOST_TTI_TP_T>, \
        ndnboost::mpl::identity<NDNBOOST_TTI_TP_MARKER_TYPE> \
        > \
      { \
      typedef NDNBOOST_TTI_TP_MARKER_TYPE boost_tti_marker_type; \
      }; \
/**/

/// Expands to a metafunction whose typedef 'type' is either the named type or a marker type.
/**

    name  = the name of the inner type.

    generates a metafunction called "member_type_name" where 'name' is the macro parameter.
    
              template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_MARKER_TYPE = ndnboost::tti::detail::notype>
              struct member_type_name
                {
                typedef unspecified type;
                
                typedef NDNBOOST_TTI_TP_MARKER_TYPE boost_tti_marker_type;
                };

              The metafunction types and return:
              
                NDNBOOST_TTI_TP_T           = the enclosing type.
                NDNBOOST_TTI_TP_MARKER_TYPE = (optional) a type to use as the marker type.
                                  defaults to the internal ndnboost::tti::detail::notype.
                
                returns         = 'type' is the inner type of 'name' if the inner type exists
                                  within the enclosing type, else 'type' is a marker type.
                                  if the end-user does not specify a marker type then
                                  an internal ndnboost::tti::detail::notype marker type is used.
                          
                The metafunction also encapsulates the type of the marker type as
                a nested 'boost_tti_marker_type'.
                          
    The purpose of this macro is to encapsulate the 'name' type as the typedef 'type'
    of a metafunction, but only if it exists within the enclosing type. This allows for
    an evaluation of inner type existence, without generating a compiler error,
    which can be used by other metafunctions in this library.
    
*/
#define NDNBOOST_TTI_MEMBER_TYPE(name) \
  NDNBOOST_TTI_TRAIT_MEMBER_TYPE \
  ( \
  NDNBOOST_TTI_MEMBER_TYPE_GEN(name), \
  name \
  ) \
/**/
  
namespace ndnboost
  {
  namespace tti
    {
  
    /// A metafunction which checks whether the member 'type' returned from invoking the macro metafunction generated by NDNBOOST_TTI_MEMBER_TYPE ( NDNBOOST_TTI_TRAIT_MEMBER_TYPE ) is a valid type.
    /**

        template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_MARKER_TYPE = ndnboost::tti::detail::notype>
        struct valid_member_type
          {
          static const value = unspecified;
          typedef mpl::bool_<true-or-false> type;
          };

        The metafunction types and return:

          NDNBOOST_TTI_TP_T           = returned inner 'type' from invoking the macro metafunction generated by NDNBOOST_TTI_MEMBER_TYPE ( NDNBOOST_TTI_TRAIT_MEMBER_TYPE ).
          NDNBOOST_TTI_TP_MARKER_TYPE = (optional) a type to use as the marker type.
                            defaults to the internal ndnboost::tti::detail::notype.
      
          returns         = 'value' is true if the type is valid, otherwise 'value' is false.
                            A valid type means that the returned inner 'type' is not the marker type.
                          
    */
    template<class NDNBOOST_TTI_TP_T,class NDNBOOST_TTI_TP_MARKER_TYPE = NDNBOOST_TTI_NAMESPACE::detail::notype>
    struct valid_member_type :
      ndnboost::mpl::not_
        <
        ndnboost::is_same<NDNBOOST_TTI_TP_T,NDNBOOST_TTI_TP_MARKER_TYPE>
        >
      {
      };
      
    /// A metafunction which checks whether the invoked macro metafunction generated by NDNBOOST_TTI_MEMBER_TYPE ( NDNBOOST_TTI_TRAIT_MEMBER_TYPE ) hold a valid type.
    /**

        template<class TTI_METAFUNCTION>
        struct valid_member_metafunction
          {
          static const value = unspecified;
          typedef mpl::bool_<true-or-false> type;
          };

        The metafunction types and return:

          TTI_METAFUNCTION = The invoked macro metafunction generated by NDNBOOST_TTI_MEMBER_TYPE ( NDNBOOST_TTI_TRAIT_MEMBER_TYPE ).
      
          returns = 'value' is true if the nested type of the invoked metafunction is valid, otherwise 'value' is false.
                    A valid type means that the invoked metafunction's inner 'type' is not the marker type.
                          
    */
    template<class TTI_METAFUNCTION>
    struct valid_member_metafunction :
      ndnboost::mpl::not_
        <
        ndnboost::is_same<typename TTI_METAFUNCTION::type,typename TTI_METAFUNCTION::boost_tti_marker_type>
        >
      {
      };
    }
  }
  
#endif // NDNBOOST_TTI_MEMBER_TYPE_HPP
