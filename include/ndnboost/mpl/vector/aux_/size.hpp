
#ifndef NDNBOOST_MPL_VECTOR_AUX_SIZE_HPP_INCLUDED
#define NDNBOOST_MPL_VECTOR_AUX_SIZE_HPP_INCLUDED

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

#include <ndnboost/mpl/size_fwd.hpp>
#include <ndnboost/mpl/vector/aux_/O1_size.hpp>
#include <ndnboost/mpl/vector/aux_/tag.hpp>
#include <ndnboost/mpl/aux_/config/typeof.hpp>
#include <ndnboost/mpl/aux_/config/ctps.hpp>

namespace ndnboost { namespace mpl {

#if defined(NDNBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES)

template<>
struct size_impl< aux::vector_tag >
    : O1_size_impl< aux::vector_tag >
{
};

#else

#if !defined(NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< long N >
struct size_impl< aux::vector_tag<N> >
    : O1_size_impl< aux::vector_tag<N> >
{
};

#endif // NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#endif // NDNBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

}}

#endif // NDNBOOST_MPL_VECTOR_AUX_SIZE_HPP_INCLUDED
