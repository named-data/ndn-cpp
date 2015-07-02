#ifndef NDNBOOST_THREAD_CONDITION_VARIABLE_HPP
#define NDNBOOST_THREAD_CONDITION_VARIABLE_HPP

//  condition_variable.hpp
//
//  (C) Copyright 2007 Anthony Williams 
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/thread/detail/platform.hpp>
#if defined(NDNBOOST_THREAD_PLATFORM_WIN32)
#include <ndnboost/thread/win32/condition_variable.hpp>
#elif defined(NDNBOOST_THREAD_PLATFORM_PTHREAD)
#include <ndnboost/thread/pthread/condition_variable.hpp>
#else
#error "Boost threads unavailable on this platform"
#endif

#endif
