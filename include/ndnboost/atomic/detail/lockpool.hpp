/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2011 Helge Bahmann
 * Copyright (c) 2013-2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/lockpool.hpp
 *
 * This header contains declaration of the lockpool used to emulate atomic ops.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_LOCKPOOL_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_LOCKPOOL_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>
#include <ndnboost/atomic/detail/link.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace ndnboost {
namespace atomics {
namespace detail {

struct lockpool
{
    class scoped_lock
    {
        void* m_lock;

    public:
        explicit NDNBOOST_ATOMIC_DECL scoped_lock(const volatile void* addr) NDNBOOST_NOEXCEPT;
        NDNBOOST_ATOMIC_DECL ~scoped_lock() NDNBOOST_NOEXCEPT;

        NDNBOOST_DELETED_FUNCTION(scoped_lock(scoped_lock const&))
        NDNBOOST_DELETED_FUNCTION(scoped_lock& operator=(scoped_lock const&))
    };

    static NDNBOOST_ATOMIC_DECL void thread_fence() NDNBOOST_NOEXCEPT;
    static NDNBOOST_ATOMIC_DECL void signal_fence() NDNBOOST_NOEXCEPT;
};

} // namespace detail
} // namespace atomics
} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_DETAIL_LOCKPOOL_HPP_INCLUDED_
