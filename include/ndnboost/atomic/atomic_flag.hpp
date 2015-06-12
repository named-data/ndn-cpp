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
 * \file   atomic/atomic_flag.hpp
 *
 * This header contains definition of \c atomic_flag.
 */

#ifndef NDNBOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_

#include <ndnboost/atomic/capabilities.hpp>
#include <ndnboost/atomic/detail/operations.hpp>
#include <ndnboost/atomic/detail/atomic_flag.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace ndnboost {

using atomics::atomic_flag;

} // namespace ndnboost

#endif // NDNBOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_
