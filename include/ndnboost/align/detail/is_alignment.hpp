/*
 Copyright (c) 2014 Glen Joseph Fernandes
 glenfe at live dot com

 Distributed under the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt
 or copy at http://boost.org/LICENSE_1_0.txt)
*/
#ifndef NDNBOOST_ALIGN_DETAIL_IS_ALIGNMENT_HPP
#define NDNBOOST_ALIGN_DETAIL_IS_ALIGNMENT_HPP

#include <ndnboost/config.hpp>
#include <cstddef>

namespace ndnboost {
    namespace alignment {
        namespace detail {
            NDNBOOST_CONSTEXPR inline bool is_alignment(std::size_t
                value) NDNBOOST_NOEXCEPT
            {
                return (value > 0) && ((value & (value - 1)) == 0);
            }
        }
    }
}

#endif
