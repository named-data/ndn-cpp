// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/10 Vicente J. Botet Escriba
//   Creation.

#ifndef NDNBOOST_CSBL_MEMORY_ALLOCATOR_ARG_HPP
#define NDNBOOST_CSBL_MEMORY_ALLOCATOR_ARG_HPP

#include <ndnboost/thread/csbl/memory/config.hpp>

// 20.7.6, allocator argument tag
#if defined NDNBOOST_NO_CXX11_ALLOCATOR
#include <ndnboost/container/scoped_allocator.hpp>

namespace ndnboost
{
  namespace csbl
  {
    using ::ndnboost::container::allocator_arg_t;
    using ::ndnboost::container::allocator_arg;
  }
}
#else
namespace ndnboost
{
  namespace csbl
  {
    using ::std::allocator_arg_t;
    using ::std::allocator_arg;
  }
}
#endif // NDNBOOST_NO_CXX11_ALLOCATOR
namespace ndnboost
{
  using ::ndnboost::csbl::allocator_arg_t;
  using ::ndnboost::csbl::allocator_arg;
}
#endif // header
