// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef NDNBOOST_RANGE_DETAIL_BEGIN_HPP
#define NDNBOOST_RANGE_DETAIL_BEGIN_HPP

#include <ndnboost/config.hpp> // NDNBOOST_MSVC
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/range/iterator.hpp>
#include <ndnboost/range/detail/common.hpp>

namespace ndnboost
{

    namespace range_detail
    {
        template< typename T >
        struct range_begin;

        //////////////////////////////////////////////////////////////////////
        // default
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_begin<std_container_>
        {
            template< typename C >
            static NDNBOOST_RANGE_DEDUCED_TYPENAME range_iterator<C>::type fun( C& c )
            {
                return c.begin();
            };
        };

        //////////////////////////////////////////////////////////////////////
        // pair
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_begin<std_pair_>
        {
            template< typename P >
            static NDNBOOST_RANGE_DEDUCED_TYPENAME range_iterator<P>::type fun( const P& p )
            {
                return p.first;
            }
        };

        //////////////////////////////////////////////////////////////////////
        // array
        //////////////////////////////////////////////////////////////////////

        template<>
        struct range_begin<array_>
        {
            template<typename T>
            static NDNBOOST_RANGE_DEDUCED_TYPENAME range_value<T>::type* fun(T& t)
            {
                return t;
            }
        };

    } // namespace 'range_detail'

    namespace range_adl_barrier
    {
        template< typename C >
        inline NDNBOOST_RANGE_DEDUCED_TYPENAME range_iterator<C>::type
        begin( C& c )
        {
            return range_detail::range_begin< NDNBOOST_RANGE_DEDUCED_TYPENAME range_detail::range<C>::type >::fun( c );
        }
    }
} // namespace 'boost'


#endif
