//  static_assert.hpp  --------------------------------------------------------------//

//  Copyright 2009-2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef NDNBOOST_CHRONO_DETAIL_STATIC_ASSERT_HPP
#define NDNBOOST_CHRONO_DETAIL_STATIC_ASSERT_HPP

#include <ndnboost/chrono/config.hpp>

#ifndef NDNBOOST_NO_CXX11_STATIC_ASSERT
#define NDNBOOST_CHRONO_STATIC_ASSERT(CND, MSG, TYPES) static_assert(CND,MSG)
#elif defined(NDNBOOST_CHRONO_USES_STATIC_ASSERT)
#include <ndnboost/static_assert.hpp>
#define NDNBOOST_CHRONO_STATIC_ASSERT(CND, MSG, TYPES) NDNBOOST_STATIC_ASSERT(CND)
#elif defined(NDNBOOST_CHRONO_USES_MPL_ASSERT)
#include <ndnboost/mpl/assert.hpp>
#include <ndnboost/mpl/bool.hpp>
#define NDNBOOST_CHRONO_STATIC_ASSERT(CND, MSG, TYPES)                                 \
    NDNBOOST_MPL_ASSERT_MSG(ndnboost::mpl::bool_< (CND) >::type::value, MSG, TYPES)
#else
//~ #elif defined(NDNBOOST_CHRONO_USES_ARRAY_ASSERT)
#define NDNBOOST_CHRONO_STATIC_ASSERT(CND, MSG, TYPES) static char NDNBOOST_JOIN(boost_chrono_test_,__LINE__)[(CND)?1:-1]
//~ #define NDNBOOST_CHRONO_STATIC_ASSERT(CND, MSG, TYPES)
#endif

#endif // NDNBOOST_CHRONO_DETAIL_STATIC_ASSERT_HPP
