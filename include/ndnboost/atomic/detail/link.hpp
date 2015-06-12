/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2012 Hartmut Kaiser
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/detail/config.hpp
 *
 * This header defines macros for linking with compiled library of Boost.Atomic
 */

#ifndef NDNBOOST_ATOMIC_DETAIL_LINK_HPP_INCLUDED_
#define NDNBOOST_ATOMIC_DETAIL_LINK_HPP_INCLUDED_

#include <ndnboost/atomic/detail/config.hpp>

#ifdef NDNBOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//  Set up dll import/export options
#if (defined(NDNBOOST_ATOMIC_DYN_LINK) || defined(NDNBOOST_ALL_DYN_LINK)) && \
    !defined(NDNBOOST_ATOMIC_STATIC_LINK)

#if defined(NDNBOOST_ATOMIC_SOURCE)
#define NDNBOOST_ATOMIC_DECL NDNBOOST_SYMBOL_EXPORT
#define NDNBOOST_ATOMIC_BUILD_DLL
#else
#define NDNBOOST_ATOMIC_DECL NDNBOOST_SYMBOL_IMPORT
#endif

#endif // building a shared library

#ifndef NDNBOOST_ATOMIC_DECL
#define NDNBOOST_ATOMIC_DECL
#endif

///////////////////////////////////////////////////////////////////////////////
//  Auto library naming
#if !defined(NDNBOOST_ATOMIC_SOURCE) && !defined(NDNBOOST_ALL_NO_LIB) && \
    !defined(NDNBOOST_ATOMIC_NO_LIB)

#define NDNBOOST_LIB_NAME ndnboost_atomic

// tell the auto-link code to select a dll when required:
#if defined(NDNBOOST_ALL_DYN_LINK) || defined(NDNBOOST_ATOMIC_DYN_LINK)
#define NDNBOOST_DYN_LINK
#endif

#include <ndnboost/config/auto_link.hpp>

#endif  // auto-linking disabled

#endif
