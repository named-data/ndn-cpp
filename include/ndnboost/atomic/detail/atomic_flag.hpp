/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/atomic_flag.hpp
 *
 * This header contains interface definition of \c atomic_flag.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_ATOMIC_FLAG_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_ATOMIC_FLAG_HPP_INCLUDED_

#include <ndnboost/assert.hpp>
#include <ndnboost/memory_order.hpp>
#include <ndnboost/atomic/detail/config.hpp>
#include <ndnboost/atomic/detail/operations_lockfree.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

/*
 * IMPLEMENTATION NOTE: All interface functions MUST be declared with NDNBOOST_FORCEINLINE,
 *                      see comment for convert_memory_order_to_gcc in ops_gcc_atomic.hpp.
 */

namespace ndnboost {
namespace atomics {

#if defined(NDNBOOST_NO_CXX11_CONSTEXPR) || defined(NDNBOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)
#define NDNBOOST_ATOMIC_NO_ATOMIC_FLAG_INIT
#else
#define NDNBOOST_ATOMIC_FLAG_INIT {}
#endif

struct atomic_flag
{
    typedef atomics::detail::operations< 1u, false > operations;
    typedef operations::storage_type storage_type;

    storage_type m_storage;

    NDNBOOST_FORCEINLINE NDNBOOST_CONSTEXPR atomic_flag() NDNBOOST_NOEXCEPT : m_storage(0)
    {
    }

    NDNBOOST_FORCEINLINE bool test_and_set(memory_order order = memory_order_seq_cst) volatile NDNBOOST_NOEXCEPT
    {
        return operations::test_and_set(m_storage, order);
    }

    NDNBOOST_FORCEINLINE void clear(memory_order order = memory_order_seq_cst) volatile NDNBOOST_NOEXCEPT
    {
        NDNBOOST_ASSERT(order != memory_order_acquire);
        NDNBOOST_ASSERT(order != memory_order_acq_rel);
        operations::clear(m_storage, order);
    }

    NDNBOOST_DELETED_FUNCTION(atomic_flag(atomic_flag const&))
    NDNBOOST_DELETED_FUNCTION(atomic_flag& operator= (atomic_flag const&))
};

} // namespace atomics
} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_DETAIL_ATOMIC_FLAG_HPP_INCLUDED_
