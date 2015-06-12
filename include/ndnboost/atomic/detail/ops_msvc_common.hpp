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
 * \file   atomic/detail/ops_msvc_common.hpp
 *
 * This header contains common tools for MSVC implementation of the \c operations template.
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_OPS_MSVC_COMMON_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_OPS_MSVC_COMMON_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

// Define compiler barriers
#if defined(__INTEL_COMPILER)
#define NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER() __memory_barrier()
#elif defined(_MSC_VER) && !defined(_WIN32_WCE)
extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)
#define NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER() _ReadWriteBarrier()
#endif

#ifndef NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER
#define NDNBOOST_ATOMIC_DETAIL_COMPILER_BARRIER()
#endif

#endif // NDNBOOST_ATOMIC_DETAIL_OPS_MSVC_COMMON_HPP_INCLUDED_
