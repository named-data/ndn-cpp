
#ifndef NDNBOOST_MPL_ITER_FOLD_HPP_INCLUDED
#define NDNBOOST_MPL_ITER_FOLD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
// Copyright David Abrahams 2001-2002
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <ndnboost/mpl/begin_end.hpp>
#include <ndnboost/mpl/O1_size.hpp>
#include <ndnboost/mpl/lambda.hpp>
#include <ndnboost/mpl/aux_/iter_fold_impl.hpp>
#include <ndnboost/mpl/aux_/na_spec.hpp>
#include <ndnboost/mpl/aux_/lambda_support.hpp>

namespace ndnboost { namespace mpl {

template<
      typename NDNBOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename NDNBOOST_MPL_AUX_NA_PARAM(State)
    , typename NDNBOOST_MPL_AUX_NA_PARAM(ForwardOp)
    >
struct iter_fold
{
    typedef typename aux::iter_fold_impl<
          ::ndnboost::mpl::O1_size<Sequence>::value
        , typename begin<Sequence>::type
        , typename end<Sequence>::type
        , State
        , typename lambda<ForwardOp>::type
        >::state type;

    NDNBOOST_MPL_AUX_LAMBDA_SUPPORT(3,iter_fold,(Sequence,State,ForwardOp))
};

NDNBOOST_MPL_AUX_NA_SPEC(3, iter_fold)

}}

#endif // NDNBOOST_MPL_ITER_FOLD_HPP_INCLUDED
