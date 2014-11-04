
#ifndef NDNBOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED

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

#include <ndnboost/mpl/aux_/config/workaround.hpp>

#if    !defined(NDNBOOST_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION) \
    && !defined(NDNBOOST_MPL_PREPROCESSING_MODE) \
    && (   NDNBOOST_WORKAROUND(__BORLANDC__, < 0x590) \
        || NDNBOOST_WORKAROUND(__MWERKS__, < 0x3001) \
        )

#   define NDNBOOST_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION

#endif

#endif // NDNBOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
