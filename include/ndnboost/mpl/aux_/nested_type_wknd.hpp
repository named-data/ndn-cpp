
#ifndef NDNBOOST_MPL_AUX_NESTED_TYPE_WKND_HPP_INCLUDED
#define NDNBOOST_MPL_AUX_NESTED_TYPE_WKND_HPP_INCLUDED

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

#include <ndnboost/mpl/aux_/config/gcc.hpp>
#include <ndnboost/mpl/aux_/config/workaround.hpp>

#if NDNBOOST_WORKAROUND(NDNBOOST_MPL_CFG_GCC, NDNBOOST_TESTED_AT(0x0302)) \
    || NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x561)) \
    || NDNBOOST_WORKAROUND(__SUNPRO_CC, NDNBOOST_TESTED_AT(0x530)) \
    || NDNBOOST_WORKAROUND(__DMC__, NDNBOOST_TESTED_AT(0x840))

namespace ndnboost { namespace mpl { namespace aux {
template< typename T > struct nested_type_wknd
    : T::type
{
};
}}}

#if NDNBOOST_WORKAROUND(__DMC__, NDNBOOST_TESTED_AT(0x840))
#   define NDNBOOST_MPL_AUX_NESTED_TYPE_WKND(T) \
    aux::nested_type_wknd<T> \
/**/
#else
#   define NDNBOOST_MPL_AUX_NESTED_TYPE_WKND(T) \
    ::ndnboost::mpl::aux::nested_type_wknd<T> \
/**/
#endif

#else // !NDNBOOST_MPL_CFG_GCC et al.

#   define NDNBOOST_MPL_AUX_NESTED_TYPE_WKND(T) T::type

#endif 

#endif // NDNBOOST_MPL_AUX_NESTED_TYPE_WKND_HPP_INCLUDED
