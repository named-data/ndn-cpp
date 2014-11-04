
#ifndef NDNBOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED

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

#include <ndnboost/mpl/aux_/config/workaround.hpp>
#include <ndnboost/config.hpp>

#if    !defined(NDNBOOST_MPL_CFG_NO_NONTYPE_TEMPLATE_PARTIAL_SPEC) \
    && !defined(NDNBOOST_MPL_PREPROCESSING_MODE) \
    && NDNBOOST_WORKAROUND(__BORLANDC__, < 0x582)

#   define NDNBOOST_MPL_CFG_NO_NONTYPE_TEMPLATE_PARTIAL_SPEC

#endif

// NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION is defined in <ndnboost/config.hpp>

#endif // NDNBOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED
