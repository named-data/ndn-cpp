/*
 Copyright (c) 2014 Glen Joseph Fernandes
 glenfe at live dot com

 Distributed under the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt
 or copy at http://boost.org/LICENSE_1_0.txt)
*/
#ifndef NDNBOOST_ALIGN_DETAIL_ALIGN_HPP
#define NDNBOOST_ALIGN_DETAIL_ALIGN_HPP

#include <ndnboost/assert.hpp>
#include <ndnboost/align/detail/address.hpp>
#include <ndnboost/align/detail/is_alignment.hpp>
#include <cstddef>

namespace ndnboost {
    namespace alignment {
        inline void* align(std::size_t alignment, std::size_t size,
            void*& ptr, std::size_t& space)
        {
            NDNBOOST_ASSERT(detail::is_alignment(alignment));
            std::size_t n = detail::address_t(ptr) & (alignment - 1);
            if (n != 0) {
                n = alignment - n;
            }
            void* p = 0;
            if (n <= space && size <= space - n) {
                p = static_cast<char*>(ptr) + n;
                ptr = p;
                space -= n;
            }
            return p;
        }
    }
}

#endif
