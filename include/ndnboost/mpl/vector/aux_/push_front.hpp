
#ifndef NDNBOOST_MPL_VECTOR_AUX_PUSH_FRONT_HPP_INCLUDED
#define NDNBOOST_MPL_VECTOR_AUX_PUSH_FRONT_HPP_INCLUDED

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

#include <ndnboost/mpl/push_front_fwd.hpp>
#include <ndnboost/mpl/aux_/config/typeof.hpp>

#if defined(NDNBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES)

#   include <ndnboost/mpl/vector/aux_/item.hpp>
#   include <ndnboost/mpl/vector/aux_/tag.hpp>

namespace ndnboost { namespace mpl {

template<>
struct push_front_impl< aux::vector_tag >
{
    template< typename Vector, typename T > struct apply
    {
        typedef v_item<T,Vector,1> type;
    };
};

}}

#endif // NDNBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

#endif // NDNBOOST_MPL_VECTOR_AUX_PUSH_FRONT_HPP_INCLUDED
