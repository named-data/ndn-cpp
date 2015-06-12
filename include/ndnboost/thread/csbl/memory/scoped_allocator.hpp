// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/10 Vicente J. Botet Escriba
//   Creation.

#ifndef NDNBOOST_CSBL_MEMORY_SCOPED_ALLOCATOR_HPP
#define NDNBOOST_CSBL_MEMORY_SCOPED_ALLOCATOR_HPP

#include <ndnboost/thread/csbl/memory/config.hpp>

// 20.7.7, uses_allocator
#if defined NDNBOOST_NO_CXX11_ALLOCATOR
#include <ndnboost/container/scoped_allocator.hpp>

namespace ndnboost
{
  namespace csbl
  {
    using ::ndnboost::container::uses_allocator;
  }
}
#else
namespace ndnboost
{
  namespace csbl
  {
    using ::std::uses_allocator;
  }
}
#endif // NDNBOOST_NO_CXX11_POINTER_TRAITS

#endif // header
