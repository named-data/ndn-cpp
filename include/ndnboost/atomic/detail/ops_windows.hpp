/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2009 Helge Bahmann
 * Copyright (c) 2012 Tim Blechmann
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/ops_windows.hpp
 *
 * This header contains implementation of the \c operations template.
 *
 * This implementation is the most basic version for Windows. It should
 * work for any non-MSVC-like compilers as long as there are Interlocked WinAPI
 * functions available. This version is also used for WinCE.
 *
 * Notably, this implementation is not as efficient as other
 * versions based on compiler intrinsics.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_OPS_WINDOWS_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_OPS_WINDOWS_HPP_INCLUDED_

#include <ndnboost/memory_order.hpp>
#include <ndnboost/type_traits/make_signed.hpp>
#include <ndnboost/atomic/detail/config.hpp>
#include <ndnboost/atomic/detail/interlocked.hpp>
#include <ndnboost/atomic/detail/storage_type.hpp>
#include <ndnboost/atomic/detail/operations_fwd.hpp>
#include <ndnboost/atomic/capabilities.hpp>
#include <ndnboost/atomic/detail/ops_msvc_common.hpp>
#include <ndnboost/atomic/detail/ops_extending_cas_based.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace ndnboost {
namespace atomics {
namespace detail {

struct windows_operations_base
{
    static NDNBOOST_FORCEINLINE void hardware_full_fence() NDNBOOST_NOEXCEPT
    {
        long tmp;
        NDNBOOST_ATOMIC_INTERLOCKED_EXCHANGE(&tmp, 0);
    }

    static NDNBOOST_FORCEINLINE void fence_before(memory_order) NDNBOOST_NOEXCEPT
    {
        NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER();
    }

    static NDNBOOST_FORCEINLINE void fence_after(memory_order) NDNBOOST_NOEXCEPT
    {
        NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER();
    }
};

template< typename T, typename Derived >
struct windows_operations :
    public windows_operations_base
{
    typedef T storage_type;

    static NDNBOOST_FORCEINLINE void store(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
        Derived::exchange(storage, v, order);
    }

    static NDNBOOST_FORCEINLINE storage_type load(storage_type const volatile& storage, memory_order order) NDNBOOST_NOEXCEPT
    {
        return Derived::fetch_add(const_cast< storage_type volatile& >(storage), (storage_type)0, order);
    }

    static NDNBOOST_FORCEINLINE storage_type fetch_sub(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
        typedef typename make_signed< storage_type >::type signed_storage_type;
        return Derived::fetch_add(storage, static_cast< storage_type >(-static_cast< signed_storage_type >(v)), order);
    }

    static NDNBOOST_FORCEINLINE bool compare_exchange_weak(
        storage_type volatile& storage, storage_type& expected, storage_type desired, memory_order success_order, memory_order failure_order) NDNBOOST_NOEXCEPT
    {
        return Derived::compare_exchange_strong(storage, expected, desired, success_order, failure_order);
    }

    static NDNBOOST_FORCEINLINE bool test_and_set(storage_type volatile& storage, memory_order order) NDNBOOST_NOEXCEPT
    {
        return !!Derived::exchange(storage, (storage_type)1, order);
    }

    static NDNBOOST_FORCEINLINE void clear(storage_type volatile& storage, memory_order order) NDNBOOST_NOEXCEPT
    {
        store(storage, (storage_type)0, order);
    }

    static NDNBOOST_FORCEINLINE bool is_lock_free(storage_type const volatile&) NDNBOOST_NOEXCEPT
    {
        return true;
    }
};

template< bool Signed >
struct operations< 4u, Signed > :
    public windows_operations< typename make_storage_type< 4u, Signed >::type, operations< 4u, Signed > >
{
    typedef windows_operations< typename make_storage_type< 4u, Signed >::type, operations< 4u, Signed > > base_type;
    typedef typename base_type::storage_type storage_type;

    static NDNBOOST_FORCEINLINE storage_type fetch_add(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
        base_type::fence_before(order);
        v = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_EXCHANGE_ADD(&storage, v));
        base_type::fence_after(order);
        return v;
    }

    static NDNBOOST_FORCEINLINE storage_type exchange(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
        base_type::fence_before(order);
        v = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_EXCHANGE(&storage, v));
        base_type::fence_after(order);
        return v;
    }

    static NDNBOOST_FORCEINLINE bool compare_exchange_strong(
        storage_type volatile& storage, storage_type& expected, storage_type desired, memory_order success_order, memory_order failure_order) NDNBOOST_NOEXCEPT
    {
        storage_type previous = expected;
        base_type::fence_before(success_order);
        storage_type old_val = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_COMPARE_EXCHANGE(&storage, desired, previous));
        expected = old_val;
        // The success and failure fences are the same anyway
        base_type::fence_after(success_order);
        return (previous == old_val);
    }

    static NDNBOOST_FORCEINLINE storage_type fetch_and(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
#if defined(NDNBOOST_ATOMIC_INTERLOCKED_AND)
        base_type::fence_before(order);
        v = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_AND(&storage, v));
        base_type::fence_after(order);
        return v;
#else
        storage_type res = storage;
        while (!compare_exchange_strong(storage, res, res & v, order, memory_order_relaxed)) {}
        return res;
#endif
    }

    static NDNBOOST_FORCEINLINE storage_type fetch_or(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
#if defined(NDNBOOST_ATOMIC_INTERLOCKED_OR)
        base_type::fence_before(order);
        v = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_OR(&storage, v));
        base_type::fence_after(order);
        return v;
#else
        storage_type res = storage;
        while (!compare_exchange_strong(storage, res, res | v, order, memory_order_relaxed)) {}
        return res;
#endif
    }

    static NDNBOOST_FORCEINLINE storage_type fetch_xor(storage_type volatile& storage, storage_type v, memory_order order) NDNBOOST_NOEXCEPT
    {
#if defined(NDNBOOST_ATOMIC_INTERLOCKED_XOR)
        base_type::fence_before(order);
        v = static_cast< storage_type >(NDNBOOST_ATOMIC_INTERLOCKED_XOR(&storage, v));
        base_type::fence_after(order);
        return v;
#else
        storage_type res = storage;
        while (!compare_exchange_strong(storage, res, res ^ v, order, memory_order_relaxed)) {}
        return res;
#endif
    }
};

template< bool Signed >
struct operations< 1u, Signed > :
    public extending_cas_based_operations< operations< 4u, Signed >, 1u, Signed >
{
};

template< bool Signed >
struct operations< 2u, Signed > :
    public extending_cas_based_operations< operations< 4u, Signed >, 2u, Signed >
{
};

NDNBOOST_FORCEINLINE void thread_fence(memory_order order) NDNBOOST_NOEXCEPT
{
    NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER();
    if (order == memory_order_seq_cst)
        windows_operations_base::hardware_full_fence();
    NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER();
}

NDNBOOST_FORCEINLINE void signal_fence(memory_order order) NDNBOOST_NOEXCEPT
{
    if (order != memory_order_relaxed)
        NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER();
}

} // namespace detail
} // namespace atomics
} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_DETAIL_OPS_WINDOWS_HPP_INCLUDED_
