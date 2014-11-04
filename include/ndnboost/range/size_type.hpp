// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef NDNBOOST_RANGE_SIZE_TYPE_HPP
#define NDNBOOST_RANGE_SIZE_TYPE_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <ndnboost/range/config.hpp>
#include <ndnboost/range/difference_type.hpp>
#include <ndnboost/range/concepts.hpp>

#include <ndnboost/utility/enable_if.hpp>
#include <ndnboost/type_traits/make_unsigned.hpp>
#include <ndnboost/type_traits/remove_const.hpp>
#include <cstddef>
#include <utility>

namespace ndnboost
{
    namespace detail
    {

        //////////////////////////////////////////////////////////////////////////
        // default
        //////////////////////////////////////////////////////////////////////////

        template<typename T>
        class has_size_type
        {
            typedef char no_type;
            struct yes_type { char dummy[2]; };

            template<typename C>
            static yes_type test(NDNBOOST_DEDUCED_TYPENAME C::size_type x);

            template<typename C>
            static no_type test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
        };

        template<typename C, typename Enabler=void>
        struct range_size
        {
            typedef NDNBOOST_DEDUCED_TYPENAME make_unsigned<
                NDNBOOST_DEDUCED_TYPENAME range_difference<C>::type
            >::type type;
        };

        template<typename C>
        struct range_size<
            C,
            NDNBOOST_DEDUCED_TYPENAME ::ndnboost::enable_if<has_size_type<C>, void>::type
        >
        {
            typedef NDNBOOST_DEDUCED_TYPENAME C::size_type type;
        };

    }

    template< class T >
    struct range_size :
        detail::range_size<T>
    {
// Very strange things happen on some compilers that have the range concept
// asserts disabled. This preprocessor condition is clearly redundant on a
// working compiler but is vital for at least some compilers such as clang 4.2
// but only on the Mac!
#if NDNBOOST_RANGE_ENABLE_CONCEPT_ASSERT == 1
        NDNBOOST_RANGE_CONCEPT_ASSERT((ndnboost::SinglePassRangeConcept<T>));
#endif
    };

    template< class T >
    struct range_size<const T >
        : detail::range_size<T>
    {
#if NDNBOOST_RANGE_ENABLE_CONCEPT_ASSERT == 1        
        NDNBOOST_RANGE_CONCEPT_ASSERT((ndnboost::SinglePassRangeConcept<T>));
#endif
    };

} // namespace ndnboost



#endif
