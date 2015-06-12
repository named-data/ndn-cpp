
#ifndef NDNBOOST_MPL_AUX_FRONT_IMPL_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_FRONT_IMPL_HPP_INCLUDED

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

#include <ndnboost/mpl/front_fwd.hpp>
#include <ndnboost/mpl/begin_end.hpp>
#include <ndnboost/mpl/deref.hpp>
#include <ndnboost/mpl/aux_/traits_lambda_spec.hpp>

namespace ndnboost { namespace mpl {

// default implementation; conrete sequences might override it by 
// specializing either the 'front_impl' or the primary 'front' template

template< typename Tag >
struct front_impl
{
    template< typename Sequence > struct apply
    {
        typedef typename begin<Sequence>::type iter_;
        typedef typename deref<iter_>::type type;
    };
};

NDNBOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(1,front_impl)

}}

#endif // NDNBOOST_MPL_AUX_FRONT_IMPL_HPP_INCLUDED
