// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef NDNBOOST_RANGE_SIZE_HPP
#define NDNBOOST_RANGE_SIZE_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <ndnboost/range/config.hpp>
#include <ndnboost/range/begin.hpp>
#include <ndnboost/range/end.hpp>
#include <ndnboost/range/size_type.hpp>
#include <ndnboost/range/detail/has_member_size.hpp>
#include <ndnboost/assert.hpp>
#include <ndnboost/cstdint.hpp>
#include <ndnboost/utility.hpp>

namespace ndnboost
{
    namespace range_detail
    {

        template<class SinglePassRange>
        inline typename ::ndnboost::enable_if<
            has_member_size<SinglePassRange>,
            typename range_size<const SinglePassRange>::type
        >::type
        range_calculate_size(const SinglePassRange& rng)
        {
            return rng.size();
        }

        template<class SinglePassRange>
        inline typename disable_if<
            has_member_size<SinglePassRange>,
            typename range_size<const SinglePassRange>::type
        >::type
        range_calculate_size(const SinglePassRange& rng)
        {
            return std::distance(ndnboost::begin(rng), ndnboost::end(rng));
        }
    }

    template<class SinglePassRange>
    inline typename range_size<const SinglePassRange>::type
    size(const SinglePassRange& rng)
    {
#if !NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x564)) && \
    !NDNBOOST_WORKAROUND(__GNUC__, < 3) \
    /**/
        using namespace range_detail;
#endif
        return range_calculate_size(rng);
    }

} // namespace 'boost'

#endif
