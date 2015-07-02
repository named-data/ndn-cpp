//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_MOVE_MOVE_HELPERS_HPP
#define NDNBOOST_MOVE_MOVE_HELPERS_HPP

#include <ndnboost/move/utility.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/move/utility.hpp>
#include <ndnboost/move/traits.hpp>

#if defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) || (defined(_MSC_VER) && (_MSC_VER == 1600))
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/is_class.hpp>
#include <ndnboost/type_traits/is_convertible.hpp>
#include <ndnboost/utility/enable_if.hpp>
#endif
#if defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES) 
#include <ndnboost/mpl/if.hpp>
#endif


#if defined(NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
struct not_a_type;
struct not_a_type2;
#define NDNBOOST_MOVE_CATCH_CONST(U)  \
   typename ::ndnboost::mpl::if_< ::ndnboost::is_class<U>, NDNBOOST_CATCH_CONST_RLVALUE(U), const U &>::type
#define NDNBOOST_MOVE_CATCH_RVALUE(U)\
   typename ::ndnboost::mpl::if_< ::ndnboost::is_class<U>, NDNBOOST_RV_REF(U), not_a_type>::type
#define NDNBOOST_MOVE_CATCH_FWD(U) NDNBOOST_FWD_REF(U)
#else
#define NDNBOOST_MOVE_CATCH_CONST(U)  const U &
#define NDNBOOST_MOVE_CATCH_RVALUE(U) U &&
#define NDNBOOST_MOVE_CATCH_FWD(U)    U &&
#endif

#ifdef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(::ndnboost::move(x));  }\
\
   RETURN_VALUE PUB_FUNCTION(TYPE &x)\
   {  return FWD_FUNCTION(const_cast<const TYPE &>(x)); }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c\
                     <  ::ndnboost::is_class<TYPE>::value &&\
                        ::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value &&\
                       !::ndnboost::has_move_emulation_enabled<NDNBOOST_MOVE_TEMPL_PARAM>::value\
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   { return FWD_FUNCTION(u); }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c\
                     < (!::ndnboost::is_class<NDNBOOST_MOVE_TEMPL_PARAM>::value || \
                        !::ndnboost::move_detail::is_rv<NDNBOOST_MOVE_TEMPL_PARAM>::value) && \
                       !::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value \
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   {\
      TYPE t(u);\
      return FWD_FUNCTION(::ndnboost::move(t));\
   }\
//
//                         ::ndnboost::is_convertible<NDNBOOST_MOVE_TEMPL_PARAM, TYPE>::value &&
#elif (defined(_MSC_VER) && (_MSC_VER == 1600))

#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(::ndnboost::move(x));  }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c\
                     <  !::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value\
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   {\
      TYPE t(u);\
      return FWD_FUNCTION(::ndnboost::move(t));\
   }\
//

#else

#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(::ndnboost::move(x));  }\
//

#endif


#ifdef NDNBOOST_NO_CXX11_RVALUE_REFERENCES

#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(arg1, ::ndnboost::move(x));  }\
\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, TYPE &x)\
   {  return FWD_FUNCTION(arg1, const_cast<const TYPE &>(x)); }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c<\
                        ::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value &&\
                       !::ndnboost::has_move_emulation_enabled<NDNBOOST_MOVE_TEMPL_PARAM>::value\
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(ARG1 arg1, const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   { return FWD_FUNCTION(arg1, u); }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c<\
                       !::ndnboost::move_detail::is_rv<NDNBOOST_MOVE_TEMPL_PARAM>::value && \
                       !::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value && \
                       !::ndnboost::is_convertible<NDNBOOST_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO>::value \
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(ARG1 arg1, const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   {\
      TYPE t(u);\
      return FWD_FUNCTION(arg1, ::ndnboost::move(t));\
   }\
//

#elif (defined(_MSC_VER) && (_MSC_VER == 1600))

#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(arg1, ::ndnboost::move(x));  }\
\
   template<class NDNBOOST_MOVE_TEMPL_PARAM>\
   typename ::ndnboost::enable_if_c\
                     <  !::ndnboost::is_same<TYPE, NDNBOOST_MOVE_TEMPL_PARAM>::value && \
                       !::ndnboost::is_convertible<NDNBOOST_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO>::value \
                     , RETURN_VALUE >::type\
   PUB_FUNCTION(ARG1 arg1, const NDNBOOST_MOVE_TEMPL_PARAM &u)\
   {\
      TYPE t(u);\
      return FWD_FUNCTION(arg1, ::ndnboost::move(t));\
   }\
//

#else

#define NDNBOOST_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_CONST(TYPE) x)\
   {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
\
   RETURN_VALUE PUB_FUNCTION(ARG1 arg1, NDNBOOST_MOVE_CATCH_RVALUE(TYPE) x) \
   {  return FWD_FUNCTION(arg1, ::ndnboost::move(x));  }\
//

#endif

#endif //#ifndef NDNBOOST_MOVE_MOVE_HELPERS_HPP
