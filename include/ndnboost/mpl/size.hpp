
#ifndef NDNBOOST_MPL_SIZE_HPP_INCLUDED
#define NDNBOOST_MPL_SIZE_HPP_INCLUDED

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
#include <ndnboost/mpl/sequence_tag.hpp>
#include <ndnboost/mpl/aux_/size_impl.hpp>
#include <ndnboost/mpl/aux_/na_spec.hpp>
#include <ndnboost/mpl/aux_/lambda_support.hpp>
#include <ndnboost/mpl/aux_/msvc_eti_base.hpp>

namespace ndnboost { namespace mpl {

template<
      typename NDNBOOST_MPL_AUX_NA_PARAM(Sequence)
    >
struct size
    : aux::msvc_eti_base<
        typename size_impl< typename sequence_tag<Sequence>::type >
            ::template apply< Sequence >::type
      >::type
{
    NDNBOOST_MPL_AUX_LAMBDA_SUPPORT(1, size, (Sequence))
};

NDNBOOST_MPL_AUX_NA_SPEC(1, size)

}}

#endif // NDNBOOST_MPL_SIZE_HPP_INCLUDED
