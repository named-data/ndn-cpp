/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/operations_lockfree.hpp
 *
 * This header defines lockfree atomic operations.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_OPERATIONS_LOCKFREE_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_OPERATIONS_LOCKFREE_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>
#include <ndnboost/atomic/detail/platform.hpp>

#if !defined(NDNBOOST_ATOMIC_EMULATED)
#include NDNBOOST_ATOMIC_DETAIL_HEADER(ndnboost/atomic/detail/ops_)
#else
#include <ndnboost/atomic/detail/operations_fwd.hpp>
#endif

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#endif // NDNBOOST_ATOMIC_DETAIL_OPERATIONS_LOCKFREE_HPP_INCLUDED_
