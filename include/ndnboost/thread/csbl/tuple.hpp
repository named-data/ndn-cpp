// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/10 Vicente J. Botet Escriba
//   Creation.

#ifndef NDNBOOST_CSBL_TUPLE_HPP
#define NDNBOOST_CSBL_TUPLE_HPP

#include <ndnboost/config.hpp>

#if defined NDNBOOST_THREAD_USES_NDNBOOST_TUPLE || defined NDNBOOST_NO_CXX11_HDR_TUPLE || defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#include <ndnboost/tuple/tuple.hpp>
#ifndef NDNBOOST_THREAD_USES_NDNBOOST_TUPLE
#define NDNBOOST_THREAD_USES_NDNBOOST_TUPLE
#endif

#else
#include <tuple>
#endif

namespace ndnboost
{
  namespace csbl
  {
#if defined NDNBOOST_THREAD_USES_NDNBOOST_TUPLE
    using ::ndnboost::tuple;
    using ::ndnboost::get;
#else
    // 20.4.2, class template tuple:
    using ::std::tuple;
    using ::std::get;
    // 20.4.2.4, tuple creation functions:
    // 20.4.2.5, tuple helper classes:
    // 20.4.2.6, element access:
    // 20.4.2.7, relational operators:
    // 20.4.2.8, allocator-related traits
    // 20.4.2.9, specialized algorithms:
#endif

  }
}
#endif // header
