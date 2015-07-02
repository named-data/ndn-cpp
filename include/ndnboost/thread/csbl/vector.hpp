// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/10 Vicente J. Botet Escriba
//   Creation.

#ifndef NDNBOOST_CSBL_VECTOR_HPP
#define NDNBOOST_CSBL_VECTOR_HPP

#include <ndnboost/config.hpp>

#if defined NDNBOOST_THREAD_USES_NDNBOOST_VECTOR || defined NDNBOOST_NO_CXX11_HDR_VECTOR || defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#ifndef NDNBOOST_THREAD_USES_NDNBOOST_VECTOR
#define NDNBOOST_THREAD_USES_NDNBOOST_VECTOR
#endif
#include <ndnboost/container/vector.hpp>
#else
#include <vector>
#endif

namespace ndnboost
{
  namespace csbl
  {
#if defined NDNBOOST_THREAD_USES_NDNBOOST_VECTOR
    using ::ndnboost::container::vector;
#else
    using ::std::vector;
#endif

  }
}
#endif // header
