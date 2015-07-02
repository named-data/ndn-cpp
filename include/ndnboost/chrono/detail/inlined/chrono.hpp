//  chrono.cpp  --------------------------------------------------------------//

//  Copyright Beman Dawes 2008
//  Copyright Vicente J. Botet Escriba 2009

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef NDNBOOST_CHRONO_DETAIL_INLINED_CHRONO_HPP
#define NDNBOOST_CHRONO_DETAIL_INLINED_CHRONO_HPP

#include <ndnboost/version.hpp>
#include <ndnboost/chrono/chrono.hpp>
#include <ndnboost/system/system_error.hpp>
#include <ndnboost/throw_exception.hpp>
#include <ndnboost/chrono/detail/system.hpp>

//----------------------------------------------------------------------------//
//                                                                            //
//                     Platform-specific Implementations                      //
//                                                                            //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//                                Windows                                     //
//----------------------------------------------------------------------------//
#if defined(NDNBOOST_CHRONO_WINDOWS_API)
#include <ndnboost/chrono/detail/inlined/win/chrono.hpp>

//----------------------------------------------------------------------------//
//                                 Mac                                        //
//----------------------------------------------------------------------------//
#elif defined(NDNBOOST_CHRONO_MAC_API)
#include <ndnboost/chrono/detail/inlined/mac/chrono.hpp>

//----------------------------------------------------------------------------//
//                                POSIX                                     //
//----------------------------------------------------------------------------//
#elif defined(NDNBOOST_CHRONO_POSIX_API)
#include <ndnboost/chrono/detail/inlined/posix/chrono.hpp>

#endif  // POSIX

#endif
