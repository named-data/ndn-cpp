#ifndef NDNBOOST_DETAIL_SP_TYPEINFO_HPP_INCLUDED
#define NDNBOOST_DETAIL_SP_TYPEINFO_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  detail/sp_typeinfo.hpp
//
//  Deprecated, please use ndnboost/core/typeinfo.hpp
//
//  Copyright 2007 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <ndnboost/core/typeinfo.hpp>

namespace ndnboost
{

namespace detail
{

typedef ndnboost::core::typeinfo sp_typeinfo;

} // namespace detail

} // namespace ndnboost

#define NDNBOOST_SP_TYPEID(T) NDNBOOST_CORE_TYPEID(T)

#endif  // #ifndef NDNBOOST_DETAIL_SP_TYPEINFO_HPP_INCLUDED
