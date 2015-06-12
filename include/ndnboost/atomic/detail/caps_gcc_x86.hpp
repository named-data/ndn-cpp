/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2009 Helge Bahmann
 * Copyright (c) 2012 Tim Blechmann
 * Copyright (c) 2013 - 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/caps_gcc_x86.hpp
 *
 * This header defines feature capabilities macros
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_CAPS_GCC_X86_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_CAPS_GCC_X86_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if defined(__i386__) &&\
    (\
        defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8) ||\
        defined(__i586__) || defined(__i686__) || defined(__pentium4__) || defined(__nocona__) || defined(__core2__) || defined(__corei7__) ||\
        defined(__k6__) || defined(__athlon__) || defined(__k8__) || defined(__amdfam10__) || defined(__bdver1__) || defined(__bdver2__) || defined(__bdver3__) || defined(__btver1__) || defined(__btver2__)\
    )
#define NDNBOOST_ATOMIC_DETAIL_X86_HAS_CMPXCHG8B 1
#endif

#if defined(__x86_64__) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16)
#define NDNBOOST_ATOMIC_DETAIL_X86_HAS_CMPXCHG16B 1
#endif

#define NDNBOOST_ATOMIC_INT8_LOCK_FREE 2
#define NDNBOOST_ATOMIC_INT16_LOCK_FREE 2
#define NDNBOOST_ATOMIC_INT32_LOCK_FREE 2
#if defined(__x86_64__) || defined(NDNBOOST_ATOMIC_DETAIL_X86_HAS_CMPXCHG8B)
#define NDNBOOST_ATOMIC_INT64_LOCK_FREE 2
#endif
#if defined(NDNBOOST_ATOMIC_DETAIL_X86_HAS_CMPXCHG16B) && (defined(NDNBOOST_HAS_INT128) || !defined(NDNBOOST_NO_ALIGNMENT))
#define NDNBOOST_ATOMIC_INT128_LOCK_FREE 2
#endif
#define NDNBOOST_ATOMIC_POINTER_LOCK_FREE 2

#define NDNBOOST_ATOMIC_THREAD_FENCE 2
#define NDNBOOST_ATOMIC_SIGNAL_FENCE 2

#endif // NDNBOOST_ATOMIC_DETAIL_CAPS_GCC_X86_HPP_INCLUDED_
