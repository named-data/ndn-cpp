#ifndef NDNBOOST_CORE_NO_EXCEPTIONS_SUPPORT_HPP
#define NDNBOOST_CORE_NO_EXCEPTIONS_SUPPORT_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

//----------------------------------------------------------------------
// (C) Copyright 2004 Pavel Vozenilek.
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// This file contains helper macros used when exception support may be
// disabled (as indicated by macro NDNBOOST_NO_EXCEPTIONS).
//
// Before picking up these macros you may consider using RAII techniques
// to deal with exceptions - their syntax can be always the same with 
// or without exception support enabled.
//----------------------------------------------------------------------

#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>

#if !(defined NDNBOOST_NO_EXCEPTIONS)
#    define NDNBOOST_TRY { try
#    define NDNBOOST_CATCH(x) catch(x)
#    define NDNBOOST_RETHROW throw;
#    define NDNBOOST_CATCH_END }
#else
#    if NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x564))
#        define NDNBOOST_TRY { if ("")
#        define NDNBOOST_CATCH(x) else if (!"")
#    else
#        define NDNBOOST_TRY { if (true)
#        define NDNBOOST_CATCH(x) else if (false)
#    endif
#    define NDNBOOST_RETHROW
#    define NDNBOOST_CATCH_END }
#endif


#endif 
