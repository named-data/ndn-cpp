
#ifndef NDNBOOST_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <ndnboost/mpl/aux_/config/gcc.hpp>
#include <ndnboost/mpl/aux_/config/workaround.hpp>

// GCC and EDG-based compilers incorrectly reject the following code:
//   template< typename T, T n > struct a;
//   template< typename T > struct b;
//   template< typename T, T n > struct b< a<T,n> > {};

#if    !defined(NDNBOOST_MPL_CFG_NO_DEPENDENT_NONTYPE_PARAMETER_IN_PARTIAL_SPEC) \
    && !defined(NDNBOOST_MPL_PREPROCESSING_MODE) \
    && (   NDNBOOST_WORKAROUND(__EDG_VERSION__, NDNBOOST_TESTED_AT(300)) \
        || NDNBOOST_WORKAROUND(NDNBOOST_MPL_CFG_GCC, NDNBOOST_TESTED_AT(0x0302)) \
        )

#   define NDNBOOST_MPL_CFG_NO_DEPENDENT_NONTYPE_PARAMETER_IN_PARTIAL_SPEC

#endif

#endif // NDNBOOST_MPL_AUX_CONFIG_DEPENDENT_NTTP_HPP_INCLUDED
