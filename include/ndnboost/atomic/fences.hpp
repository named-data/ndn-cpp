/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2011 Helge Bahmann
 * Copyright (c) 2013 Tim Blechmann
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/fences.hpp
 *
 * This header contains definition of \c atomic_thread_fence and \c atomic_signal_fence functions.
 */

#ifndef NDNBOOST_ATOMIC_FENCES_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_FENCES_HPP_INCLUDED_

#include <ndnboost/memory_order.hpp>
#include <ndnboost/atomic/capabilities.hpp>
#include <ndnboost/atomic/detail/operations.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

/*
 * IMPLEMENTATION NOTE: All interface functions MUST be declared with NDNBOOST_FORCEINLINE,
 *                      see comment for convert_memory_order_to_gcc in ops_gcc_atomic.hpp.
 */

namespace ndnboost {

namespace atomics {

#if NDNBOOST_ATOMIC_THREAD_FENCE > 0
NDNBOOST_FORCEINLINE void atomic_thread_fence(memory_order order) NDNBOOST_NOEXCEPT
{
    detail::thread_fence(order);
}
#else
NDNBOOST_FORCEINLINE void atomic_thread_fence(memory_order) NDNBOOST_NOEXCEPT
{
    detail::lockpool::thread_fence();
}
#endif

#if NDNBOOST_ATOMIC_SIGNAL_FENCE > 0
NDNBOOST_FORCEINLINE void atomic_signal_fence(memory_order order) NDNBOOST_NOEXCEPT
{
    detail::signal_fence(order);
}
#else
NDNBOOST_FORCEINLINE void atomic_signal_fence(memory_order) NDNBOOST_NOEXCEPT
{
    detail::lockpool::signal_fence();
}
#endif

} // namespace atomics

using atomics::atomic_thread_fence;
using atomics::atomic_signal_fence;

} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_FENCES_HPP_INCLUDED_
