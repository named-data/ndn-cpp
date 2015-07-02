
//  (C) Copyright Edward Diener 2012
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(NDNBOOST_TTI_DETAIL_NULLPTR_HPP)
#define NDNBOOST_TTI_DETAIL_NULLPTR_HPP

#include <ndnboost/config.hpp>

#if defined(NDNBOOST_NO_CXX11_NULLPTR)

#define NDNBOOST_TTI_DETAIL_NULLPTR 0

#else // !NDNBOOST_NO_CXX11_NULLPTR

#define NDNBOOST_TTI_DETAIL_NULLPTR nullptr

#endif // NDNBOOST_NO_CXX11_NULLPTR

#endif // NDNBOOST_TTI_DETAIL_NULLPTR_HPP
