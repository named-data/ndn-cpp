//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011-2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/thread for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef NDNBOOST_THREAD_DETAIL_IS_CONVERTIBLE_HPP
#define NDNBOOST_THREAD_DETAIL_IS_CONVERTIBLE_HPP

#include <ndnboost/type_traits/is_convertible.hpp>
#include <ndnboost/thread/detail/move.hpp>

namespace ndnboost
{
  namespace thread_detail
  {
    template <typename T1, typename T2>
    struct is_convertible : ndnboost::is_convertible<T1,T2> {};

#if defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES

#if defined(NDNBOOST_INTEL_CXX_VERSION) && (NDNBOOST_INTEL_CXX_VERSION <= 1300)

#if defined NDNBOOST_THREAD_USES_MOVE
    template <typename T1, typename T2>
    struct is_convertible<
      rv<T1> &,
      rv<rv<T2> > &
    > : false_type {};
#endif

#elif defined __GNUC__ && (__GNUC__ < 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ <= 4 ))

    template <typename T1, typename T2>
    struct is_convertible<T1&, T2&> : ndnboost::is_convertible<T1, T2> {};
#endif

#endif
  }

} // namespace ndnboost


#endif //  NDNBOOST_THREAD_DETAIL_MEMORY_HPP
