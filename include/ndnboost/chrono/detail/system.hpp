//  Copyright 2009-2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef NDNBOOST_CHRONO_DETAIL_SYSTEM_HPP
#define NDNBOOST_CHRONO_DETAIL_SYSTEM_HPP

#if !defined NDNBOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING

#include <ndnboost/version.hpp>
#include <ndnboost/system/error_code.hpp>

#if ((NDNBOOST_VERSION / 100000) < 2) && ((NDNBOOST_VERSION / 100 % 1000) < 44)
#define NDNBOOST_CHRONO_SYSTEM_CATEGORY ndnboost::system::system_category
#else
#define NDNBOOST_CHRONO_SYSTEM_CATEGORY ndnboost::system::system_category()
#endif

#ifdef NDNBOOST_SYSTEM_NO_DEPRECATED
#define NDNBOOST_CHRONO_THROWS ndnboost::throws()
#define NDNBOOST_CHRONO_IS_THROWS(EC) (&EC==&ndnboost::throws())
#else
#define NDNBOOST_CHRONO_THROWS ndnboost::system::throws
#define NDNBOOST_CHRONO_IS_THROWS(EC) (&EC==&ndnboost::system::throws)
#endif

#endif
#endif
