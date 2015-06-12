//  (C) Copyright Gennadiy Rozental 2001-2008.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : Entry point for the end user into the Test Execution Monitor.
// ***************************************************************************

#ifndef NDNBOOST_TEST_EXEC_MONITOR_HPP_071894GER
#define NDNBOOST_TEST_EXEC_MONITOR_HPP_071894GER

// Boost.Test
#include <ndnboost/test/test_tools.hpp>

//____________________________________________________________________________//

// ************************************************************************** //
// **************                 Auto Linking                 ************** //
// ************************************************************************** //

// Automatically link to the correct build variant where possible. 
#if !defined(NDNBOOST_ALL_NO_LIB) && !defined(NDNBOOST_TEST_NO_LIB) && \
    !defined(NDNBOOST_TEST_SOURCE) && !defined(NDNBOOST_TEST_INCLUDED)

#  define NDNBOOST_LIB_NAME ndnboost_test_exec_monitor
#  include <ndnboost/config/auto_link.hpp>

#endif  // auto-linking disabled

#endif // NDNBOOST_TEST_EXEC_MONITOR_HPP_071894GER
