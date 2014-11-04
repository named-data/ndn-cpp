
#ifndef NDNBOOST_MPL_AUX_NA_FWD_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_NA_FWD_HPP_INCLUDED

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

#include <ndnboost/mpl/aux_/adl_barrier.hpp>

NDNBOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN

// n.a. == not available
struct na
{
    typedef na type;
    enum { value = 0 };
};

NDNBOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE
NDNBOOST_MPL_AUX_ADL_BARRIER_DECL(na)

#endif // NDNBOOST_MPL_AUX_NA_FWD_HPP_INCLUDED
