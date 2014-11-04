/*
 Copyright (c) 2014 Glen Joseph Fernandes
 glenfe at live dot com

 Distributed under the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt
 or copy at http://boost.org/LICENSE_1_0.txt)
*/
#ifndef NDNBOOST_ALIGN_DETAIL_ADDRESS_HPP
#define NDNBOOST_ALIGN_DETAIL_ADDRESS_HPP

#include <ndnboost/cstdint.hpp>
#include <cstddef>

namespace ndnboost {
    namespace alignment {
        namespace detail {
#if defined(NDNBOOST_HAS_INTPTR_T)
            typedef ndnboost::uintptr_t address_t;
#else
            typedef std::size_t address_t;
#endif
        }
    }
}

#endif
