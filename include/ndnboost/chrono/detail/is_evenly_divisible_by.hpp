//  is_evenly_divisible_by.hpp  --------------------------------------------------------------//

//  Copyright 2009-2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef NDNBOOST_CHRONO_DETAIL_IS_EVENLY_DIVISIBLE_BY_HPP
#define NDNBOOST_CHRONO_DETAIL_IS_EVENLY_DIVISIBLE_BY_HPP

#include <ndnboost/chrono/config.hpp>

#include <ndnboost/mpl/logical.hpp>
#include <ndnboost/ratio/detail/overflow_helpers.hpp>

namespace ndnboost {
namespace chrono {
namespace chrono_detail {

//  template <class R1, class R2>
//  struct is_evenly_divisible_by : public ndnboost::mpl::bool_ < ratio_divide<R1, R2>::type::den == 1 >
//  {};
  template <class R1, class R2>
  struct is_evenly_divisible_by : public ndnboost::ratio_detail::is_evenly_divisible_by<R1, R2>
  {};

} // namespace chrono_detail
} // namespace detail
} // namespace chrono

#endif // NDNBOOST_CHRONO_DETAIL_IS_EVENLY_DIVISIBLE_BY_HPP
