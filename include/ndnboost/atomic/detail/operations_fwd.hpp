/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/operations_fwd.hpp
 *
 * This header contains forward declaration of the \c operations template.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_OPERATIONS_FWD_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_OPERATIONS_FWD_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace ndnboost {
namespace atomics {
namespace detail {

template< unsigned int Size, bool Signed >
struct operations;

} // namespace detail
} // namespace atomics
} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_DETAIL_OPERATIONS_FWD_HPP_INCLUDED_
