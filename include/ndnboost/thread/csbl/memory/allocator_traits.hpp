// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/10 Vicente J. Botet Escriba
//   Creation.

#ifndef NDNBOOST_CSBL_MEMORY_ALLOCATOR_TRAITS_HPP
#define NDNBOOST_CSBL_MEMORY_ALLOCATOR_TRAITS_HPP

#include <ndnboost/thread/csbl/memory/config.hpp>

// 20.7.8, allocator traits
#if defined NDNBOOST_NO_CXX11_ALLOCATOR
#include <ndnboost/container/allocator_traits.hpp>

namespace ndnboost
{
  namespace csbl
  {
    using ::ndnboost::container::allocator_traits;
  }
}
#else
namespace ndnboost
{
  namespace csbl
  {
    using ::std::allocator_traits;
  }
}
#endif // NDNBOOST_NO_CXX11_POINTER_TRAITS

#endif // header
