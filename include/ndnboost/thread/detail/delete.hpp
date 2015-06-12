// Copyright (C) 2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NDNBOOST_THREAD_DETAIL_DELETE_HPP
#define NDNBOOST_THREAD_DETAIL_DELETE_HPP

#include <ndnboost/config.hpp>

/**
 * NDNBOOST_THREAD_DELETE_COPY_CTOR deletes the copy constructor when the compiler supports it or
 * makes it private.
 *
 * NDNBOOST_THREAD_DELETE_COPY_ASSIGN deletes the copy assignment when the compiler supports it or
 * makes it private.
 */

#ifndef NDNBOOST_NO_CXX11_DELETED_FUNCTIONS
#define NDNBOOST_THREAD_DELETE_COPY_CTOR(CLASS) \
      CLASS(CLASS const&) = delete; \

#define NDNBOOST_THREAD_DELETE_COPY_ASSIGN(CLASS) \
      CLASS& operator=(CLASS const&) = delete;

#else // NDNBOOST_NO_CXX11_DELETED_FUNCTIONS
#if defined(NDNBOOST_MSVC) && _MSC_VER >= 1600
#define NDNBOOST_THREAD_DELETE_COPY_CTOR(CLASS) \
    private: \
      CLASS(CLASS const&); \
    public:

#define NDNBOOST_THREAD_DELETE_COPY_ASSIGN(CLASS) \
    private: \
      CLASS& operator=(CLASS const&); \
    public:
#else
#define NDNBOOST_THREAD_DELETE_COPY_CTOR(CLASS) \
    private: \
      CLASS(CLASS&); \
    public:

#define NDNBOOST_THREAD_DELETE_COPY_ASSIGN(CLASS) \
    private: \
      CLASS& operator=(CLASS&); \
    public:
#endif
#endif // NDNBOOST_NO_CXX11_DELETED_FUNCTIONS

/**
 * NDNBOOST_THREAD_NO_COPYABLE deletes the copy constructor and assignment when the compiler supports it or
 * makes them private.
 */
#define NDNBOOST_THREAD_NO_COPYABLE(CLASS) \
    NDNBOOST_THREAD_DELETE_COPY_CTOR(CLASS) \
    NDNBOOST_THREAD_DELETE_COPY_ASSIGN(CLASS)

#endif // NDNBOOST_THREAD_DETAIL_DELETE_HPP
