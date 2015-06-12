// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef NDNBOOST_THREAD_EXCEPTIONAL_PTR_HPP
#define NDNBOOST_THREAD_EXCEPTIONAL_PTR_HPP

#include <ndnboost/thread/detail/move.hpp>
#include <ndnboost/exception_ptr.hpp>

#include <ndnboost/config/abi_prefix.hpp>

namespace ndnboost
{
  struct exceptional_ptr {
    exception_ptr ptr_;

    exceptional_ptr() : ptr_() {}
    explicit exceptional_ptr(exception_ptr ex) : ptr_(ex) {}
    template <class E>
    explicit exceptional_ptr(NDNBOOST_FWD_REF(E) ex) : ptr_(ndnboost::copy_exception(ndnboost::forward<E>(ex))) {}
  };

  template <class E>
  inline exceptional_ptr make_exceptional(NDNBOOST_FWD_REF(E) ex) {
    return exceptional_ptr(ndnboost::forward<E>(ex));
  }

  inline exceptional_ptr make_exceptional(exception_ptr ex)
  {
    return exceptional_ptr(ex);
  }

  inline exceptional_ptr make_exceptional()
  {
    return exceptional_ptr();
  }

} // namespace ndnboost

#include <ndnboost/config/abi_suffix.hpp>

#endif
