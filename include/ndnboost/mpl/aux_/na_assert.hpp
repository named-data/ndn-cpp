
#ifndef NDNBOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <ndnboost/mpl/aux_/na.hpp>
#include <ndnboost/mpl/aux_/config/msvc.hpp>
#include <ndnboost/mpl/aux_/config/workaround.hpp>

#if !NDNBOOST_WORKAROUND(_MSC_FULL_VER, <= 140050601)    \
    && !NDNBOOST_WORKAROUND(__EDG_VERSION__, <= 243)
#   include <ndnboost/mpl/assert.hpp>
#   define NDNBOOST_MPL_AUX_ASSERT_NOT_NA(x) \
    NDNBOOST_MPL_ASSERT_NOT((ndnboost::mpl::is_na<type>)) \
/**/
#else
#   include <ndnboost/static_assert.hpp>
#   define NDNBOOST_MPL_AUX_ASSERT_NOT_NA(x) \
    NDNBOOST_STATIC_ASSERT(!ndnboost::mpl::is_na<x>::value) \
/**/
#endif

#endif // NDNBOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED
