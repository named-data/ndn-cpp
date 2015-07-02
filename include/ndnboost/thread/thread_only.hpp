#ifndef NDNBOOST_THREAD_THREAD_ONLY_HPP
#define NDNBOOST_THREAD_THREAD_ONLY_HPP

//  thread.hpp
//
//  (C) Copyright 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/platform.hpp>

#if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#include <ndnboost/thread/win32/thread_data.hpp>
#elif defined(NDNBOOST_THREAD_PLATFORM_PTHREAD)
#include <ndnboost/thread/pthread/thread_data.hpp>
#else
#error "Boost threads unavailable on this platform"
#endif

#include <ndnboost/thread/detail/thread.hpp>
#if defined NDNBOOST_THREAD_PROVIDES_INTERRUPTIONS
#include <ndnboost/thread/detail/thread_interruption.hpp>
#endif
#include <ndnboost/thread/v2/thread.hpp>


#endif
