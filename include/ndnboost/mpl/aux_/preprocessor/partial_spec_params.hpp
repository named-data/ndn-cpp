
#ifndef NDNBOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED

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

#include <ndnboost/mpl/limits/arity.hpp>
#include <ndnboost/mpl/aux_/preprocessor/params.hpp>
#include <ndnboost/mpl/aux_/preprocessor/enum.hpp>
#include <ndnboost/mpl/aux_/preprocessor/sub.hpp>
#include <ndnboost/preprocessor/comma_if.hpp>

#define NDNBOOST_MPL_PP_PARTIAL_SPEC_PARAMS(n, param, def) \
NDNBOOST_MPL_PP_PARAMS(n, param) \
NDNBOOST_PP_COMMA_IF(NDNBOOST_MPL_PP_SUB(NDNBOOST_MPL_LIMIT_METAFUNCTION_ARITY,n)) \
NDNBOOST_MPL_PP_ENUM( \
      NDNBOOST_MPL_PP_SUB(NDNBOOST_MPL_LIMIT_METAFUNCTION_ARITY,n) \
    , def \
    ) \
/**/

#endif // NDNBOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
