// Boost.Range library
//
//  Copyright Neil Groves 2010. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
// Acknowledgments:
// Ticket #8341: Arno Schoedl - improved handling of has_range_iterator upon
// use-cases where T was const.
#ifndef NDNBOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED
#define NDNBOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED

#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/eval_if.hpp>
#include <ndnboost/mpl/has_xxx.hpp>
#include <ndnboost/range/iterator.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>
#include <ndnboost/utility/enable_if.hpp>

namespace ndnboost
{
    namespace range_detail
    {
        NDNBOOST_MPL_HAS_XXX_TRAIT_DEF(type)

        template<class T, class Enabler = void>
        struct has_range_iterator_impl
            : ndnboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_iterator_impl<
            T,
            NDNBOOST_DEDUCED_TYPENAME ::ndnboost::enable_if<
                NDNBOOST_DEDUCED_TYPENAME mpl::eval_if<is_const<T>,
                    has_type<range_const_iterator<
                                NDNBOOST_DEDUCED_TYPENAME remove_const<T>::type> >,
                    has_type<range_mutable_iterator<T> >
                >::type
            >::type
        >
            : ndnboost::mpl::true_
        {
        };

        template<class T, class Enabler = void>
        struct has_range_const_iterator_impl
            : ndnboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_const_iterator_impl<
            T,
            NDNBOOST_DEDUCED_TYPENAME ::ndnboost::enable_if<
                has_type<range_const_iterator<T> >
            >::type
        >
            : ndnboost::mpl::true_
        {
        };

    } // namespace range_detail

    template<class T>
    struct has_range_iterator
        : range_detail::has_range_iterator_impl<
            NDNBOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};

    template<class T>
    struct has_range_const_iterator
        : range_detail::has_range_const_iterator_impl<
            NDNBOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};
} // namespace ndnboost

#endif // include guard

