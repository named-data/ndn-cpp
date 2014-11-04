
#ifndef NDNBOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED
#define NDNBOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED

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

#include <ndnboost/mpl/arg_fwd.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/aux_/yes_no.hpp>
#include <ndnboost/mpl/aux_/type_wrapper.hpp>
#include <ndnboost/mpl/aux_/nttp_decl.hpp>
#include <ndnboost/mpl/aux_/config/ctps.hpp>
#include <ndnboost/mpl/aux_/config/static_constant.hpp>

namespace ndnboost { namespace mpl {

#if !defined(NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template< typename T >
struct is_placeholder
    : bool_<false>
{
};

template< NDNBOOST_MPL_AUX_NTTP_DECL(int, N) >
struct is_placeholder< arg<N> >
    : bool_<true>
{
};

#else

namespace aux {

aux::no_tag is_placeholder_helper(...);

template< NDNBOOST_MPL_AUX_NTTP_DECL(int, N) >
aux::yes_tag is_placeholder_helper(aux::type_wrapper< arg<N> >*);

} // namespace aux

template< typename T >
struct is_placeholder
{
    static aux::type_wrapper<T>* get();
    NDNBOOST_STATIC_CONSTANT(bool, value = 
          sizeof(aux::is_placeholder_helper(get())) == sizeof(aux::yes_tag)
        );
    
    typedef bool_<value> type;
};

#endif // NDNBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

}}

#endif // NDNBOOST_MPL_IS_PLACEHOLDER_HPP_INCLUDED
