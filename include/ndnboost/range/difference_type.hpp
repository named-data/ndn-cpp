// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef NDNBOOST_RANGE_DIFFERENCE_TYPE_HPP
#define NDNBOOST_RANGE_DIFFERENCE_TYPE_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <ndnboost/range/config.hpp>
#include <ndnboost/range/iterator.hpp>
#include <ndnboost/iterator/iterator_traits.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>

namespace ndnboost
{
    template< class T >
    struct range_difference
            : iterator_difference<
                NDNBOOST_DEDUCED_TYPENAME range_iterator<
                    NDNBOOST_DEDUCED_TYPENAME remove_reference<T>::type
                >::type
            >
    { };
}

#endif
