
#ifndef NDNBOOST_MPL_LIST_AUX_PUSH_FRONT_HPP_INCLUDED
#define NDNBOOST_MPL_LIST_AUX_PUSH_FRONT_HPP_INCLUDED

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
#include <ndnboost/mpl/next.hpp>
#include <ndnboost/mpl/list/aux_/item.hpp>
#include <ndnboost/mpl/list/aux_/tag.hpp>

namespace ndnboost { namespace mpl {

template<>
struct push_front_impl< aux::list_tag >
{
    template< typename List, typename T > struct apply
    {
        typedef l_item<
              typename next<typename List::size>::type
            , T
            , typename List::type
            > type;
    };
};

}}

#endif // NDNBOOST_MPL_LIST_AUX_PUSH_FRONT_HPP_INCLUDED
