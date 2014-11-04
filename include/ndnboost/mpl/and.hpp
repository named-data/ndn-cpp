
#ifndef NDNBOOST_MPL_AND_HPP_INCLUDED
#define NDNBOOST_MPL_AND_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <ndnboost/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(NDNBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(NDNBOOST_MPL_PREPROCESSING_MODE)

#   include <ndnboost/mpl/bool.hpp>
#   include <ndnboost/mpl/aux_/nested_type_wknd.hpp>
#   include <ndnboost/mpl/aux_/na_spec.hpp>
#   include <ndnboost/mpl/aux_/lambda_support.hpp>

// agurt, 19/may/04: workaround a conflict with <iso646.h> header's 
// 'or' and 'and' macros, see http://tinyurl.com/3et69; 'defined(and)'
// has to be checked in a separate condition, otherwise GCC complains 
// about 'and' being an alternative token
#if defined(_MSC_VER) && !defined(__clang__)
#ifndef __GCCXML__
#if defined(and) 
#   pragma push_macro("and")
#   undef and
#   define and(x)
#endif
#endif
#endif

#   define NDNBOOST_MPL_PREPROCESSED_HEADER and.hpp
#   include <ndnboost/mpl/aux_/include_preprocessed.hpp>

#if defined(_MSC_VER) && !defined(__clang__)
#ifndef __GCCXML__
#if defined(and) 
#   pragma pop_macro("and")
#endif
#endif
#endif

#else

#   define AUX778076_OP_NAME and_
#   define AUX778076_OP_VALUE1 false
#   define AUX778076_OP_VALUE2 true
#   include <ndnboost/mpl/aux_/logical_op.hpp>

#endif // NDNBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // NDNBOOST_MPL_AND_HPP_INCLUDED
